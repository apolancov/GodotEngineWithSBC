
#include "display_server_sdl.h"
#include "core/input/input.h"
#include "core/input/input_event.h"
#include "os_sbc.h"
#include "sdl_map.h"

#ifdef GLES3_ENABLED
#include "drivers/gles3/rasterizer_gles3.h"
#endif

#ifdef VULKAN_ENABLED
#include "drivers/vulkan/godot_vulkan.h"
#include "rendering_context_driver_vulkan_sdl.h"
#include "servers/rendering/renderer_rd/renderer_compositor_rd.h"
#include "servers/rendering/rendering_context_driver.h"

#include <SDL_vulkan.h>
#endif

DisplayServerSDL::DisplayServerSDL(const String &p_rendering_driver, WindowMode p_mode, VSyncMode p_vsync, uint32_t p_flags, const Point2i *p_position, const Size2i &p_resolution, int p_screen, Context p_context, int64_t p_parent_window, Error &r_error) {
	rendering_driver = p_rendering_driver;
	print_line("Initializing with rendering driver: " + rendering_driver);

	print_line("Sync mode: ",
			p_vsync == DisplayServer::VSYNC_DISABLED
					? "disabled"
					: p_vsync == DisplayServer::VSYNC_ENABLED
					? "enabled"
					: p_vsync == DisplayServer::VSYNC_ADAPTIVE
					? "adaptive"
					: "mailbox");

	if (window) {
		SDL_DestroyWindow(window);
	}

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS) < 0) {
		print_line("DisplayServerSDL: Error initializing SDL: " + String(SDL_GetError()));
		ERR_PRINT("Error initializing SDL: " + String(SDL_GetError()));
		return;
	}

	ShowControllerInfo();
	int flags = SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI;

	inputHandler = Input::get_singleton();

#ifdef VULKAN_ENABLED
	if (rendering_driver == "vulkan") {
		print_line("resolution: ", p_resolution.width, "x", p_resolution.height);
		vulkan_context = memnew(RenderingContextDriverVulkanSDL);
		if (!vulkan_context) {
			printf("Vulkan context is null!\n");
			r_error = ERR_CANT_CREATE;
			return;
		}

		if (vulkan_context->initialize() != OK) {
			WARN_PRINT("Your video card drivers seem not to support the required Vulkan version, switching to GLES3.");
#ifdef GLES3_ENABLED
			print_line("Switching to GLES3 rendering driver due to Vulkan initialization failure.");
			rendering_driver = "opengl3";
			OS::get_singleton()->set_current_rendering_method("gl_compatibility");
			OS::get_singleton()->set_current_rendering_driver_name(rendering_driver);
#endif // GLES3_ENABLED
		} else {
			print_line("Using Vulkan rendering driver.");
			flags |= SDL_WINDOW_VULKAN;
		}
	}
#endif
#ifdef GLES3_ENABLED
	if (rendering_driver == "opengl3") {
		print_line("Vulkan rendering driver is not compatible with your video card, using GLES3 instead.");
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
		flags |= SDL_WINDOW_OPENGL;
	}
#endif

	print_line("Video Card Driver: " + String(SDL_GetCurrentVideoDriver()));

	window = SDL_CreateWindow(
			"Godot",
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			p_resolution.width,
			p_resolution.height,
			flags);

	window_size = p_resolution;
	print_line("Window created with size: " + itos(window_size.width) + "x" + itos(window_size.height));
	print_line("SDL_Window created: " + itos((uintptr_t)window));

	if (!window) {
		print_line("Failed to create SDL window: " + String(SDL_GetError()));
		ERR_FAIL_MSG("Failed to create SDL window: " + String(SDL_GetError()));
	}

#ifdef VULKAN_ENABLED
	if (rendering_driver == "vulkan") {
		print_line("resolution: ", p_resolution.width, "x", p_resolution.height);

		RenderingContextDriverVulkanSDL::WindowPlatformData wpd;
		wpd.window = window;
		vulkan_context->window_create(DisplayServer::MAIN_WINDOW_ID, &wpd);

		//print_line("Vulkan surface ID: " + itos(vulkan_surface_id));

		print_line("Creating RenderingDevice...");
		rendering_device = memnew(RenderingDevice);
		print_line("RenderingDevice pointer: " + itos((uintptr_t)rendering_device));
		Error err = rendering_device->initialize(vulkan_context, MAIN_WINDOW_ID);
		print_line("Errors of initialize: " + itos((int)err));
		if (err != OK) {
			// Resource cleanup here...
			memdelete(rendering_device);
			print_line("Failed to initialize rendering device: rendering_device=" + itos((uintptr_t)rendering_device));
			print_line("Error initializing rendering device: " + itos((int)err));
			ERR_FAIL_MSG("Failed to initialize rendering device: " + itos((int)err));
			r_error = err;
			return;
		}
		print_line("Rendering device initialized with Vulkan");
		rendering_device->screen_create(MAIN_WINDOW_ID);
		RendererCompositorRD::make_current();
	}
