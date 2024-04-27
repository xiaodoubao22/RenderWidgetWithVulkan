#ifndef __SCENE_DEMO_CONFIG_H__
#define __SCENE_DEMO_CONFIG_H__

#include <vulkan/vulkan.h>
#include <string>
#include <vector>

namespace framework {
struct WindowConfig {
    // 窗口大小
    uint32_t width = 1280;
    uint32_t height = 960;
    uint32_t minWidth = 200;
    uint32_t minHeight = 200;
};

struct PhisicalDeviceConfig {
    VkPhysicalDeviceType defaultDeviceType = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
};

struct LayerConfig {
    bool enableValidationLayer = false;
    std::vector<const char*> instanceLayers = {};
    std::vector<const char*> deviceLayers = {};
};

struct ExtensionConfig {
    std::vector<const char*> instanceExtensions = { VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME };
    std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME,
    };
};

struct SwapchainConfig {
    VkSurfaceFormatKHR surfaceFormat = {};
    uint32_t imageCount = 2;
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
};

struct PresentFbConfig {
    std::vector<VkFormat> depthFormatCandidates = { VK_FORMAT_D24_UNORM_S8_UINT };
    VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
    VkFormatFeatureFlags features = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
};

struct DirectoryConfig {
    std::string dirSpvFiles = "../spv_files/";
    std::string dirResource = "../resource/";
};

struct SceneDemoConfig {
    WindowConfig window = {};
    PhisicalDeviceConfig phisicalDevice = {};
    LayerConfig layer = {};
    ExtensionConfig extension = {};
    VkPhysicalDeviceFeatures deviceFeatures = {};
    SwapchainConfig swapchain = {};
    PresentFbConfig presentFb = {};
    DirectoryConfig directory = {};
};
}   // namespace framework

#endif // !__SCENE_DEMO_CONFIG_H__
