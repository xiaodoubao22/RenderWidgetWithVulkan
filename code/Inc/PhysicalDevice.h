#ifndef __PHYSICAL_DEVICE_H__
#define __PHYSICAL_DEVICE_H__

#include <vulkan/vulkan.h>
#include <optional>
#include <functional>

namespace render {
    extern struct SwapChainSupportdetails;
    
    class PhysicalDevice {
    public:
        struct QueueFamilyIndices {
            std::optional<uint32_t> graphicsFamily;
            std::optional<uint32_t> presentFamily;
            bool IsComplete() {
                return graphicsFamily.has_value() && presentFamily.has_value();
            }
        };

    public:
        PhysicalDevice();
        ~PhysicalDevice();

        /*
         * @brief Init.
         * @param instance The instanse handle.
         * @param supportedSurface The physical device we pick must support this surface.
         */
        void Init(VkInstance instance, VkSurfaceKHR supportedSurface);

        /*
         * @brief Clean up.
         */
        void CleanUp();

        /*
         * @brief Set additional suiatble test function, it need to call before Init().
         */
        void SetAdditionalSuiatbleTestFunction(const std::function<bool(VkPhysicalDevice)>& func);

        /*
         * @brief Return true if physical device is valid.
         */
        bool IsValid() { return mPhysicalDevice != VK_NULL_HANDLE; }

        /*
         * @brief Get the VkPhysicalDevice handle.
         */
        VkPhysicalDevice Get() { return mPhysicalDevice; }

        /*
         * @brief Get Surface supported by this VkPhysicalDevice.
         */
        VkSurfaceKHR GetSupportedSurface() { return mSupportedSurface; }

        /*
         * @brief Get queue family indices.
         */
        QueueFamilyIndices GetQueueFamilyIndices() { return mQueueFamilyIndices; }

        /*
         * @brief Query SwapChainSupportdetails.
         */
        SwapChainSupportdetails QuerySwapChainSupport();

        /*
         * @brief Choose format according to features.
         */
        VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

        /*
         * @brief Find memory type according to properties.
         */
        uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    private:
        void PickPhysicalDevices();

        // tool functions
        bool IsDeviceSuatiable(VkPhysicalDevice device);
        QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);

        VkResult GetPhysicalDeviceFragmentShadingRatesKHR(VkInstance instance, VkPhysicalDevice physicalDevice,
            uint32_t* pFragmentShadingRateCount, VkPhysicalDeviceFragmentShadingRateKHR* pFragmentShadingRates) {
            auto func = (PFN_vkGetPhysicalDeviceFragmentShadingRatesKHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceFragmentShadingRatesKHR");
            if (func != nullptr) {
                return func(physicalDevice, pFragmentShadingRateCount, pFragmentShadingRates);
            }
            else {
                return VK_ERROR_EXTENSION_NOT_PRESENT;
            }
        }
    private:
        // external objects
        VkInstance mInstance = VK_NULL_HANDLE;
        VkSurfaceKHR mSupportedSurface = VK_NULL_HANDLE;

        // handle
        VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;	// 物理设备

        std::function<bool(VkPhysicalDevice)> mAdditionalSuiatbleTest = nullptr;
        QueueFamilyIndices mQueueFamilyIndices = {};
    };
}

#endif // !__PHYSICAL_DEVICE_H__