#endif
#ifdef GLES3_ENABLED
	if (rendering_driver == "opengl3") {
		if (gl_context == nullptr) {
			gl_context = SDL_GL_CreateContext(window);

			if (!gl_context) {
				print_line("DisplayServerSDL: Failed to create OpenGL ES context: " + String(SDL_GetError()));
				ERR_FAIL_MSG("Failed to create OpenGL context: " + String(SDL_GetError()));
			}
			print_line("OpenGL ES context created");

			SDL_GL_MakeCurrent(window, gl_context);

			SDL_GL_SetSwapInterval(get_sdl_swap_interval(p_vsync)); // Enable VSync
			print_line("VSync set to ", (get_sdl_swap_interval(p_vsync) ? "enabled" : "disabled"));
			RasterizerGLES3::make_current(false);
			print_line("Rasterizer initialized");
		}
	}
#endif

	inputHandler->set_event_dispatch_function(&DisplayServerSDL::event_dispatch_function);

	r_error = OK;
}

// Make the dispatch function static for Input::set_event_dispatch_function
void DisplayServerSDL::event_dispatch_function(const Ref<InputEvent> &event) {
	// You may need to access a singleton or global instance here if needed.
	// For now, just call the non-static version if possible.
	DisplayServerSDL *instance = static_cast<DisplayServerSDL *>(DisplayServer::get_singleton());
	if (instance) {
		instance->dispatch_events(event);
	}
}

void DisplayServerSDL::dispatch_events(const Ref<InputEvent> &event) {
	Variant arg0 = event;
	invoke_callback_dispatch_events(input_event_callback, arg0);
}

void DisplayServerSDL::invoke_callback_dispatch_events(Callable &callback, Variant &arg0) {
	if (!callback.is_valid()) {
		return;
	}
	const Variant *args[] = { &arg0 };
	Variant return_value;
	Callable::CallError err;
	callback.callp(args, 1, return_value, err);
}

void DisplayServerSDL::ShowControllerInfo() {
	// get controller counts
	int num_joysticks = SDL_NumJoysticks();
	print_line("Number of joysticks detected: " + itos(num_joysticks));
	// Initialize joysticks
	if (num_joysticks < 0) {
		fprintf(stderr, "Could not get joystick count from SDL, Error: %s\n", SDL_GetError());
	} else {
		for (int i = 0; i < num_joysticks; ++i) {
			SDL_Joystick *joystick = SDL_JoystickOpen(i);
			if (joystick) {
				SDL_JoystickID id = SDL_JoystickInstanceID(joystick);
				joysticks[id] = joystick;
				print_line("Joystick connected: " + String(SDL_JoystickName(joystick)));
			} else {
				fprintf(stderr, "Could not open joystick %d, Error: %s\n", i, SDL_GetError());
			}
		}
	}
}

bool DisplayServerSDL::has_feature(Feature p_feature) const {
	switch (p_feature) {
		case FEATURE_MOUSE:
		case FEATURE_ICON:
		case FEATURE_SWAP_BUFFERS:
			return true;
		default:
			return false;
	}
}

void DisplayServerSDL::swap_buffers() {
#ifdef GLES3_ENABLED
	if (rendering_driver == "opengl3" && gl_context) {
		SDL_GL_SwapWindow(window);
	}
#endif
#ifdef VULKAN_ENABLED
	if (rendering_driver == "vulkan" && vulkan_context) {
	}
#endif
}

String DisplayServerSDL::get_name() const {
	return "sbc";
}

int DisplayServerSDL::get_screen_count() const {
	int num_displays = SDL_GetNumVideoDisplays();

	if (num_displays < 0) {
		fprintf(stderr, "Could not get display count from SDL, Error: %s\n", SDL_GetError());
	}

	return num_displays;
}

int DisplayServerSDL::get_primary_screen() const {
	return MAIN_WINDOW_ID; // SDL does not provide a primary screen, so we return the main window ID.
}

Point2i DisplayServerSDL::screen_get_position(int p_screen) const {
	int w, h;
	SDL_GetWindowPosition(window, &w, &h);
	return Point2i(w, h);
}

Size2i DisplayServerSDL::screen_get_size(int p_screen) const {
	int w, h;

	SDL_GetWindowSize(window, &w, &h);

	return Size2i(w, h);
}

int DisplayServerSDL::screen_get_dpi(int p_screen) const {
	if (p_screen == -1) {
		p_screen = SDL_GetWindowDisplayIndex(window);
	}

	// Invalid screen?
	ERR_FAIL_INDEX_V(p_screen, get_screen_count(), 0);

	float diagonal_dpi = 96.0f;

	if (SDL_GetDisplayDPI(p_screen, &diagonal_dpi, NULL, NULL) != 0) {
		fprintf(stderr, "Could not get the screen DPI for display %i from SDL, Error: %s\n", p_screen, SDL_GetError());
	}

	return static_cast<int>(diagonal_dpi);
}

void DisplayServerSDL::initialize_gl_context() {
	// if (gl_context == nullptr) {
	// 	gl_context = SDL_GL_CreateContext(window);
	// 	if (!gl_context) {
	// 		print_line("DisplayServerSDL: Failed to create OpenGL context: " + String(SDL_GetError()));
	// 		ERR_FAIL_MSG("Failed to create OpenGL context: " + String(SDL_GetError()));
	// 	}
	// 	//SDL_GL_SetSwapInterval(1); // Enable VSync
	// 	SDL_GL_MakeCurrent(window, gl_context);
	// 	SDL_GL_SwapWindow(window);
	// 	print_line("DisplayServerSDL: OpenGL context created");
	// }
}

