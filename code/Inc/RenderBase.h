#ifndef __RENDER_BASE__
#define __RENDER_BASE__

#include "GraphicsDevice.h"
#include "Swapchain.h"

#include <vector>
#include <vulkan/vulkan.h>

namespace render {
    class RenderBase {
    public:
        RenderBase(window::WindowTemplate& w);
        virtual ~RenderBase();

    protected:
        void Init();
        void CleanUp();

        VkDevice GetDevice();
    private:
        void CreateInstance();

        // ----- tool functions -----
        void CheckValidationLayerSupport(bool enableValidationLayer);
        bool CheckExtensionSupport(const std::vector<const char*>& target);

    protected:
        bool mEnableValidationLayer = false;

        // ---- externel objects ----
        window::WindowTemplate& mWindow;

        // ---- basic objects ----
        VkInstance mInstance = VK_NULL_HANDLE;
        VkSurfaceKHR mSurface = VK_NULL_HANDLE;

        GraphicsDevice* mGraphicsDevice = nullptr;
        Swapchain* mSwapchain = nullptr;

        // depth resources and terget framebuffers
        VkImage mDepthImage = VK_NULL_HANDLE;
        VkDeviceMemory mDepthImageMemory = VK_NULL_HANDLE;
        VkImageView mDepthImageView = VK_NULL_HANDLE;
        std::vector<VkFramebuffer> mSwapchainFramebuffers = {};

    };
}

#endif // !__RENDER_BASE__

