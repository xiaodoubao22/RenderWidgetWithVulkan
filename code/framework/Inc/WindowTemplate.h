#ifndef __WINDOW_TEMPLATE_H__
#define __WINDOW_TEMPLATE_H__

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <vector>
#include <unordered_set>
#include <string>

namespace window {
class WindowTemplate {
public:
    explicit WindowTemplate(bool resizable);
    virtual ~WindowTemplate();

    void Exec();
    VkSurfaceKHR CreateSurface(VkInstance instance);
    std::vector<const char*> QueryWindowRequiredExtensions();
    VkExtent2D GetWindowExtent();

protected:
    virtual void Initialize() = 0;
    virtual void Update() = 0;
    virtual void CleanUp() = 0;
    virtual void OnFramebufferResized(int width, int height) = 0;

private:
    static void FramebufferSizeCallback(GLFWwindow* window, int width, int height) {
        auto windowTemplate = reinterpret_cast<WindowTemplate*>(glfwGetWindowUserPointer(window));
        windowTemplate->OnFramebufferResized(width, height);
    }

protected:
    GLFWwindow* mWindow = nullptr;

};
}

#endif // !__WINDOW_TEMPLATE_H__