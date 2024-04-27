#include "RenderBase.h"

#include <stdexcept>
#include <array>
#include <Log.h>

#include "Utils.h"
#include "DebugUtils.h"
#include "WindowTemplate.h"
#include "AppDispatchTable.h"
#include "SceneDemoDefs.h"

namespace framework {
RenderBase::RenderBase(window::WindowTemplate& w) : mWindow(w)
{
    mPhysicalDevice = new PhysicalDevice();
    mDevice = new Device();
    mSwapchain = new Swapchain();
}

RenderBase::~RenderBase() {
    delete mPhysicalDevice;
    delete mDevice;
    delete mSwapchain;
}

void RenderBase::Init() {
    CheckValidationLayerSupport(GetConfig().layer.enableValidationLayer);

    // instance
    CreateInstance();
    AppInstanceDispatchTable::GetInstance().InitInstance(mInstance);

    // debug utils
    if (GetConfig().layer.enableValidationLayer) {
        DebugUtils::GetInstance().Setup(mInstance);
    }
        
    // surface
    mSurface = mWindow.CreateSurface(mInstance);
        
    // physical device
    mPhysicalDevice->Init(mInstance, mSurface);

    // device
    if (!mPhysicalDevice->IsValid()) {
        throw std::runtime_error("physical device invalid!");
    }
    RequestPhysicalDeviceFeatures(mPhysicalDevice);
    mDevice->Init(mPhysicalDevice);
    AppDeviceDispatchTable::GetInstance().InitDevice(mInstance, mDevice->Get());

    // swapchain
    mSwapchain->Init(mPhysicalDevice, mDevice, mWindow.GetWindowExtent(), mSurface);
}

void RenderBase::CleanUp() {
    // destroy basic objects
    mSwapchain->CleanUp();
    mDevice->CleanUp();
    mPhysicalDevice->CleanUp();
    vkDestroySurfaceKHR(mInstance, mSurface, nullptr);

    if (GetConfig().layer.enableValidationLayer) {
        DebugUtils::GetInstance().Destroy(mInstance);
    }

    vkDestroyInstance(mInstance, nullptr);
}

void RenderBase::RequestPhysicalDeviceFeatures(PhysicalDevice* physicalDevice) {
    LOGI("Did not request any physical device features");
    return;
}

void RenderBase::CreateInstance() {
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2;

    // 检查需要的拓展
    LOGI("--------- check extensions used by instance ----------");
    auto extensions = mWindow.QueryWindowRequiredExtensions();
    if (mEnableValidationLayer){
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    std::vector<const char*>& otherExtensions = GetConfig().extension.instanceExtensions;
    extensions.insert(extensions.end(), otherExtensions.begin(), otherExtensions.end());
    utils::PrintStringList(extensions, "enable extensions:");
    if (!CheckExtensionSupport(extensions)) {
        throw std::runtime_error("extension not all supported!");
    }
    else {
        LOGI("-------- instance extensions are all supported --------");
    }

    // add layers
    std::vector<const char*> layers = {};
    if (mEnableValidationLayer) {
        layers.insert(layers.end(), consts::validationLayers.begin(), consts::validationLayers.end());
    }
    std::vector<const char*>& otherlayers = GetConfig().layer.instanceLayers;
    layers.insert(layers.end(), otherlayers.begin(), otherlayers.end());

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = extensions.size();
    createInfo.ppEnabledExtensionNames = extensions.size() == 0 ? nullptr : extensions.data();
    createInfo.enabledLayerCount = layers.size();
    createInfo.ppEnabledLayerNames = layers.size() == 0 ? nullptr : layers.data();
    createInfo.pNext = nullptr;

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (mEnableValidationLayer) {
        framework::DebugUtils::GetInstance().PopulateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }

    if (vkCreateInstance(&createInfo, nullptr, &mInstance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }
}

void RenderBase::CheckValidationLayerSupport(bool enableValidationLayer) {
    if (enableValidationLayer == false) {
        LOGI("validation layer disabled");
        mEnableValidationLayer = false;
        return;
    }
    // 获取支持的层
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    // 提取名称
    std::vector<const char*> availableLayerNames;
    for (VkLayerProperties& layer : availableLayers) {
        availableLayerNames.push_back(layer.layerName);
    }

    // 检查
    utils::PrintStringList(consts::validationLayers, "validationLayers:");
    utils::PrintStringList(availableLayerNames, "availableLayers:");
    if (utils::CheckSupported(consts::validationLayers, availableLayerNames)) {
        LOGI("validation layers are all supported");
        mEnableValidationLayer = true;
    }
    else {
        LOGE("validation layer enabled but not supported");
        mEnableValidationLayer = false;
    }
    return;
}

bool RenderBase::CheckExtensionSupport(const std::vector<const char*>& target) {
    // 获取支持的拓展
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    // 提取名称
    std::vector<const char*> supportExtensionNames;
    for (VkExtensionProperties& extension : extensions) {
        supportExtensionNames.push_back(extension.extensionName);
    }
    utils::PrintStringList(supportExtensionNames, "support extensions names:");

    // 检查
    return utils::CheckSupported(target, supportExtensionNames);
}

VkFormat RenderBase::FindSupportedFormat() {
    if (mPhysicalDevice->Get() == VK_NULL_HANDLE) {
        throw std::runtime_error("mPhysicalDevice is null");
    }

    std::vector<VkFormat>& candidates = GetConfig().presentFb.depthFormatCandidates;
    VkImageTiling tiling = GetConfig().presentFb.tiling;
    VkFormatFeatureFlags features = GetConfig().presentFb.features;

    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(mPhysicalDevice->Get(), format, &props);
        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }
    throw std::runtime_error("failed to find supported format!");
}
}   // namespace framework

