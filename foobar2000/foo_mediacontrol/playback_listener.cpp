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
timeline_properties_for_duration(double duration_s, double position_s = 0.0)
{
	duration_s = max(0.0, duration_s);
	position_s = max(0.0, min(position_s, duration_s));
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

		// Store the duration for later retrieval
		auto duration = data.get_duration();
		m_last_duration = duration;

		// update the media controls
		media_controls::get()
			.begin_update()
			.set_title(data.get_title() != track_data::empty ? data.get_title() : data.get_file_name())
			.set_artist(data.get_artist())
			.set_genres(data.get_genres())
			.set_album(data.get_album())
			.set_track_number(data.get_track_number())
			.set_thumbnail(data.get_album_art())
			.set_timeline_properties(timeline_properties_for_duration(duration))
			.end_update();
	}
	catch (pfc::exception e) {
		popup_message::g_show("Caught exception", "Error");
	}
}

void playback_listener::on_playback_starting(play_control::t_track_command p_command, bool p_paused) {
	media_controls::get().play();
}

void playback_listener::on_playback_stop(play_control::t_stop_reason p_reason) {
	if (p_reason != play_control::t_stop_reason::stop_reason_starting_another) {
		media_controls::get().stop();
	}
}

void playback_listener::on_playback_pause(bool p_state) {
	if (p_state) {
		media_controls::get().pause();
	}
	else {
		media_controls::get().play();
	}
}

void playback_listener::on_playback_seek(double p_time) {
	if (m_last_duration >= 0.0) {
	media_controls::get()
		.set_timeline_properties(timeline_properties_for_duration(m_last_duration, p_time));
}
}