void DisplayServerSDL::_destroy_window() {
#ifdef GLES3_ENABLED
	if (gl_context) {
		SDL_GL_DeleteContext(gl_context);
		gl_context = nullptr;
	}

#endif
#ifdef VULKAN_ENABLED

	if (vk_surface != VK_NULL_HANDLE && vk_instance != VK_NULL_HANDLE) {
		vkDestroySurfaceKHR(vk_instance, vk_surface, nullptr);
	}
	if (vk_instance != VK_NULL_HANDLE) {
		vkDestroyInstance(vk_instance, nullptr);
	}
	if (vulkan_context) {
		vulkan_context->window_destroy(DisplayServer::MAIN_WINDOW_ID);
		vulkan_context = nullptr;
	}

#endif
	if (window) {
		SDL_DestroyWindow(window);
		window = nullptr;
	}

	SDL_Quit();
}

void DisplayServerSDL::window_set_title(const String &p_title, WindowID p_window) {
	if (window) {
		SDL_SetWindowTitle(window, p_title.utf8().get_data());
	}
}

void DisplayServerSDL::window_set_size(const Size2i p_size, WindowID p_window) {
	window_size = p_size;
	if (window) {
		SDL_SetWindowSize(window, p_size.x, p_size.y);
	}
}

Size2i DisplayServerSDL::window_get_size(WindowID p_window) const {
	int w, h;
	SDL_GetWindowSize(window, &w, &h);

	return Size2i(w, h);
}

void DisplayServerSDL::window_set_mode(WindowMode p_mode, WindowID p_window) {
	window_mode = p_mode;
	if (!window) {
		return;
	}

	switch (p_mode) {
		case WINDOW_MODE_FULLSCREEN:
			SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
			break;
		case WINDOW_MODE_WINDOWED:
			SDL_SetWindowFullscreen(window, 0);
			break;
		default:
			break;
	}
}

DisplayServer::WindowMode DisplayServerSDL::window_get_mode(WindowID p_window) const {
	return window_mode;
}

DisplayServerSDL::~DisplayServerSDL() {
	_destroy_window();
}

void DisplayServerSDL::initialize_rendering() {
#ifdef GLES3_ENABLED

#endif
}

Vector<DisplayServer::WindowID> DisplayServerSDL::get_window_list() const {
	Vector<WindowID> list;
	list.push_back(DisplayServer::MAIN_WINDOW_ID);
	return list;
}

DisplayServer::WindowID DisplayServerSDL::get_window_at_screen_position(const Point2i &p_position) const {
	return DisplayServer::MAIN_WINDOW_ID;
}

void DisplayServerSDL::window_attach_instance_id(ObjectID p_instance, WindowID p_window) {
	attached_instance_id = p_instance;
}

ObjectID DisplayServerSDL::window_get_attached_instance_id(WindowID p_window) const {
	return attached_instance_id;
}

void DisplayServerSDL::window_set_rect_changed_callback(const Callable &p_callable, WindowID p_window) {
	rect_changed_callback = p_callable;
}

void DisplayServerSDL::window_set_window_event_callback(const Callable &p_callable, WindowID p_window) {
	window_event_callback = p_callable;
}

void DisplayServerSDL::window_set_input_event_callback(const Callable &p_callable, WindowID p_window) {
	input_event_callback = p_callable;
}

void DisplayServerSDL::window_set_input_text_callback(const Callable &p_callable, WindowID p_window) {
	input_text_callback = p_callable;
}

void DisplayServerSDL::window_set_drop_files_callback(const Callable &p_callable, WindowID p_window) {
	drop_files_callback = p_callable;
}

// Funciones de creación
DisplayServer *DisplayServerSDL::create_func(
		const String &p_rendering_driver,
		WindowMode p_mode,
		VSyncMode p_vsync,
		uint32_t p_flags,
		const Point2i *p_position,
		const Size2i &p_resolution,
		int p_screen,
		Context p_context,
		int64_t p_parent_window,
		Error &r_error) {
	DisplayServer *ds = memnew(DisplayServerSDL(
			p_rendering_driver,
			p_mode,
			p_vsync,
			p_flags,
			p_position,
			p_resolution,
			p_screen,
			p_context,
			p_parent_window,
			r_error));

	if (r_error != OK) {
		print_line("DisplayServerSDL: Error creating display server: " + itos(r_error));
		ERR_PRINT("Error creating display server: " + itos(r_error));
		memdelete(ds);
		return nullptr;
	}
	return ds;
}

Rect2i DisplayServerSDL::screen_get_usable_rect(int p_screen) const {
	Rect2i usable_rect = Rect2i(Point2i(0, 0), window_size);
	return usable_rect;
}

float DisplayServerSDL::screen_get_refresh_rate(int p_screen) const {
	SDL_DisplayMode mode;
	if (SDL_GetCurrentDisplayMode(0, &mode)) {
		return 60.0f;
	}
	return (float)mode.refresh_rate;
}

