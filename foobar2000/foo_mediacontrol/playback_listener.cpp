#include "stdafx.h"

#include "playback_listener.h"
#include "media_controls.h"
#include "track_data.h"

static inline Windows::Foundation::TimeSpan timespan_for_duration(double duration_s)
{
	// TimeSpan's contain a time period expressed in 100-nanosecond units.
	// https://learn.microsoft.com/en-us/uwp/api/windows.foundation.timespan
	return Windows::Foundation::TimeSpan{ static_cast<long long>(duration_s * 1e7) };
}

static Windows::Media::SystemMediaTransportControlsTimelineProperties^
create_timeline_properties(double duration_s, double position_s)
{
	duration_s = std::max(0.0, duration_s);
	position_s = std::max(0.0, std::min(position_s, duration_s));
	auto timeline_properties = ref new Windows::Media::SystemMediaTransportControlsTimelineProperties();
	timeline_properties->Position = timespan_for_duration(position_s);
	timeline_properties->StartTime = timespan_for_duration(0);
	timeline_properties->EndTime = timespan_for_duration(duration_s);
	timeline_properties->MinSeekTime = timespan_for_duration(0);
	timeline_properties->MaxSeekTime = timespan_for_duration(duration_s);
	return timeline_properties;
}

void playback_listener::on_playback_new_track(metadb_handle_ptr p_track) {
	try {
		// create new track data from metadb handle
		track_data data(p_track);

		// Reset the playback information to its initial state
		reset_playback(data.get_duration(), true);

		// update the media controls
		media_controls::get()
			.reset()
			.set_title(data.get_title() != track_data::empty ? data.get_title() : data.get_file_name())
			.set_artist(data.get_artist())
			.set_genres(data.get_genres())
			.set_album(data.get_album())
			.set_track_number(data.get_track_number())
			.set_thumbnail(data.get_album_art())
			.set_timeline_properties(current_timeline_properties())
			.apply_changes();
	}
	catch (pfc::exception e) {
		popup_message::g_show("Caught exception", "Error");
	}
}

void playback_listener::on_playback_starting(play_control::t_track_command p_command, bool p_paused) {
	// Note: This is called before on_playback_new_track()
	media_controls::get().play();
}

void playback_listener::on_playback_stop(play_control::t_stop_reason p_reason) {
	if (p_reason != play_control::t_stop_reason::stop_reason_starting_another) {
		set_playing(false);
		update_timeline_properties();
		media_controls::get().stop();
	}
}

void playback_listener::on_playback_pause(bool p_state) {
	set_playing(!p_state);
	update_timeline_properties();
	if (p_state) {
		media_controls::get().pause();
	}
	else {
		media_controls::get().play();
	}
}

void playback_listener::on_playback_seek(double p_time) {
	set_position(p_time);
	update_timeline_properties(p_time);
}

static inline std::chrono::milliseconds get_steady_time()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::steady_clock::now().time_since_epoch());
}

void playback_listener::reset_playback(double duration, bool playing)
{
	m_last_duration = std::max(0.0, duration);
	m_position = 0.0;
	m_position_timestamp = get_steady_time();
	m_playing = playing;
}

inline bool playback_listener::has_duration() const
{
	return m_last_duration >= 0;
}

inline double playback_listener::get_duration() const
{
	return std::max(0.0, m_last_duration);
}

void playback_listener::set_position(double position)
{
	if (has_duration())
		position = std::min(position, get_duration());
	m_position = std::max(0.0, position);
	m_position_timestamp = get_steady_time();
}

double playback_listener::get_live_position(std::chrono::milliseconds when) const
{
	if (is_playing()) {
		std::chrono::milliseconds now = std::max(when, m_position_timestamp);
		auto delta_s = static_cast<double>((now - m_position_timestamp).count()) / 1000.0;
		auto live_position = m_position + delta_s;
		if (has_duration()) {
			live_position = std::min(live_position, get_duration());
		}
		return live_position;
	}
	return m_position;
}

inline double playback_listener::get_live_position() const
{
	return get_live_position(get_steady_time());
}

void playback_listener::freeze_live_position()
{
	auto now = get_steady_time();
	m_position = get_live_position(now);
	m_position_timestamp = now;
}

inline void playback_listener::set_playing(bool playing)
{
	if (m_playing && !playing || !m_playing && playing) {
		// Freeze the playback position when the state changes.
		freeze_live_position();
	}
	m_playing = playing;
}

inline bool playback_listener::is_playing() const
{
	return m_playing;
}

inline Windows::Media::SystemMediaTransportControlsTimelineProperties^
playback_listener::current_timeline_properties() const
{
	return create_timeline_properties(get_duration(), m_position);
}

inline void playback_listener::update_timeline_properties(double position) const
{
	media_controls::get()
		.set_timeline_properties(create_timeline_properties(get_duration(), position))
		.apply_changes();
}

inline void playback_listener::update_timeline_properties() const
{
	media_controls::get()
		.set_timeline_properties(current_timeline_properties())
		.apply_changes();
}
