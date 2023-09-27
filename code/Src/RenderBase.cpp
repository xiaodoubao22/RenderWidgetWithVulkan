#include "RenderBase.h"

#include <iostream>
#include <array>

#include "Utils.h"
#include "DebugUtils.h"
#include "WindowTemplate.h"

namespace render {
    RenderBase::RenderBase(window::WindowTemplate& w) : mWindow(w)
    {
        mGraphicsDevice = new GraphicsDevice();
        mSwapchain = new Swapchain();
    }

    RenderBase::~RenderBase() {
        delete mGraphicsDevice;
        delete mSwapchain;
    }

    void RenderBase::Init() {
        CheckValidationLayerSupport(setting::enableValidationLayer);

        // create basic objects
        CreateInstance();
        DebugUtils::GetInstance().Setup(mInstance);
        mSurface = mWindow.CreateSurface(mInstance);
        mGraphicsDevice->Init(mInstance, mSurface);
        mSwapchain->Init(mGraphicsDevice, mWindow.GetWindowExtent(), mSurface);
    }

    void RenderBase::CleanUp() {
        // destroy basic objects
        mSwapchain->CleanUp();
        mGraphicsDevice->CleanUp();
        vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
        DebugUtils::GetInstance().Destroy(mInstance);
        vkDestroyInstance(mInstance, nullptr);
    }

    VkDevice RenderBase::GetDevice() {
        if (mGraphicsDevice == nullptr) {
            throw std::runtime_error("mGraphicsDevice is null");
        }
        mGraphicsDevice->GetDevice();
    }

    void RenderBase::CreateInstance() {
        // 检查需要的拓展
        auto extensions = mWindow.QueryWindowRequiredExtensions();
        if (mEnableValidationLayer) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }
        utils::PrintStringList(extensions, "extensions:");
        if (!CheckExtensionSupport(extensions)) {
            throw std::runtime_error("extension not all supported!");
        }
        else {
            std::cout << "extensions are all supported" << std::endl;
        }

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

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

        // 检查
        return utils::CheckSupported(target, supportExtensionNames);
    }
}

