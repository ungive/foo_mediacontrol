#pragma once

class playback_listener : private play_callback_impl_base {
private:
	void on_playback_starting(play_control::t_track_command p_command, bool p_paused);
	void on_playback_new_track(metadb_handle_ptr p_track);
	void on_playback_stop(play_control::t_stop_reason p_reason);
	void on_playback_pause(bool p_state);
	void on_playback_seek(double p_time);

	void reset_playback(double duration, bool playing);
	bool has_duration() const;
	double get_duration() const;
	void set_position(double position);
	double get_live_position() const;
	double get_live_position(std::chrono::milliseconds when) const;
	void freeze_live_position();
	void set_playing(bool playing);
	bool is_playing() const;

	Windows::Media::SystemMediaTransportControlsTimelineProperties^ current_timeline_properties() const;
	void update_timeline_properties(double position) const;
	void update_timeline_properties() const;

	double m_last_duration{ -1.0 };
	double m_position{ 0.0 };
	std::chrono::milliseconds m_position_timestamp{ std::chrono::milliseconds::zero() };
	bool m_playing{ false };
};
