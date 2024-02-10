#ifndef __UTILS_H__
#define __UTILS_H__

#include <vector>
#include <vulkan/vulkan.h>
#include <string>

namespace setting {
// 窗口大小
const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 960;

// 验证层
#ifdef NDEBUG
const bool enableValidationLayer = false;
#else
const bool enableValidationLayer = true;
#endif

// 独显/核显
const VkPhysicalDeviceType defaultDeviceType = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
//const VkPhysicalDeviceType defaultDeviceType = VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;

const VkSurfaceFormatKHR swapchainSurfaceFormat = {
    VK_FORMAT_B8G8R8A8_SRGB,
    VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT
};

const uint32_t swapchainImageCount = 2;

const VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;

const std::string dirSpvFiles = "../spv_files/";

const std::string dirTexture = "../resource/textures/";

}

namespace consts {
const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation",
};

const VkClearValue CLEAR_COLOR_WHITE_FLT = { 1.0f, 1.0f, 1.0f, 1.0f };
const VkClearValue CLEAR_COLOR_NAVY_FLT = { 0.2f, 0.3f, 0.3f, 1.0f };
const VkClearValue CLEAR_DEPTH_ONE_STENCIL_ZERO = { 1.0f, 0 };

const std::string MAIN_FUNC_NAME = "main";
}

namespace utils {
std::vector<const char*> StringToCstr(const std::vector<std::string>& strings);
std::vector<std::string> CstrToString(const std::vector<const char*>& cstrs);
void PrintStringList(const std::vector<std::string>& stringList, const std::string& head);
void PrintStringList(const std::vector<const char*>& stringList, const std::string& head);
bool CheckSupported(const std::vector<const char*>& componentList, const std::vector<const char*>& availableList);
std::vector<char> ReadFile(const std::string& filename);
}

#endif // !__UTILS_H__

