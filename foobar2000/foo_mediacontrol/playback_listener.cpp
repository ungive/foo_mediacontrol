#include "stdafx.h"

#include "playback_listener.h"
#include "media_controls.h"
#include "track_data.h"

void playback_listener::on_playback_new_track(metadb_handle_ptr p_track) {
	try {
		// create new track data from metadb handle
		track_data data(p_track);

		// Store the duration for later retrieval
		auto duration = data.get_duration();
		m_last_duration = duration;

		// update the media controls
		auto controls = media_controls::get();
		controls.begin_update();
		controls.set_title(data.get_title() != track_data::empty ? data.get_title() : data.get_file_name());
		controls.set_artist(data.get_artist());
		controls.set_genres(data.get_genres());
		controls.set_album(data.get_album());
		controls.set_track_number(data.get_track_number());
		controls.set_thumbnail(data.get_album_art());
		auto timeline_properties = ref new Windows::Media::SystemMediaTransportControlsTimelineProperties();
		timeline_properties->Position = Windows::Foundation::TimeSpan{ 0 };
		if (duration > 0) {
			// TimeSpan's contain a time period expressed in 100-nanosecond units.
			// https://learn.microsoft.com/en-us/uwp/api/windows.foundation.timespan
			timeline_properties->StartTime = Windows::Foundation::TimeSpan{ 0 };
			timeline_properties->EndTime = Windows::Foundation::TimeSpan{ static_cast<long long>(duration * 1e7) };
			timeline_properties->MinSeekTime = Windows::Foundation::TimeSpan{ 0 };
			timeline_properties->MaxSeekTime = Windows::Foundation::TimeSpan{ static_cast<long long>(duration * 1e7) };
		}
		controls.set_timeline_properties(timeline_properties);
		controls.end_update();
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
	auto timeline_properties = ref new Windows::Media::SystemMediaTransportControlsTimelineProperties();
	timeline_properties->Position = Windows::Foundation::TimeSpan{ static_cast<long long>(p_time * 1e7) };
	if (m_last_duration > 0) {
		timeline_properties->StartTime = Windows::Foundation::TimeSpan{ 0 };
		timeline_properties->EndTime = Windows::Foundation::TimeSpan{ static_cast<long long>(m_last_duration * 1e7) };
		timeline_properties->MinSeekTime = Windows::Foundation::TimeSpan{ 0 };
		timeline_properties->MaxSeekTime = Windows::Foundation::TimeSpan{ static_cast<long long>(m_last_duration * 1e7) };
	}
	media_controls::get()
		.set_timeline_properties(timeline_properties);
}
