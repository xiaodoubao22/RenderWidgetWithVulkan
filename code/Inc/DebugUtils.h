#ifndef __DEBUG_UTILS_H__
#define __DEBUG_UTILS_H__

#include <vulkan/vulkan.h>

namespace render {
    class DebugUtils {
    public:
        DebugUtils();
        ~DebugUtils();

        static DebugUtils& GetInstance();

        void Setup(VkInstance instance);
        void Destroy(VkInstance instance);

        static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData);

        void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

    private:
        bool mIsSetup = false;

        // functions
        PFN_vkCreateDebugUtilsMessengerEXT mPfVkCreateDebugUtilsMessengerEXT = nullptr;
        PFN_vkDestroyDebugUtilsMessengerEXT mPfVkDestroyDebugUtilsMessengerEXT = nullptr;

        // instance
        VkDebugUtilsMessengerEXT mDebugUtilsMessenger = VK_NULL_HANDLE;
    };
}

#endif // !__DEBUG_UTILS_H__

