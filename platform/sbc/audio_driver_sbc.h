#ifndef AUDIO_DRIVER_SBC_H
#define AUDIO_DRIVER_SBC_H

#pragma once

#include "core/error/error_list.h"
#include "core/os/main_loop.h"
#include "core/os/memory.h"
#include "core/os/mutex.h"
#include "core/os/os.h"
#include "core/os/semaphore.h"
#include "core/os/thread.h"
#include "core/typedefs.h"
#include "servers/audio_server.h"
#include <SDL2/SDL.h>

class AudioDriverSBC : public AudioDriver {
	SDL_AudioDeviceID device = 0;
	SDL_AudioSpec audio_spec = {};
	String output_device_name = "Default";
	String new_output_device;
	bool active = false;
	bool exit_thread = false;
	Thread thread;
	Mutex mutex;
	Vector<int32_t> samples_in;
	int mix_rate = 48000;
	int channels = 2;
	int latency = 2048; // Default latency in samples
	SpeakerMode speaker_mode = SPEAKER_MODE_STEREO;

	Error init_output_device();
	void finish_output_device();

	static void audio_callback(void *userdata, Uint8 *stream, int len);
	static void thread_func(void *p_udata);

public:
	virtual const char *get_name() const override { return "SBC"; }
	virtual Error init() override;
	virtual void start() override;
	virtual int get_mix_rate() const override;
	virtual SpeakerMode get_speaker_mode() const override;

	virtual PackedStringArray get_output_device_list() override;
	virtual String get_output_device() override;
	virtual void set_output_device(const String &p_name) override;

	virtual void lock() override;
	virtual void unlock() override;
	virtual void finish() override;

	~AudioDriverSBC();
};

#endif // AUDIO_DRIVER_SBC_H