int DisplayServerSDL::window_get_current_screen(WindowID p_window) const {
	int current_display = SDL_GetWindowDisplayIndex(window);

	if (current_display < 0) {
		fprintf(stderr, "Could not get the current display from SDL, Error: %s\n", SDL_GetError());
	}

	return current_display;
}

void DisplayServerSDL::window_set_current_screen(int p_screen, WindowID p_window) {
	int screen_count = DisplayServerSDL::get_screen_count();
	if (p_screen >= screen_count) {
		return;
	}

	if (window_mode == WINDOW_MODE_FULLSCREEN) {
		Point2i position = DisplayServerSDL::window_get_position(p_screen);

		SDL_SetWindowPosition(window, position.x, position.y);
	} else {
		if (p_screen != DisplayServerSDL::window_get_current_screen(p_window)) {
			Point2i position = DisplayServerSDL::window_get_position(p_screen);
			SDL_SetWindowPosition(window, position.x, position.y);
		}
	}
}

Point2i DisplayServerSDL::window_get_position(WindowID p_window) const {
	if (window) {
		int x, y;
		SDL_GetWindowPosition(window, &x, &y);
		return Point2i(x, y);
	}
	return Point2i(0, 0);
}

Point2i DisplayServerSDL::window_get_position_with_decorations(WindowID p_window) const {
	if (window) {
		int x, y;
		SDL_GetWindowPosition(window, &x, &y);
		return Point2i(x, y);
	}
	return Point2i(0, 0);
}

void DisplayServerSDL::window_set_position(const Point2i &p_position, WindowID p_window) {
	if (window) {
		SDL_SetWindowPosition(window, p_position.x, p_position.y);
	}
}

void DisplayServerSDL::window_set_transient(WindowID p_window, WindowID p_parent) {
}

void DisplayServerSDL::window_set_max_size(const Size2i p_size, WindowID p_window) {
	if (window) {
		SDL_SetWindowMaximumSize(window, p_size.x, p_size.y);
	}
}

Size2i DisplayServerSDL::window_get_max_size(WindowID p_window) const {
	if (window) {
		int width, height;
		SDL_GetWindowMaximumSize(window, &width, &height);
		return Size2i(width, height);
	}
	return Size2i(0, 0);
}

void DisplayServerSDL::window_set_min_size(const Size2i p_size, WindowID p_window) {
	if (window) {
		SDL_SetWindowMinimumSize(window, p_size.x, p_size.y);
	}
}

Size2i DisplayServerSDL::window_get_min_size(WindowID p_window) const {
	if (window) {
		int width, height;
		SDL_GetWindowMinimumSize(window, &width, &height);
		return Size2i(width, height);
	}
	return Size2i(0, 0);
}

Size2i DisplayServerSDL::window_get_size_with_decorations(WindowID p_window) const {
	if (window) {
		int width, height;
		SDL_GetWindowSize(window, &width, &height);
		return Size2i(width, height);
	}
	return Size2i(0, 0);
}

bool DisplayServerSDL::window_is_maximize_allowed(WindowID p_window) const {
	if (window) {
		return SDL_GetWindowFlags(window) & SDL_WINDOW_MAXIMIZED;
	}
	return false;
}

void DisplayServerSDL::window_set_flag(WindowFlags p_flag, bool p_enabled, WindowID p_window) {
	if (window) {
		switch (p_flag) {
			case WINDOW_FLAG_RESIZE_DISABLED:
				SDL_SetWindowResizable(window, p_enabled ? SDL_TRUE : SDL_FALSE);
				break;
			case WINDOW_FLAG_MAX:
				SDL_SetWindowFullscreen(window, p_enabled ? SDL_WINDOW_FULLSCREEN : 0);
				break;
			case WINDOW_FLAG_BORDERLESS:
				SDL_SetWindowBordered(window, p_enabled ? SDL_TRUE : SDL_FALSE);
				break;
			default:
				break;
		}
	}
}

bool DisplayServerSDL::window_get_flag(WindowFlags p_flag, WindowID p_window) const {
	if (window) {
		switch (p_flag) {
			case WINDOW_FLAG_RESIZE_DISABLED:
				return SDL_GetWindowFlags(window) & SDL_WINDOW_RESIZABLE;
			case WINDOW_FLAG_MAX:
				return SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN;
			case WINDOW_FLAG_BORDERLESS:
				return !(SDL_GetWindowFlags(window) & SDL_WINDOW_BORDERLESS);
			default:
				break;
		}
	}
	return false;
}

void DisplayServerSDL::window_request_attention(WindowID p_window) {
	if (window) {
		SDL_SetWindowInputFocus(window);
	}
}

void DisplayServerSDL::window_move_to_foreground(WindowID p_window) {
	if (window) {
		SDL_RaiseWindow(window);
	}
}

bool DisplayServerSDL::window_is_focused(WindowID p_window) const {
	if (window) {
		return SDL_GetWindowFlags(window) & SDL_WINDOW_INPUT_FOCUS;
	}
	return false;
}

bool DisplayServerSDL::window_can_draw(WindowID p_window) const {
	if (window) {
		return SDL_GetWindowFlags(window) & SDL_WINDOW_SHOWN;
	}
	return false;
}

