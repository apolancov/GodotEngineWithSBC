// display_server_sdl.h

#include "core/input/input.h"
#include "core/input/input_event.h"
#include "servers/display_server.h"
#include <SDL2/SDL.h>
#include <set>
#include <unordered_map>

#ifdef GLES3_ENABLED
#include "drivers/gles3/rasterizer_gles3.h"
#endif

#ifdef VULKAN_ENABLED
#include "drivers/vulkan/rendering_context_driver_vulkan.h"
#include "drivers/vulkan/rendering_device_driver_vulkan.h"
#include "servers/rendering/renderer_rd/renderer_compositor_rd.h"
#include "servers/rendering/rendering_device.h"

#include <SDL_vulkan.h>
#endif

class DisplayServerSDL : public DisplayServer {
	GDCLASS(DisplayServerSDL, DisplayServer);

private:
	ObjectID attached_instance_id;
	Callable rect_changed_callback;
	Callable window_event_callback;
	Callable input_event_callback;
	Callable input_text_callback;
	Callable drop_files_callback;

	Size2i window_size = Size2i(800, 600);
	Point2i window_position = Point2i(0, 0);
	WindowMode window_mode = WINDOW_MODE_FULLSCREEN;
	SDL_Window *window = nullptr;
	ObjectID instance_id;
	String rendering_driver;

	// Input handling
	Input *inputHandler;
	Vector2 last_mouse_pos = Vector2();
	std::set<SDL_JoystickID> gamecontroller_ids; // IDs managed by SDL_GameController
	std::unordered_map<SDL_JoystickID, SDL_GameController *> controllers; // Optional, for direct access
	std::unordered_map<SDL_JoystickID, SDL_Joystick *> joysticks; // For non-compatible ones

#ifdef GLES3_ENABLED
	SDL_GLContext gl_context = nullptr;
#endif

#ifdef VULKAN_ENABLED
	VkInstance vk_instance = VK_NULL_HANDLE;
	VkSurfaceKHR vk_surface = VK_NULL_HANDLE;
	RenderingContextDriverVulkan *vulkan_context = nullptr;
	RenderingDevice *rendering_device = nullptr;
	uint64_t vulkan_surface_id = 0;
#endif

	void _destroy_window();

public:
	//SDL_Window *get_window();
	DisplayServerSDL(const String &p_rendering_driver, WindowMode p_mode, VSyncMode p_vsync, uint32_t p_flags, const Point2i *p_position, const Size2i &p_resolution, int p_screen, Context p_context, int64_t p_parent_window, Error &r_error);
	void ShowControllerInfo();
	~DisplayServerSDL();

	static void event_dispatch_function(const Ref<InputEvent> &event);
	void dispatch_events(const Ref<InputEvent> &event);
	void invoke_callback_dispatch_events(Callable &callback, Variant &arg0);

	virtual bool has_feature(Feature p_feature) const override;
	virtual String get_name() const override;

	virtual int get_screen_count() const override;
	virtual int get_primary_screen() const override;
	virtual Point2i screen_get_position(int p_screen = SCREEN_OF_MAIN_WINDOW) const override;
	virtual Size2i screen_get_size(int p_screen = SCREEN_OF_MAIN_WINDOW) const override;
	virtual int screen_get_dpi(int p_screen = SCREEN_OF_MAIN_WINDOW) const override;

	void window_set_title(const String &p_title, WindowID p_window = MAIN_WINDOW_ID) override;
	void window_set_size(const Size2i p_size, WindowID p_window = MAIN_WINDOW_ID) override;
	Size2i window_get_size(WindowID p_window = MAIN_WINDOW_ID) const override;

	void window_set_mode(WindowMode p_mode, WindowID p_window = MAIN_WINDOW_ID) override;
	WindowMode window_get_mode(WindowID p_window = MAIN_WINDOW_ID) const override;

	virtual void swap_buffers() override;
	virtual void window_set_vsync_mode(VSyncMode p_vsync_mode, WindowID p_window = MAIN_WINDOW_ID) override;
	virtual VSyncMode window_get_vsync_mode(WindowID p_window) const override;
	int get_sdl_swap_interval(VSyncMode mode);

	virtual void set_icon(const Ref<Image> &p_icon) override;
	virtual Vector<WindowID> get_window_list() const override;
	virtual WindowID get_window_at_screen_position(const Point2i &p_position) const override;

	virtual void window_attach_instance_id(ObjectID p_instance, WindowID p_window = MAIN_WINDOW_ID) override;
	virtual ObjectID window_get_attached_instance_id(WindowID p_window = MAIN_WINDOW_ID) const override;
	virtual void window_set_rect_changed_callback(const Callable &p_callable, WindowID p_window = MAIN_WINDOW_ID) override;

