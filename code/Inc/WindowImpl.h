#ifndef __WINDOW_IMPL_H__
#define __WINDOW_IMPL_H__

#include "WindowTemplate.h"
#include "DrawTriangleThread.h"

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

    private:
        render::DrawTriangleThread* mDrawTriangleThread = nullptr;

        bool mIsMinimized = false;
    };
}



#endif // !__WINDOW_IMPL_H__