bool DisplayServerSDL::can_any_window_draw() const {
	if (window) {
		return SDL_GetWindowFlags(window) & SDL_WINDOW_SHOWN;
	}
	return false;
}
int c = 0;
void DisplayServerSDL::process_events() {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_TEXTINPUT:
				_process_sdl_text_input(event.text);
				break;
			case SDL_KEYUP:
			case SDL_KEYDOWN:
				_process_sdl_key_event(event.key);
				break;
			case SDL_MOUSEMOTION:
			case SDL_MOUSEWHEEL:
			case SDL_MOUSEBUTTONUP:
			case SDL_MOUSEBUTTONDOWN:
				_process_sdl_mouse_event(event);
				break;
			case SDL_JOYAXISMOTION:
			case SDL_JOYHATMOTION:
			case SDL_JOYBUTTONDOWN:
			case SDL_JOYBUTTONUP:
			case SDL_JOYDEVICEADDED:
			case SDL_JOYDEVICEREMOVED:
				_process_sdl_joystick_event(event);
				break;
			case SDL_CONTROLLERAXISMOTION:
			case SDL_CONTROLLERBUTTONDOWN:
			case SDL_CONTROLLERBUTTONUP:
			case SDL_CONTROLLERDEVICEADDED:
			case SDL_CONTROLLERDEVICEREMOVED:
			case SDL_CONTROLLERDEVICEREMAPPED:
				_process_sdl_gamecontroller_event(event);
				break;
			case SDL_FINGERDOWN:
			case SDL_FINGERUP:
			case SDL_FINGERMOTION:
				_process_sdl_touch_event(event);
			case SDL_QUIT:
				OS_SBC::get_singleton()->set_quit_requested(true);
				break;
		}
	}

	inputHandler->flush_buffered_events();
}

Vector<String> DisplayServerSDL::get_rendering_drivers_func() {
	Vector<String> drivers;

#ifdef VULKAN_ENABLED
	drivers.push_back("vulkan");
#endif
#ifdef GLES3_ENABLED
	drivers.push_back("opengl3");
	drivers.push_back("opengl3_es");
#endif

	return drivers;
}

void DisplayServerSDL::register_sbc_driver() {
	print_line("Registering SBC driver");
	register_create_function("SDL2", create_func, get_rendering_drivers_func);
	print_line("Available rendering drivers:", get_create_function_count());

	for (int i = 0; i < get_create_function_count(); i++) {
		print_line("Driver " + String::num(i) + ": " + get_create_function_name(i));
	}
}

// SDL_Window *DisplayServerSDL::get_window() {
// #ifdef GLES3_ENABLED
// 	if (rendering_driver == "opengl3" && gl_context) {
// 		return window;
// 	}
// #endif
// 	//#ifdef VULKAN_ENABLED
// 	if (rendering_driver == "vulkan" && vulkan_context) {
// 		return vulkan_context->get_window();
// 	}
// 	//#endif
// 	print_error("DisplayServerSDL::get_window: No valid rendering driver or context available.");
// 	return nullptr;
// }

void DisplayServerSDL::window_set_vsync_mode(VSyncMode p_vsync_mode, WindowID p_window) {
#ifdef GLES3_ENABLED
	if (rendering_driver == "opengl3" && gl_context) {
		int swap_interval = get_sdl_swap_interval(p_vsync_mode);
		SDL_GL_SetSwapInterval(swap_interval);
		print_line("VSync mode set to ", swap_interval == 0 ? "disabled" : (swap_interval == 1 ? "enabled" : "adaptive"));
	}
#endif

#ifdef VULKAN_ENABLED

	if (rendering_driver == "vulkan" && vulkan_context) {
		const char *safe_mode = getenv("GODOT_SAFE_MODE");
		if (safe_mode && strcmp(safe_mode, "1") == 0) {
			// In safe mode, we do not change VSync settings.
			print_line("Safe mode is enabled, skipping VSync mode change.");
		} else {
			vulkan_context->window_set_vsync_mode(p_window, p_vsync_mode);
			print_line("VSync mode set to ", p_vsync_mode == VSYNC_DISABLED ? "disabled" : (p_vsync_mode == VSYNC_ENABLED ? "enabled" : "adaptive"));
		}
		// NOTE: In Vulkan, changing VSync requires recreating the swapchain with a different presentation mode.
		// You should mark the surface for recreation, typically with surface_set_needs_resize or similar.
		// This is important because Vulkan does not allow dynamic VSync changes like OpenGL.
	}
#endif
}

DisplayServer::VSyncMode DisplayServerSDL::window_get_vsync_mode(WindowID p_window) const {
#ifdef GLES3_ENABLED
	if (rendering_driver == "opengl3" && gl_context) {
		int interval = SDL_GL_GetSwapInterval();
		switch (interval) {
			case 0:
				return VSYNC_DISABLED;
			case 1:
				return VSYNC_ENABLED;
			case -1:
				return VSYNC_ADAPTIVE;
			default:
				return VSYNC_ENABLED;
		}
	}
#endif
#ifdef VULKAN_ENABLED
	if (rendering_driver == "vulkan" && vulkan_context) {
		return vulkan_context->window_get_vsync_mode(MAIN_WINDOW_ID);
	}
#endif
	return VSYNC_DISABLED;
}

