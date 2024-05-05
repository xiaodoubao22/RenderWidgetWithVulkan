#ifndef __RENDER_BASE__
#define __RENDER_BASE__

#include "PhysicalDevice.h"
#include "Device.h"
#include "Swapchain.h"

#include <vector>

#include <vulkan/vulkan.h>

namespace framework {
class RenderBase {
public:
    RenderBase(window::WindowTemplate& w);
    virtual ~RenderBase();

protected:
    void Init();
    void CleanUp();

    VkDevice GetDevice() { return mDevice->Get(); }

    // inherit interface
    virtual void RequestPhysicalDeviceFeatures(PhysicalDevice* physicalDevice);

private:
    void CreateInstance();

    // ----- tool functions -----
    void CheckValidationLayerSupport(bool enableValidationLayer);
    bool CheckExtensionSupport(const std::vector<const char*>& target);

protected:
    VkFormat FindSupportedFormat();

protected:
    bool mEnableValidationLayer = false;

    // ---- externel objects ----
    window::WindowTemplate& mWindow;

    // ---- basic objects ----
    VkInstance mInstance = VK_NULL_HANDLE;
    VkSurfaceKHR mSurface = VK_NULL_HANDLE;

    PhysicalDevice* mPhysicalDevice = nullptr;
    Device* mDevice = nullptr;
    Swapchain* mSwapchain = nullptr;

    // depth resources and terget framebuffers
    VkImage mDepthImage = VK_NULL_HANDLE;
    VkDeviceMemory mDepthImageMemory = VK_NULL_HANDLE;
    VkImageView mDepthImageView = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> mSwapchainFramebuffers = {};

};
}   // namespace framework

#endif // !__RENDER_BASE__

