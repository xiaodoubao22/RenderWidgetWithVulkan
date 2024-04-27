#include "WindowTemplate.h"
#include <stdexcept>
#include <iostream>
#include "Utils.h"
#include "SceneDemoDefs.h"

namespace window {
WindowTemplate::WindowTemplate(bool resizable) {
    framework::SceneDemoConfig& config = GetConfig();
    // 初始化GLFW窗口
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);	// 不要创建OpenGL上下文
    glfwWindowHint(GLFW_RESIZABLE, resizable);		// 禁止调整窗口大小
    mWindow = glfwCreateWindow(config.window.width, config.window.height, "render widget", nullptr, nullptr);

    glfwSetWindowUserPointer(mWindow, this);
    glfwSetFramebufferSizeCallback(mWindow, FramebufferSizeCallback);
    glfwSetMouseButtonCallback(mWindow, MouseButtonCallback);
    glfwSetCursorPosCallback(mWindow, CursorPosCallback);
    glfwSetKeyCallback(mWindow, KeyCallback);

    glfwSetWindowSizeLimits(mWindow, config.window.minWidth, config.window.minHeight, GLFW_DONT_CARE, GLFW_DONT_CARE);
}

WindowTemplate::~WindowTemplate() {
    // 销毁glfw窗口
    glfwDestroyWindow(mWindow);
    glfwTerminate();
}

void WindowTemplate::Exec() {
    Initialize();
    while (!glfwWindowShouldClose(mWindow)) {
        glfwPollEvents();
        Update();
    }
    CleanUp();
}

VkSurfaceKHR WindowTemplate::CreateSurface(VkInstance instance) {
    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(instance, mWindow, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
    return surface;
}

std::vector<const char*> WindowTemplate::QueryWindowRequiredExtensions() {
    // 获取glfw的拓展
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    return extensions;
}

VkExtent2D WindowTemplate::GetWindowExtent() {
    int width, height;
    glfwGetFramebufferSize(mWindow, &width, &height);

    VkExtent2D windowExtent = {
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height)
    };

    return windowExtent;
}

}