void DisplayServerSDL::set_icon(const Ref<Image> &p_icon) {
	SDL_Surface *icon = SDL_CreateRGBSurfaceWithFormat(0, 10, 10, 32, SDL_PIXELFORMAT_ABGR8888);
	if (!icon) {
		return;
	}
	//SDL_LockSurface(icon);
	//memcpy(icon->pixels, data, width * height * 4);
	//SDL_UnlockSurface(icon);
	//SDL_SetWindowIcon(window, icon);
	//SDL_FreeSurface(icon);
}

void DisplayServerSDL::_process_sdl_key_event(const SDL_KeyboardEvent &key_event) {
	Ref<InputEventKey> ev; // = memnew(InputEventKey);
	ev.instantiate();
	bool pressed = (key_event.type == SDL_KEYDOWN);

	Key gd_key = sdl2godot_keycode(key_event.keysym.sym); // lógico
	Key gd_physical_key = sdl2godot_physical_keycode(key_event.keysym.scancode);
	KeyLocation location = KeyLocation::UNSPECIFIED;

	if (gd_key == Key::NONE && gd_physical_key == Key::NONE /* && unicode == 0 */) {
		return; // discards invalid event
	}
	if (gd_key == Key::NONE) {
		gd_key = gd_physical_key; // fallback solo si el lógico falla
	}

	Uint32 window_id = DisplayServer::MAIN_WINDOW_ID; // Default to main window ID

	ev->set_pressed(pressed);
	ev->set_echo(key_event.repeat != 0);
	ev->set_keycode(gd_key);
	ev->set_location(location);
	ev->set_physical_keycode(gd_key);

	ev->set_window_id(window_id);
	ev->set_key_label(gd_key); // Use the keycode name as label
	ev->set_shift_pressed((key_event.keysym.mod & KMOD_SHIFT) != 0);
	ev->set_alt_pressed((key_event.keysym.mod & KMOD_ALT) != 0);
	ev->set_ctrl_pressed((key_event.keysym.mod & KMOD_CTRL) != 0);
	ev->set_meta_pressed((key_event.keysym.mod & KMOD_GUI) != 0);

	inputHandler->parse_input_event(ev);
}

void DisplayServerSDL::_process_sdl_mouse_event(const SDL_Event &event) {
	Ref<InputEvent> ie;
	switch (event.type) {
		case SDL_MOUSEMOTION: {
			last_mouse_pos = Vector2(event.motion.x, event.motion.y);
			Ref<InputEventMouseMotion> mouse_motion;
			mouse_motion.instantiate();
			mouse_motion->set_position(last_mouse_pos);
			mouse_motion->set_relative(Vector2(event.motion.xrel, event.motion.yrel));
			mouse_motion->set_button_mask(inputHandler->get_mouse_button_mask());
			mouse_motion->set_pressure(1.0);
			ie = mouse_motion;
			break;
		}
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP: {
			last_mouse_pos = Vector2(event.button.x, event.button.y);
			Ref<InputEventMouseButton> mouse_button;
			mouse_button.instantiate();
			mouse_button->set_position(last_mouse_pos);
			mouse_button->set_button_index(static_cast<MouseButton>(event.button.button));
			mouse_button->set_pressed(event.type == SDL_MOUSEBUTTONDOWN);
			mouse_button->set_double_click(event.button.clicks == 2);
			ie = mouse_button;
			break;
		}
		case SDL_MOUSEWHEEL: {
			// Usar la última posición conocida
			if (event.wheel.y != 0) {
				Ref<InputEventMouseButton> mouse_wheel_v;
				mouse_wheel_v.instantiate();
				mouse_wheel_v->set_position(last_mouse_pos);
				mouse_wheel_v->set_button_index(event.wheel.y > 0 ? MouseButton::WHEEL_UP : MouseButton::WHEEL_DOWN);
				mouse_wheel_v->set_pressed(true);
				inputHandler->parse_input_event(mouse_wheel_v);
			}
			if (event.wheel.x != 0) {
				Ref<InputEventMouseButton> mouse_wheel_h;
				mouse_wheel_h.instantiate();
				mouse_wheel_h->set_position(last_mouse_pos);
				mouse_wheel_h->set_button_index(event.wheel.x > 0 ? MouseButton::WHEEL_RIGHT : MouseButton::WHEEL_LEFT);
				mouse_wheel_h->set_pressed(true);
				inputHandler->parse_input_event(mouse_wheel_h);
				break;
			}
		}
	}

	if (ie.is_valid()) {
		inputHandler->parse_input_event(ie);
	}
}

void DisplayServerSDL::_process_sdl_text_input(const SDL_TextInputEvent &text_event) {
	String utf8_text = String::utf8(text_event.text);

	// Process each character in the UTF-8 string
	if (utf8_text.is_empty()) {
		return; // No text to process
	}
	for (int i = 0; i < utf8_text.length(); ++i) {
		char32_t codepoint = utf8_text.unicode_at(i);

		Ref<InputEventKey> ev;
		ev.instantiate();
		ev->set_unicode(codepoint);
		ev->set_pressed(true);
		ev->set_echo(false);
		ev->set_keycode(Key(0)); // No physical keycode for pure text
		ev->set_physical_keycode(Key(0));

		inputHandler->parse_input_event(ev);
	}
}

