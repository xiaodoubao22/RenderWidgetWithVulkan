#ifndef __PHYSICAL_DEVICE_H__
#define __PHYSICAL_DEVICE_H__

#include <optional>
#include <functional>
#include <map>
#include <memory>

#include <vulkan/vulkan.h>

namespace framework {
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

    void Init(VkInstance instance, VkSurfaceKHR supportedSurface);

    void CleanUp();

    void SetDeviceExtensions(const std::vector<const char*>& deviceExtensions) {
        mDeviceExtensions = deviceExtensions;
    }

    std::vector<const char*>& GetDeviceExtensions() { return mDeviceExtensions; }

    bool IsValid() { return mPhysicalDevice != VK_NULL_HANDLE; }

    VkPhysicalDevice Get() { return mPhysicalDevice; }

    VkInstance GetInstance() { return mInstance; }

    VkSurfaceKHR GetSupportedSurface() { return mSupportedSurface; }

    QueueFamilyIndices GetQueueFamilyIndices() { return mQueueFamilyIndices; }

    SwapChainSupportdetails QuerySwapChainSupport();

    VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    VkSampleCountFlagBits GetMaxUsableSampleCount();

    /*
     * @brief Store a extension feature in PhysicalDevice, reference to Vulkan-Samples.
     */
    template <typename T>
    T& RequestExtensionsFeatures(VkStructureType type) {
        // If the type already exists in the map, return a casted pointer to get the extension feature struct
        auto it = mExtensionsFeatures.find(type);
        if (it != mExtensionsFeatures.end()) {
            return *static_cast<T*>(it->second.get());
        }

        // Get the extension feature
        VkPhysicalDeviceFeatures2 physicalDeviceFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2_KHR };
        T                            extension{ type };
        physicalDeviceFeatures.pNext = &extension;
        vkGetPhysicalDeviceFeatures2(mPhysicalDevice, &physicalDeviceFeatures);

        // Insert the extension feature into the extension feature map so its ownership is held
        mExtensionsFeatures.insert({ type, std::make_shared<T>(extension) });

        // Pull out the dereferenced void pointer, we can assume its type based on the template
        auto* extensionPtr = static_cast<T*>(mExtensionsFeatures.find(type)->second.get());
        if (mRequestedExtensionFeatureHeadPtr) {
            extensionPtr->pNext = mRequestedExtensionFeatureHeadPtr;
        }
        mRequestedExtensionFeatureHeadPtr = extensionPtr;
        return *extensionPtr;
    }

    /*
     * @brief return requested extension feature head pointer, reference to Vulkan-Samples.
     */
    void* GetRequestedExtensionFeatureHeadPtr() { return mRequestedExtensionFeatureHeadPtr; }

private:
    void ReadRequiredExtensions();
    void PickPhysicalDevices();

    // tool functions
    bool IsDeviceSuatiable(VkPhysicalDevice device);
    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);

private:
    // external objects
    VkInstance mInstance = VK_NULL_HANDLE;
    VkSurfaceKHR mSupportedSurface = VK_NULL_HANDLE;

    // handle
    VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;	// 物理设备

    std::vector<const char*> mDeviceExtensions = {};
    QueueFamilyIndices mQueueFamilyIndices = {};

    // request extension feature
    std::map<VkStructureType, std::shared_ptr<void>> mExtensionsFeatures = {};
    void* mRequestedExtensionFeatureHeadPtr = nullptr;
};
}   // namespace framework

#endif // !__PHYSICAL_DEVICE_H__
