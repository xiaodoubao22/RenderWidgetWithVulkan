#ifndef __APP_DISPATCH_TABLE__
#define __APP_DISPATCH_TABLE__

#include <stdexcept>
#include <vulkan/vulkan.h>
#include "Log.h"

#define LOAD_AND_DISPATCH_INSTANCE_FUNCTION(funcName, returnErr, ...)     \
{                                                                         \
if (pfn##funcName == nullptr && mInstance != VK_NULL_HANDLE) {            \
    pfn##funcName = reinterpret_cast<PFN_vk##funcName>(                   \
        vkGetInstanceProcAddr(mInstance, "vk" #funcName));                \
}                                                                         \
if (pfn##funcName == nullptr) {                                           \
    std::runtime_error("Get procces address vk" #funcName " failed!");    \
    return returnErr;                                                     \
}                                                                         \
return pfn##funcName(##__VA_ARGS__);                                      \
}

#define LOAD_AND_DISPATCH_DEVICE_FUNCTION(funcName, returnErr, ...)       \
{                                                                         \
if (pfn##funcName == nullptr && mDevice != VK_NULL_HANDLE) {              \
    pfn##funcName = reinterpret_cast<PFN_vk##funcName>(                   \
    vkGetDeviceProcAddr(mDevice, "vk" #funcName));                        \
}                                                                         \
if (pfn##funcName == nullptr && mInstance != VK_NULL_HANDLE) {            \
    pfn##funcName = reinterpret_cast<PFN_vk##funcName>(                   \
        vkGetInstanceProcAddr(mInstance, "vk" #funcName));                \
}                                                                         \
if (pfn##funcName == nullptr) {                                           \
    std::runtime_error("Get procces address vk" #funcName " failed!");    \
    return returnErr;                                                     \
}                                                                         \
return pfn##funcName(##__VA_ARGS__);                                      \
}

namespace render{
class AppDispatchTable {
public:
    void InitInstance(VkInstance instance) {
        mInstance = instance;
    }

    void InitDevice(VkDevice device) {
        mDevice = device;
    }

    static AppDispatchTable& GetInstance() {
        static AppDispatchTable inst;
        return inst;
    }

    void CmdSetFragmentShadingRateKHR(VkCommandBuffer commandBuffer,
        const VkExtent2D* pFragmentSize, const VkFragmentShadingRateCombinerOpKHR combinerOps[2])
    {
        LOAD_AND_DISPATCH_DEVICE_FUNCTION(CmdSetFragmentShadingRateKHR, ,
            commandBuffer, pFragmentSize, combinerOps);
        pfnCmdSetFragmentShadingRateKHR(commandBuffer, pFragmentSize, combinerOps);
    }

    VkResult GetPhysicalDeviceFragmentShadingRatesKHR(VkPhysicalDevice physicalDevice,
        uint32_t* pFragmentShadingRateCount, VkPhysicalDeviceFragmentShadingRateKHR* pFragmentShadingRates)
    {
        LOAD_AND_DISPATCH_INSTANCE_FUNCTION(GetPhysicalDeviceFragmentShadingRatesKHR, VK_ERROR_DEVICE_LOST,
            physicalDevice, pFragmentShadingRateCount, pFragmentShadingRates);
    }

private:
    VkInstance mInstance = VK_NULL_HANDLE;
    VkDevice mDevice = VK_NULL_HANDLE;

    PFN_vkCmdSetFragmentShadingRateKHR pfnCmdSetFragmentShadingRateKHR = nullptr;
    PFN_vkGetPhysicalDeviceFragmentShadingRatesKHR pfnGetPhysicalDeviceFragmentShadingRatesKHR = nullptr;


};
}

#endif __APP_DISPATCH_TABLE__
