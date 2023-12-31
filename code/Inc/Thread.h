#ifndef __THREAD_H__
#define __THREAD_H__

#include <thread>
#include <condition_variable>
#include <iostream>

namespace common {
    class Thread {
    public:
        Thread();
        virtual ~Thread();

        void Start();
        void Stop();
        void Destroy();

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
    };
}


#endif // !__THREAD_H__

