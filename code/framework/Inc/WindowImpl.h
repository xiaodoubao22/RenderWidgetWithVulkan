#ifndef __WINDOW_IMPL_H__
#define __WINDOW_IMPL_H__

#include "WindowTemplate.h"
#include "RenderThread.h"

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
    virtual void OnFramebufferResized(int width, int height) override;
    virtual void OnMouseButton(int button, int action, int mods) override;
    virtual void OnCursorPosChanged(double xpos, double ypos) override;
    virtual void OnKeyEvent(int key, int scancode, int action, int mods) override;

private:
    render::RenderThread* mRenderThread = nullptr;
    //render::Thread* mRenderThread = nullptr;

    bool mIsMinimized = false;
};
}



#endif // !__WINDOW_IMPL_H__