	virtual void window_set_window_event_callback(const Callable &p_callable, WindowID p_window = MAIN_WINDOW_ID) override;
	virtual void window_set_input_event_callback(const Callable &p_callable, WindowID p_window = MAIN_WINDOW_ID) override;
	virtual void window_set_input_text_callback(const Callable &p_callable, WindowID p_window = MAIN_WINDOW_ID) override;

	virtual void window_set_drop_files_callback(const Callable &p_callable, WindowID p_window = MAIN_WINDOW_ID) override;

	virtual int window_get_current_screen(WindowID p_window = MAIN_WINDOW_ID) const override;
	virtual void window_set_current_screen(int p_screen, WindowID p_window = MAIN_WINDOW_ID) override;

	virtual Point2i window_get_position(WindowID p_window = MAIN_WINDOW_ID) const override;
	virtual Point2i window_get_position_with_decorations(WindowID p_window = MAIN_WINDOW_ID) const override;
	virtual void window_set_position(const Point2i &p_position, WindowID p_window = MAIN_WINDOW_ID) override;

	virtual void window_set_transient(WindowID p_window, WindowID p_parent) override;

	virtual void window_set_max_size(const Size2i p_size, WindowID p_window = MAIN_WINDOW_ID) override;
	virtual Size2i window_get_max_size(WindowID p_window = MAIN_WINDOW_ID) const override;

	virtual void window_set_min_size(const Size2i p_size, WindowID p_window = MAIN_WINDOW_ID) override;
	virtual Size2i window_get_min_size(WindowID p_window = MAIN_WINDOW_ID) const override;

	virtual Size2i window_get_size_with_decorations(WindowID p_window = MAIN_WINDOW_ID) const override;

	virtual bool window_is_maximize_allowed(WindowID p_window = MAIN_WINDOW_ID) const override;

	virtual void window_set_flag(WindowFlags p_flag, bool p_enabled, WindowID p_window = MAIN_WINDOW_ID) override;
	virtual bool window_get_flag(WindowFlags p_flag, WindowID p_window = MAIN_WINDOW_ID) const override;

	virtual void window_request_attention(WindowID p_window = MAIN_WINDOW_ID) override;
	virtual void window_move_to_foreground(WindowID p_window = MAIN_WINDOW_ID) override;
	virtual bool window_is_focused(WindowID p_window = MAIN_WINDOW_ID) const override;

	virtual bool window_can_draw(WindowID p_window = MAIN_WINDOW_ID) const override;
	virtual bool can_any_window_draw() const override;

	virtual void process_events() override;
	virtual Rect2i screen_get_usable_rect(int p_screen = SCREEN_OF_MAIN_WINDOW) const override;

	virtual float screen_get_refresh_rate(int p_screen = SCREEN_OF_MAIN_WINDOW) const override;

	// Mouse handling
	virtual Point2i mouse_get_position() const override;
	virtual void mouse_set_mode(MouseMode p_mode) override;

	// Constructor/destructor
	static DisplayServer *create_func(const String &p_rendering_driver, WindowMode p_mode, VSyncMode p_vsync, uint32_t p_flags, const Point2i *p_position, const Size2i &p_resolution, int p_screen, Context p_context, int64_t p_parent_window, Error &r_error);
	static Vector<String> get_rendering_drivers_func();
	static void register_sbc_driver();

	void initialize_rendering();
	void initialize_gl_context();

	// Input handling
	void _process_sdl_key_event(const SDL_KeyboardEvent &key_event);
	void _process_sdl_mouse_event(const SDL_Event &mouse_event);
	void _process_sdl_text_input(const SDL_TextInputEvent &text_event);
	void _process_sdl_touch_event(const SDL_Event &event);
	void _process_sdl_joystick_event(const SDL_Event &event);
	void _process_sdl_gamecontroller_event(const SDL_Event &event);
	void _handle_device_added(int device_index);
	void _handle_device_removed(SDL_JoystickID joystick_instance_id);

// Vulkan Surface access
#ifdef VULKAN_ENABLED
	VkSurfaceKHR get_vk_surface() const {
		return vk_surface;
	}
#endif
};

#ifdef GLES3_ENABLED
static void *get_gl_proc_address(const char *p_name) {
	void *ptr = SDL_GL_GetProcAddress(p_name);
	if (!ptr) {
		printf("GLAD/SDL: Cannot load function: %s\n", p_name);
	}
	return ptr;
}
#endif