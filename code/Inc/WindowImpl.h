#ifndef __WINDOW_IMPL_H__
#define __WINDOW_IMPL_H__

#include "WindowTemplate.h"
#include "RenderBase.h"

namespace window {
    class WindowImpl : public WindowTemplate
    {
    public:
        WindowImpl(bool resizable);
        ~WindowImpl();

    private:
        virtual void Initialize() override;
        virtual void Update() override;
        virtual void CleanUp() override;
        virtual void OnFramebufferResize(GLFWwindow* window, int width, int height) override;

    private:
        render::RenderBase* mRenderer = nullptr;
    };
}



#endif // !__WINDOW_IMPL_H__
