#include "os_sbc.h"
#include "display_server_sdl.h"

#include "core/config/project_settings.h"
#include "main/main.h"
#include "servers/display_server.h"
#include "servers/rendering_server.h"

#ifdef GLES3_ENABLED

#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#endif

void OS_SBC::initialize() {
	OS_Unix::initialize_core();
	print_line("OS Core initialized successfully");
}

void OS_SBC::finalize() {
	SDL_Quit();
	if (main_loop) {
		main_loop->finalize();
	}
	main_loop = nullptr;
}

void OS_SBC::run() {
	print_line("Rendering server initialized successfully");
	print_line("Starting main loop");

	if (!main_loop) {
		return;
	}

	main_loop->initialize();
	print_line("Main loop initialized successfully");

	while (!quit_requested) {
		DisplayServerSDL::get_singleton()->process_events();

		if (Main::iteration()) {
			break;
		}
	}

	main_loop->finalize();
}

MainLoop *OS_SBC::get_main_loop() const {
	return main_loop;
}

void OS_SBC::set_main_loop(MainLoop *p_main_loop) {
	main_loop = p_main_loop;
}

void OS_SBC::delete_main_loop() {
	if (main_loop) {
		memdelete(main_loop);
	}
	main_loop = nullptr;
}

String OS_SBC::get_name() const {
	return "sbc";
}

String OS_SBC::get_identifier() const {
	return "linuxbsd";
}

void OS_SBC::initialize_joypads() {
}

bool OS_SBC::_check_internal_feature_support(const String &p_feature) {
	return p_feature == "pc" || p_feature == "s3tc";
}

OS_SBC::OS_SBC() {
	//main_loop = nullptr;
	DisplayServerSDL::register_sbc_driver();
	print_line("Video driver SBC initialized");
	AudioDriverManager::add_driver(&audio_driver_sbc);
	print_line("Audio driver SBC registered");
}

OS_SBC::~OS_SBC() {
	if (main_loop) {
		memdelete(main_loop);
	}
	main_loop = nullptr;
}

String OS_SBC::get_config_path() const {
	if (has_environment("XDG_CONFIG_HOME")) {
		if (get_environment("XDG_CONFIG_HOME").is_absolute_path()) {
			return get_environment("XDG_CONFIG_HOME");
		} else {
			WARN_PRINT_ONCE("`XDG_CONFIG_HOME` is a relative path. Ignoring its value and falling back to `$HOME/.config` or `.` per the XDG Base Directory specification.");
			return has_environment("HOME") ? get_environment("HOME").path_join(".config") : ".";
		}
	} else if (has_environment("HOME")) {
		return get_environment("HOME").path_join(".config");
	} else {
		return ".";
	}
}

String OS_SBC::get_data_path() const {
	if (has_environment("XDG_DATA_HOME")) {
		if (get_environment("XDG_DATA_HOME").is_absolute_path()) {
			return get_environment("XDG_DATA_HOME");
		} else {
			WARN_PRINT_ONCE("`XDG_DATA_HOME` is a relative path. Ignoring its value and falling back to `$HOME/.local/share` or `get_config_path()` per the XDG Base Directory specification.");
			return has_environment("HOME") ? get_environment("HOME").path_join(".local/share") : get_config_path();
		}
	} else if (has_environment("HOME")) {
		return get_environment("HOME").path_join(".local/share");
	} else {
		return get_config_path();
	}
}

String OS_SBC::get_cache_path() const {
	if (has_environment("XDG_CACHE_HOME")) {
		if (get_environment("XDG_CACHE_HOME").is_absolute_path()) {
			return get_environment("XDG_CACHE_HOME");
		} else {
			WARN_PRINT_ONCE("`XDG_CACHE_HOME` is a relative path. Ignoring its value and falling back to `$HOME/.cache` or `get_config_path()` per the XDG Base Directory specification.");
			return has_environment("HOME") ? get_environment("HOME").path_join(".cache") : get_config_path();
		}
	} else if (has_environment("HOME")) {
		return get_environment("HOME").path_join(".cache");
	} else {
		return get_config_path();
	}
}

void OS_SBC::set_quit_requested(bool p_quit) {
	quit_requested = p_quit;
}
// get singleton
OS_SBC *OS_SBC::get_singleton() {
	return static_cast<OS_SBC *>(OS::get_singleton());
}