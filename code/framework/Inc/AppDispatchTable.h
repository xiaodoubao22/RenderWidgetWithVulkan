#ifndef __APP_DISPATCH_TABLE__
#define __APP_DISPATCH_TABLE__

#include <vulkan/vulkan.h>
#include "Log.h"

#define DEFINE_FUNCTION(returnType, funcName, ...) returnType funcName(##__VA_ARGS__)

#define LOAD_AND_DISPATCH_INSTANCE_FUNCTION(funcName, returnErr, ...)         \
{                                                                             \
    static PFN_vk##funcName pfn##funcName = nullptr;                          \
    if (pfn##funcName == nullptr && mInstance != VK_NULL_HANDLE) {            \
        pfn##funcName = reinterpret_cast<PFN_vk##funcName>(                   \
            vkGetInstanceProcAddr(mInstance, "vk" #funcName));                \
    }                                                                         \
    if (pfn##funcName == nullptr) {                                           \
        LOGE("Get procces address vk" #funcName " failed!");                  \
        return returnErr;                                                     \
    }                                                                         \
    return pfn##funcName(##__VA_ARGS__);                                      \
}

#define LOAD_AND_DISPATCH_DEVICE_FUNCTION(funcName, returnErr, ...)           \
{                                                                             \
    static PFN_vk##funcName pfn##funcName = nullptr;                          \
    if (pfn##funcName == nullptr && mDevice != VK_NULL_HANDLE) {              \
        pfn##funcName = reinterpret_cast<PFN_vk##funcName>(                   \
        vkGetDeviceProcAddr(mDevice, "vk" #funcName));                        \
    }                                                                         \
    if (pfn##funcName == nullptr && mInstance != VK_NULL_HANDLE) {            \
        pfn##funcName = reinterpret_cast<PFN_vk##funcName>(                   \
            vkGetInstanceProcAddr(mInstance, "vk" #funcName));                \
    }                                                                         \
    if (pfn##funcName == nullptr) {                                           \
        LOGE("Get procces address vk" #funcName " failed!");                  \
        return returnErr;                                                     \
    }                                                                         \
    return pfn##funcName(##__VA_ARGS__);                                      \
}

namespace framework {
class AppInstanceDispatchTable {
public:
    void InitInstance(VkInstance instance) {
        mInstance = instance;
    }

    static AppInstanceDispatchTable& GetInstance() {
        static AppInstanceDispatchTable inst;
        return inst;
    }

    DEFINE_FUNCTION(VkResult, GetPhysicalDeviceFragmentShadingRatesKHR, VkPhysicalDevice physicalDevice, uint32_t* pFragmentShadingRateCount, VkPhysicalDeviceFragmentShadingRateKHR* pFragmentShadingRates)
    LOAD_AND_DISPATCH_INSTANCE_FUNCTION(GetPhysicalDeviceFragmentShadingRatesKHR, VK_ERROR_INITIALIZATION_FAILED, physicalDevice, pFragmentShadingRateCount, pFragmentShadingRates)

private:
    VkInstance mInstance = VK_NULL_HANDLE;
};

class AppDeviceDispatchTable {
public:
    void InitDevice(VkInstance instance, VkDevice device) {
        mInstance = instance;
        mDevice = device;
    }

    static AppDeviceDispatchTable& GetInstance() {
        static AppDeviceDispatchTable inst;
        return inst;
    }

    DEFINE_FUNCTION(void, CmdSetFragmentShadingRateKHR, VkCommandBuffer commandBuffer, const VkExtent2D* pFragmentSize, const VkFragmentShadingRateCombinerOpKHR combinerOps[2])
    LOAD_AND_DISPATCH_DEVICE_FUNCTION(CmdSetFragmentShadingRateKHR, void(), commandBuffer, pFragmentSize, combinerOps)

private:
    VkDevice mDevice = VK_NULL_HANDLE;
    VkInstance mInstance = VK_NULL_HANDLE;
};
}   // namespace framework
#endif // !__APP_DISPATCH_TABLE__
