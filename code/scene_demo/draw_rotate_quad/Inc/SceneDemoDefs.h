#ifndef __SCENE_DEMO_DEFS__
#define __SCENE_DEMO_DEFS__

#include "DrawRotateQuad.h"
#include "SceneDemoConfig.h"

static framework::SceneDemoConfig g_SceneDemoConfig = {};

static void FillConfig()
{
    // window
    g_SceneDemoConfig.window.width = 1280;
    g_SceneDemoConfig.window.height = 960;
    g_SceneDemoConfig.window.minWidth = 200;
    g_SceneDemoConfig.window.minHeight = 200;

    // physical device
    g_SceneDemoConfig.phisicalDevice.defaultDeviceType = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;

    // layers
    g_SceneDemoConfig.layer.instanceLayers = {};
    g_SceneDemoConfig.layer.deviceLayers = {};

    // validation layer
#ifdef NDEBUG
    g_SceneDemoConfig.layer.enableValidationLayer = false;
#else
    g_SceneDemoConfig.layer.enableValidationLayer = true;
#endif

    // extensions
    g_SceneDemoConfig.extension.instanceExtensions = {
        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
    };
    g_SceneDemoConfig.extension.deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME,
    };

    // swapchain
    g_SceneDemoConfig.swapchain.surfaceFormat = {
        VK_FORMAT_B8G8R8A8_SRGB,
        VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT
    };
    g_SceneDemoConfig.swapchain.imageCount = 2;
    g_SceneDemoConfig.swapchain.presentMode = VK_PRESENT_MODE_FIFO_KHR;

    // present fb
    g_SceneDemoConfig.presentFb.depthFormatCandidates = { VK_FORMAT_D24_UNORM_S8_UINT };
    g_SceneDemoConfig.presentFb.tiling = VK_IMAGE_TILING_OPTIMAL;
    g_SceneDemoConfig.presentFb.features = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;

    // dirs
    g_SceneDemoConfig.directory.dirSpvFiles = "../spv_files/";
    g_SceneDemoConfig.directory.dirResource = "../resource/";
}

static framework::SceneDemoConfig& GetConfig()
{
    static bool configInited = false;
    if (!configInited) {
        FillConfig();
        configInited = true;
    }
    return g_SceneDemoConfig;
}

static framework::DrawRotateQuad* CreateSceneRender()
{
    return new framework::DrawRotateQuad;
}

#endif // !__SCENE_DEMO_DEFS__