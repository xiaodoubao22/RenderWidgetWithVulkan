#include "WindowImpl.h"
#include "Utils.h"
#include "RenderThread.h"

namespace window {
WindowImpl::WindowImpl(bool resizable) : WindowTemplate(resizable)
{
    mRenderThread = new framework::RenderThread(*this);
}

WindowImpl::~WindowImpl()
{
    mRenderThread->Destroy();
    delete mRenderThread;
}

void WindowImpl::Initialize()
{
    mRenderThread->Start();
}

void WindowImpl::Update()
{

}

void WindowImpl::CleanUp()
{
    mRenderThread->Stop();
}

void WindowImpl::OnFramebufferResized(int width, int height)
{
    if (width == 0 || height == 0) {
        if (mIsMinimized == false) {
            mRenderThread->Stop();    // 最小化时停止渲染线程
        }
        mIsMinimized = true;
    }
    else {
        if (mIsMinimized == true) {
            mRenderThread->Start();   // 恢复时打开渲染线程
        }
        mIsMinimized = false;
    }

    mRenderThread->SetFbResized();
}

void WindowImpl::OnMouseButton(int button, int action, int mods)
{
    mRenderThread->SetMouseButton(button, action, mods);
}

void WindowImpl::OnCursorPosChanged(double xpos, double ypos)
{
    mRenderThread->SetCursorPosChanged(xpos, ypos);
}

void WindowImpl::OnKeyEvent(int key, int scancode, int action, int mods)
{
    mRenderThread->SetKeyEvent(key, scancode, action, mods);
}
}   // namespace window
