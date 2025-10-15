// #ifdef VULKAN_ENABLED
// #include "drivers/vulkan/rendering_context_driver_vulkan.h"
// class VulkanContextSDL : public RenderingContextDriverVulkan {
// 	GDCLASS(VulkanContextSDL, RenderingContextDriverVulkan);

// 	// SDL-specific methods.
// 	virtual const char *_get_platform_surface_extension() const override { return "VK_KHR_sdl2_surface"; }
// 	virtual bool _use_validation_layers() const override { return false; } // TODO: Implement validation layers for SDL.
// 	virtual Error _create_vulkan_instance(const VkInstanceCreateInfo *p_create_info, VkInstance *r_instance) override;

// public:
// 	VulkanContextSDL(DisplayServerSDL *p_display_server);
// 	~VulkanContextSDL();

// 	virtual void initialize() override;
// 	virtual void terminate() override;

// private:
// 	DisplayServerSDL *display_server;
// };
// #endif // VULKAN_ENABLED