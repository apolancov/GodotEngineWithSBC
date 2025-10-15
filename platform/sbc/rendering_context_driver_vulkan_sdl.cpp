#ifdef VULKAN_ENABLED

#include "rendering_context_driver_vulkan_sdl.h"
//#include "drivers/vulkan/godot_vulkan.h"
#include "drivers/vulkan/rendering_context_driver_vulkan.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

const char *RenderingContextDriverVulkanSDL::_get_platform_surface_extension() const {
	const char *driver = SDL_GetCurrentVideoDriver();
	if (!driver) {
		driver = "<null>";
	} else if (!strcmp(driver, "x11")) {
		return "VK_KHR_xlib_surface";
	} else if (!strcmp(driver, "wayland")) {
		return "VK_KHR_wayland_surface";
	} else if (!strcmp(driver, "KMSDRM")) {
		return "VK_KHR_display";
	}
	return "VK_KHR_display";
}

RenderingContextDriver::SurfaceID RenderingContextDriverVulkanSDL::surface_create(const void *p_platform_data) {
	const WindowPlatformData *wpd = (const WindowPlatformData *)(p_platform_data);
	print_line("surface_create: wpd=" + itos((uintptr_t)wpd));
	if (!wpd || !wpd->window) {
		ERR_FAIL_V_MSG(0, "Invalid WindowPlatformData or null window pointer.");
	}
	print_line("surface_create: wpd->window=" + itos((uintptr_t)(wpd->window)));
	print_line("surface_create: instance=" + itos((uintptr_t)instance_get()));

	if (!SDL_Vulkan_CreateSurface(wpd->window, instance_get(), &vk_surface)) {
		String sdl_err = SDL_GetError();
		ERR_PRINT("Failed to create Vulkan surface: " + sdl_err);
		return 0;
	}

	Surface *surface = memnew(Surface);
	surface->vk_surface = vk_surface;

	return SurfaceID(surface);
}

RenderingContextDriverVulkanSDL::RenderingContextDriverVulkanSDL() {
	// Does nothing.
}

RenderingContextDriverVulkanSDL::~RenderingContextDriverVulkanSDL() {
	// Does nothing.
}

#endif // VULKAN_ENABLED
