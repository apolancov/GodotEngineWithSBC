#include "audio_driver_sbc.h"
#include "core/config/project_settings.h"
#include "core/os/os.h"

void AudioDriverSBC::audio_callback(void *userdata, Uint8 *stream, int len) {
	AudioDriverSBC *ad = (AudioDriverSBC *)userdata;
	int n_of_samples = len / sizeof(int16_t);
	int frames = n_of_samples / ad->channels;

	ad->lock();
	// Mix audio into the samples_in buffer
	// Mix Godot in int32_t (frames, channels)
	ad->audio_server_process(frames, ad->samples_in.ptrw());
	ad->unlock();

	int16_t *data16 = (int16_t *)stream;
	// Convert each sample from int32 to int16 with saturation
	for (int i = 0; i < n_of_samples; i++) {
		int32_t s = ad->samples_in[i];
		// Saturation to avoid wrap
		if (s > 32767 << 16) {
			s = 32767 << 16;
		}
		if (s < -32768 << 16) {
			s = -32768 << 16;
		}
		data16[i] = s >> 16;
	}
}

Error AudioDriverSBC::init_output_device() {
	if (device) {
		SDL_ClearQueuedAudio(device); // Clear the queue before closing
		SDL_CloseAudioDevice(device);
		device = 0;
	}

	SDL_AudioSpec want = {};
	want.freq = mix_rate;
	want.format = AUDIO_S16SYS; // int16_t
	want.channels = channels;
	want.samples = latency;

	device = SDL_OpenAudioDevice(
			output_device_name == "Default" ? nullptr : output_device_name.utf8().get_data(),
			0, &want, &audio_spec, 0);
	if (!device) {
		return ERR_CANT_OPEN;
	}

	print_line("SDL Opened Audio Device: format=", audio_spec.format, ", freq=", audio_spec.freq, ", channels=", audio_spec.channels);

	samples_in.resize(latency * channels);
	return OK;
}

Error AudioDriverSBC::init() {
	active = false;

	SDL_AudioSpec want = {};
	want.freq = mix_rate;
	want.format = AUDIO_S16SYS; // El driver de hardware suele preferir S16
	want.channels = channels;
	want.samples = latency;
	want.callback = audio_callback;
	want.userdata = this;

	device = SDL_OpenAudioDevice(
			output_device_name == "Default" ? nullptr : output_device_name.utf8().get_data(),
			0, &want, &audio_spec, 0);

	if (!device) {
		OS::get_singleton()->print("Failed to open audio device: %s\n", SDL_GetError());
		return ERR_CANT_OPEN;
	}

	mix_rate = audio_spec.freq;
	channels = audio_spec.channels;
	latency = audio_spec.samples;

	samples_in.resize(latency * channels);

	OS::get_singleton()->print("SDL Opened Audio Device: format=%d, freq=%d, channels=%d\n",
			audio_spec.format, audio_spec.freq, audio_spec.channels);

	return OK;
}

void AudioDriverSBC::thread_func(void *p_udata) {
	AudioDriverSBC *ad = static_cast<AudioDriverSBC *>(p_udata);

	// Maximum size of the queue is based on latency, channels, and sample size
	// Latency is in frames, channels is the number of audio channels, and sample size is 4 bytes for int32_t
	// This is to prevent the queue from overflowing and causing memory leaks
	// The queue size is calculated as: latency * channels * sizeof(int32_t) * 2
	const Uint32 max_queue = ad->latency * ad->channels * sizeof(int32_t) * 2;

	while (!ad->exit_thread) {
		ad->lock();
		if (!ad->active) {
			for (int i = 0; i < ad->latency * ad->channels; i++) {
				ad->samples_in.write[i] = 0;
			}
		} else {
			// Mix audio using the godot system (int32_t*)
			ad->audio_server_process(ad->latency, ad->samples_in.ptrw());
		}

		if (ad->device) {
			if (ad->device) {
				Uint32 queued = SDL_GetQueuedAudioSize(ad->device);
				if (queued < max_queue) {
					// Conversión de int32 a int16
					// SDL expects 16-bit samples, so we need to convert the 32-bit samples to 16-bit
					static Vector<int16_t> temp_buf;
					temp_buf.resize(ad->latency * ad->channels);
					for (int i = 0; i < ad->latency * ad->channels; ++i) {
						// CLAMP the sample to prevent overflow
						int32_t sample = ad->samples_in[i];
						sample = CLAMP(sample, -2147483648, 2147483647); // safety, clamp to int32_t range
						temp_buf.write[i] = (int16_t)(sample >> 16);
					}
					SDL_QueueAudio(ad->device, temp_buf.ptr(), ad->latency * ad->channels * sizeof(int16_t));
				}
			}
		}
		ad->unlock();
		OS::get_singleton()->delay_usec(1000); // Sleep for a short time to avoid busy waiting
		// Adjust the sleep time based on the latency to avoid busy waiting
		// This is a rough estimate, you may want to adjust it based on your needs
		// 1000 microseconds = 1 millisecond
		// This is a placeholder, you may want to adjust it based on your needs
	}
}

void AudioDriverSBC::start() {
	active = true;
	if (device) {
		SDL_PauseAudioDevice(device, 0);
	}
}

int AudioDriverSBC::get_mix_rate() const {
	return mix_rate;
}

AudioDriver::SpeakerMode AudioDriverSBC::get_speaker_mode() const {
	return speaker_mode;
}

PackedStringArray AudioDriverSBC::get_output_device_list() {
	PackedStringArray list;
	list.push_back("Default");
	int num = SDL_GetNumAudioDevices(0);
	for (int i = 0; i < num; ++i) {
		list.push_back(String::utf8(SDL_GetAudioDeviceName(i, 0)));
	}
	return list;
}

String AudioDriverSBC::get_output_device() {
	return output_device_name;
}

void AudioDriverSBC::set_output_device(const String &p_name) {
	lock();
	new_output_device = p_name;
	unlock();
}

void AudioDriverSBC::lock() {
	mutex.lock();
}

void AudioDriverSBC::unlock() {
	mutex.unlock();
}

void AudioDriverSBC::finish_output_device() {
	if (device) {
		SDL_ClearQueuedAudio(device); // Clear the queue before closing
		SDL_CloseAudioDevice(device);
		device = 0;
	}
}

void AudioDriverSBC::finish() {
	if (device) {
		SDL_PauseAudioDevice(device, 1);
		SDL_CloseAudioDevice(device);
		device = 0;
	}
}

AudioDriverSBC::~AudioDriverSBC() {
	finish();
}