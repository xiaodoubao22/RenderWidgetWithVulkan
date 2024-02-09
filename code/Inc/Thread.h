#ifndef __THREAD_H__
#define __THREAD_H__

#include <thread>
#include <condition_variable>
#include <iostream>
#include <atomic>

namespace common {
    class Thread {
    public:
        Thread();
        virtual ~Thread();

        void Start();
        void Stop();
        void Destroy();

        virtual void PushData(std::string& lable, void* data);
        void SetFbResized();
        void ResetFbResized();
        bool IsFbResized();

    private:
        void ThreadFunction();

    protected:
        virtual void OnThreadInit() = 0;
        virtual void OnThreadLoop() = 0;
        virtual void OnThreadDestroy() = 0;

    private:
        std::thread mThread;
        std::condition_variable mThreadActiveCondition;
        std::mutex mThreadActiveMutex;
        bool mThreadActiveFlag = false;
        bool mIsDestroying = false;

        // common data
        std::atomic<bool> mFramebufferResized = false;
    };
}


#endif // !__THREAD_H__

