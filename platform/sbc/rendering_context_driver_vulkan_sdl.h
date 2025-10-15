#ifndef RENDERING_CONTEXT_DRIVER_VULKAN_SDL_H
#define RENDERING_CONTEXT_DRIVER_VULKAN_SDL_H

#ifdef VULKAN_ENABLED

#pragma once
//#include "drivers/vulkan/godot_vulkan.h"
#include "drivers/vulkan/rendering_context_driver_vulkan.h"
#include "drivers/vulkan/rendering_device_driver_vulkan.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

class RenderingContextDriverVulkanSDL : public RenderingContextDriverVulkan {
	VkSurfaceKHR vk_surface = VK_NULL_HANDLE;
	RenderingDeviceDriverVulkan *device_ = nullptr;

private:
	virtual const char *_get_platform_surface_extension() const override final;

protected:
	SurfaceID surface_create(const void *p_platform_data) override final;

public:
	struct WindowPlatformData {
		SDL_Window *window;
	};

	RenderingContextDriverVulkanSDL();
	~RenderingContextDriverVulkanSDL();
};

#endif // VULKAN_ENABLED

#endif // RENDERING_CONTEXT_DRIVER_VULKAN_SDL_H