void DisplayServerSDL::_process_sdl_touch_event(const SDL_Event &event) {
	switch (event.type) {
		case SDL_FINGERDOWN: {
			Ref<InputEventScreenTouch> touch;
			touch.instantiate();
			int win_w = 0, win_h = 0;
			SDL_GetWindowSize(window, &win_w, &win_h);
			Vector2 pos(event.tfinger.x * win_w, event.tfinger.y * win_h);
			touch->set_index(event.tfinger.fingerId);
			touch->set_position(pos);
			touch->set_pressed(true);
			inputHandler->parse_input_event(touch);
			break;
		}
		case SDL_FINGERUP: {
			Ref<InputEventScreenTouch> touch;
			touch.instantiate();
			int win_w = 0, win_h = 0;
			SDL_GetWindowSize(window, &win_w, &win_h);
			Vector2 pos(event.tfinger.x * win_w, event.tfinger.y * win_h);
			touch->set_index(event.tfinger.fingerId);
			touch->set_position(pos);
			touch->set_pressed(false);
			inputHandler->parse_input_event(touch);
			break;
		}
		case SDL_FINGERMOTION: {
			Ref<InputEventScreenDrag> drag;
			drag.instantiate();
			int win_w = 0, win_h = 0;
			SDL_GetWindowSize(window, &win_w, &win_h);
			Vector2 pos(event.tfinger.x * win_w, event.tfinger.y * win_h);
			Vector2 rel(event.tfinger.dx * win_w, event.tfinger.dy * win_h);
			drag->set_index(event.tfinger.fingerId);
			drag->set_position(pos);
			drag->set_relative(rel);
			drag->set_pressure(event.tfinger.pressure);
			inputHandler->parse_input_event(drag);
			break;
		}
	}
}

void DisplayServerSDL::_process_sdl_joystick_event(const SDL_Event &event) {
	// avoid processing events from GameControllers
	SDL_JoystickID joystick_instance_id = 0;
	switch (event.type) {
		case SDL_JOYAXISMOTION:
			joystick_instance_id = event.jaxis.which;
			break;
		case SDL_JOYBUTTONDOWN:
		case SDL_JOYBUTTONUP:
			joystick_instance_id = event.jbutton.which;
			break;
		case SDL_JOYHATMOTION:
			joystick_instance_id = event.jhat.which;
			break;
		case SDL_JOYDEVICEADDED:
		case SDL_CONTROLLERDEVICEADDED:
			// handled below
			break;
		default:
			return;
	}
	// ignore when the joystick is a GameController
	if (gamecontroller_ids.count(joystick_instance_id)) {
		return;
	}

	switch (event.type) {
		case SDL_JOYAXISMOTION: {
			Ref<InputEventJoypadMotion> motion;
			motion.instantiate();
			motion->set_device(joystick_instance_id); // Joystick instance ID
			motion->set_axis(static_cast<JoyAxis>(event.jaxis.axis)); // Axis index
			float value = event.jaxis.value / 32767.0f;
			if (value > 1.0f) {
				value = 1.0f;
			}
			if (value < -1.0f) {
				value = -1.0f;
			}
			motion->set_axis_value(value);
			inputHandler->parse_input_event(motion);
			break;
		}
		case SDL_JOYBUTTONDOWN:
		case SDL_JOYBUTTONUP: {
			Ref<InputEventJoypadButton> button;
			button.instantiate();
			button->set_device(event.jbutton.which); // Joystick instance ID
			button->set_button_index(static_cast<JoyButton>(event.jbutton.button)); // Button index
			button->set_pressed(event.type == SDL_JOYBUTTONDOWN);
			button->set_pressure(event.type == SDL_JOYBUTTONDOWN ? 1.0f : 0.0f);
			inputHandler->parse_input_event(button);
			break;
		}
		case SDL_JOYHATMOTION: {
			int device = event.jhat.which;
			uint8_t value = event.jhat.value;
			struct {
				uint8_t flag;
				int btn;
			} dirs[] = {
				{ SDL_HAT_UP, 12 }, // DPAD_UP
				{ SDL_HAT_RIGHT, 15 }, // DPAD_RIGHT
				{ SDL_HAT_DOWN, 13 }, // DPAD_DOWN
				{ SDL_HAT_LEFT, 14 }, // DPAD_LEFT
			};
			for (auto &dir : dirs) {
				bool pressed = (value & dir.flag) != 0;
				Ref<InputEventJoypadButton> hat_btn;
				hat_btn.instantiate();
				hat_btn->set_device(device);
				hat_btn->set_button_index(static_cast<JoyButton>(dir.btn));
				hat_btn->set_pressed(pressed);
				hat_btn->set_pressure(pressed ? 1.0f : 0.0f);
				inputHandler->parse_input_event(hat_btn);
			}
			break;
		}
		case SDL_JOYDEVICEADDED: {
			// ADAPTADO: Solo abrir Joystick si NO es GameController
			int idx = event.jdevice.which;
			if (!SDL_IsGameController(idx)) {
				SDL_JoystickOpen(idx);
			}
			break;
		}
		case SDL_CONTROLLERDEVICEADDED: {
			// ADAPTADO: Abrir GameController y registrar el ID
			int idx = event.cdevice.which;
			if (SDL_IsGameController(idx)) {
				SDL_GameController *gc = SDL_GameControllerOpen(idx);
				if (gc) {
					SDL_JoystickID id = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(gc));
					gamecontroller_ids.insert(id);
				}
			}
			break;
		}
	}
}

