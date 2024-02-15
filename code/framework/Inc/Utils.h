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

constexpr int FRAMEWORK_KEY_PRESS = 1;
constexpr int FRAMEWORK_KEY_RELEASE = 0;

constexpr int FRAMEWORK_KEY_UNKNOWN = -1;
constexpr int FRAMEWORK_KEY_A = 65;
constexpr int FRAMEWORK_KEY_B = 66;
constexpr int FRAMEWORK_KEY_C = 67;
constexpr int FRAMEWORK_KEY_D = 68;
constexpr int FRAMEWORK_KEY_E = 69;
constexpr int FRAMEWORK_KEY_F = 70;
constexpr int FRAMEWORK_KEY_G = 71;
constexpr int FRAMEWORK_KEY_H = 72;
constexpr int FRAMEWORK_KEY_I = 73;
constexpr int FRAMEWORK_KEY_J = 74;
constexpr int FRAMEWORK_KEY_K = 75;
constexpr int FRAMEWORK_KEY_L = 76;
constexpr int FRAMEWORK_KEY_M = 77;
constexpr int FRAMEWORK_KEY_N = 78;
constexpr int FRAMEWORK_KEY_O = 79;
constexpr int FRAMEWORK_KEY_P = 80;
constexpr int FRAMEWORK_KEY_Q = 81;
constexpr int FRAMEWORK_KEY_R = 82;
constexpr int FRAMEWORK_KEY_S = 83;
constexpr int FRAMEWORK_KEY_T = 84;
constexpr int FRAMEWORK_KEY_U = 85;
constexpr int FRAMEWORK_KEY_V = 86;
constexpr int FRAMEWORK_KEY_W = 87;
constexpr int FRAMEWORK_KEY_X = 88;
constexpr int FRAMEWORK_KEY_Y = 89;
constexpr int FRAMEWORK_KEY_Z = 90;


#endif // !__UTILS_H__

