#include "mod-ollama-chat_threadpool.h"
#include "mod-ollama-chat_config.h"
#include "Log.h"
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

namespace
{
    class OllamaChatThreadPool
    {
    public:
        OllamaChatThreadPool()
        {
            uint32_t size = g_OllamaThreadPoolSize > 0 ? g_OllamaThreadPoolSize : 8;
            for (uint32_t i = 0; i < size; ++i)
            {
                // Detached, like every other thread this module already spawns: workers run for
                // the life of the process and are never joined.
                std::thread(&OllamaChatThreadPool::WorkerLoop, this).detach();
            }
        }

        void Enqueue(std::function<void()> task)
        {
            {
                std::lock_guard<std::mutex> lock(mutex_);
                tasks_.push(std::move(task));
            }
            cv_.notify_one();
        }

    private:
        void WorkerLoop()
        {
            for (;;)
            {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(mutex_);
                    cv_.wait(lock, [this] { return !tasks_.empty(); });
                    task = std::move(tasks_.front());
                    tasks_.pop();
                }

                try
                {
                    task();
                }
                catch (const std::exception& ex)
                {
                    LOG_ERROR("playerbots", "[OllamaChat] Exception in thread pool task: {}", ex.what());
                }
                catch (...)
                {
                    LOG_ERROR("playerbots", "[OllamaChat] Unknown exception in thread pool task");
                }
            }
        }

        std::queue<std::function<void()>> tasks_;
        std::mutex mutex_;
        std::condition_variable cv_;
    };
}

void EnqueueOllamaChatTask(std::function<void()> task)
{
    static OllamaChatThreadPool pool;
    pool.Enqueue(std::move(task));
}
