#include "RenderBase.h"

#include <iostream>
#include <array>

#include "Utils.h"
#include "DebugUtils.h"
#include "WindowTemplate.h"

namespace render {
    RenderBase::RenderBase(window::WindowTemplate& w) : mWindow(w)
    {
        //mGraphicsDevice = new GraphicsDevice();
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
        CheckValidationLayerSupport(setting::enableValidationLayer);

        // instance
        CreateInstance();

        // debug utils
        if (setting::enableValidationLayer) {
            DebugUtils::GetInstance().Setup(mInstance);
        }
        
        // surface
        mSurface = mWindow.CreateSurface(mInstance);

        
        // physical device
        std::function<bool(VkPhysicalDevice)> testFunc =
            std::bind(&RenderBase::PhysicalDeviceSelectionCondition, this, std::placeholders::_1);
        mPhysicalDevice->SetAdditionalSuiatbleTestFunction(testFunc);
        mPhysicalDevice->Init(mInstance, mSurface);
        // device
        mDevice->Init(mPhysicalDevice);
        // swapchain
        mSwapchain->Init(mPhysicalDevice, mDevice, mWindow.GetWindowExtent(), mSurface);
    }

    void RenderBase::CleanUp() {
        // destroy basic objects
        mSwapchain->CleanUp();
        mDevice->CleanUp();
        mPhysicalDevice->CleanUp();
        vkDestroySurfaceKHR(mInstance, mSurface, nullptr);

        if (setting::enableValidationLayer) {
            DebugUtils::GetInstance().Destroy(mInstance);
        }

        vkDestroyInstance(mInstance, nullptr);
    }

    bool RenderBase::PhysicalDeviceSelectionCondition(VkPhysicalDevice physicalDevice) {
        std::cout << "No additional conditions on choosing physical device\n";
        return true;
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
        std::cout << "--------- check extensions used by instance ----------\n";
        auto extensions = mWindow.QueryWindowRequiredExtensions();
        if (mEnableValidationLayer) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }
        extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        utils::PrintStringList(extensions, "enable extensions:");
        if (!CheckExtensionSupport(extensions)) {
            throw std::runtime_error("extension not all supported!");
        }
        else {
            std::cout << "-------- instance extensions are all supported --------\n\n";
        }

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = extensions.size();
        createInfo.ppEnabledExtensionNames = extensions.data();
        createInfo.enabledLayerCount = 0;
        createInfo.ppEnabledLayerNames = nullptr;
        createInfo.pNext = nullptr;

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if (mEnableValidationLayer) {
            render::DebugUtils::GetInstance().PopulateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.enabledLayerCount = consts::validationLayers.size();
            createInfo.ppEnabledLayerNames = consts::validationLayers.data();
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
        }

        if (vkCreateInstance(&createInfo, nullptr, &mInstance) != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance!");
        }
    }

    void RenderBase::CheckValidationLayerSupport(bool enableValidationLayer) {
        if (enableValidationLayer == false) {
            std::cout << "validation layer disabled" << std::endl;
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
        if (utils::CheckSupported(consts::validationLayers, availableLayerNames)) {
            std::cout << "validation layers are all supported" << std::endl;
            mEnableValidationLayer = true;
        }
        else {
            std::cerr << "validation layer enabled but not supported" << std::endl;
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
}