void DisplayServerSDL::_process_sdl_gamecontroller_event(const SDL_Event &event) {
	switch (event.type) {
		case SDL_CONTROLLERAXISMOTION: {
			Ref<InputEventJoypadMotion> motion;
			motion.instantiate();
			motion->set_device(event.caxis.which); // Controller instance ID
			motion->set_axis(static_cast<JoyAxis>(event.caxis.axis));
			float value = event.caxis.value / 32767.0f;
			if (value > 1.0f) {
				value = 1.0f;
			}
			if (value < -1.0f) {
				value = -1.0f;
			}
			motion->set_axis_value(value);
			inputHandler->parse_input_event(motion);
			break;
		}
		case SDL_CONTROLLERBUTTONDOWN:
		case SDL_CONTROLLERBUTTONUP: {
			Ref<InputEventJoypadButton> button;
			button.instantiate();
			button->set_device(event.cbutton.which); // Controller instance ID
			button->set_button_index(static_cast<JoyButton>(event.cbutton.button));
			button->set_pressed(event.type == SDL_CONTROLLERBUTTONDOWN);
			button->set_pressure(event.type == SDL_CONTROLLERBUTTONDOWN ? 1.0f : 0.0f);
			inputHandler->parse_input_event(button);
			break;
		}
		case SDL_CONTROLLERDEVICEADDED: {
			// Open and register the GameController
			int idx = event.cdevice.which;
			if (SDL_IsGameController(idx)) {
				SDL_GameController *gc = SDL_GameControllerOpen(idx);
				if (gc) {
					SDL_JoystickID id = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(gc));
					gamecontroller_ids.insert(id); // Register to avoid double handling
				}
			}
			break;
		}
		case SDL_CONTROLLERDEVICEREMOVED: {
			// clear registration and close the GameController
			SDL_JoystickID id = event.cdevice.which;
			if (gamecontroller_ids.count(id)) {
				gamecontroller_ids.erase(id);
				SDL_GameController *gc = SDL_GameControllerFromInstanceID(id);
				if (gc) {
					SDL_GameControllerClose(gc);
					print_line("DisplayServerSDL: GameController closed and removed, ID: " + String::num(id));
				}
			}
			break;
		}
		case SDL_CONTROLLERDEVICEREMAPPED: {
			print_line("DisplayServerSDL: GameController remapped");
			break;
		}
	}
}

// When a device is added (either GameController or Joystick)
void DisplayServerSDL::_handle_device_added(int device_index) {
	if (SDL_IsGameController(device_index)) {
		SDL_GameController *gc = SDL_GameControllerOpen(device_index);
		if (gc) {
			SDL_JoystickID id = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(gc));
			gamecontroller_ids.insert(id);
			controllers[id] = gc;
			print_line("GameController connected, ID: ", id);
		}
	} else {
		SDL_Joystick *joy = SDL_JoystickOpen(device_index);
		if (joy) {
			SDL_JoystickID id = SDL_JoystickInstanceID(joy);
			joysticks[id] = joy;
			print_line("Joystick connected, ID: ", id);
		}
	}
}

// When a device is disconnected
void DisplayServerSDL::_handle_device_removed(SDL_JoystickID joystick_instance_id) {
	if (gamecontroller_ids.count(joystick_instance_id)) {
		// It's a GameController
		auto it = controllers.find(joystick_instance_id);
		if (it != controllers.end()) {
			SDL_GameControllerClose(it->second);
			controllers.erase(it);
		}
		gamecontroller_ids.erase(joystick_instance_id);
		print_line("GameController disconnected, ID: ", joystick_instance_id);
	} else {
		// It's a classic Joystick
		auto it = joysticks.find(joystick_instance_id);
		if (it != joysticks.end()) {
			SDL_JoystickClose(it->second);
			joysticks.erase(it);
		}
		print_line("Joystick disconnected, ID: ", joystick_instance_id);
	}
}

int DisplayServerSDL::get_sdl_swap_interval(DisplayServer::VSyncMode p_vsync_mode) {
	switch (p_vsync_mode) {
		case DisplayServer::VSYNC_ENABLED:
			return 1;
		case DisplayServer::VSYNC_DISABLED:
			return 0;
		case DisplayServer::VSYNC_ADAPTIVE:
			return -1;
		case DisplayServer::VSYNC_MAILBOX:
			return -2;
		default:
			return 0;
	}
}

Point2i DisplayServerSDL::mouse_get_position() const {
	Point2i mouse_pos;
	if (!window) {
		return Point2i(0, 0); // Return a default position if the window is not initialized
	}

	int mouse_x, mouse_y;
	SDL_GetMouseState(&mouse_x, &mouse_y);
	return Point2i(mouse_x, mouse_y);
}

void DisplayServerSDL::mouse_set_mode(MouseMode p_mode) {
	if (!window) {
		return;
	}
	SDL_SetRelativeMouseMode(p_mode == MOUSE_MODE_CAPTURED ? SDL_TRUE : SDL_FALSE);
}