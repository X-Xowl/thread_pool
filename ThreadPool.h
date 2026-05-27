//
// Created by X_XOWL on 2026/5/26.
//

#ifndef XOWL_THREADPOOL_H
#define XOWL_THREADPOOL_H
#include <condition_variable>
#include <thread>
#include <vector>
#include <queue>
#include <functional>
#include <future>
#include <optional>
#include <mutex>
#include <concepts>
#include <type_traits>
/*********************************************/
class [[nodiscard]] current_thread
{
friend class thread_pool;
public:
    [[nodiscard]] static std::optional<std::size_t> get_index() noexcept
    {
        return index;
    }
private:
    inline static thread_local std::optional<std::size_t> index = std::nullopt;
};
/********************************************/

class [[nodiscard]] thread_pool
{
public:
    thread_pool() : thread_pool(std::thread::hardware_concurrency()) {}
    explicit thread_pool(const std::size_t num_threads) : thread_count(num_threads),running_tasks(num_threads)
    {
        for (std::size_t i=0;i<num_threads;++i)
        {
            threads.emplace_back([this,i](const std::stop_token& stop_token)
            {
                worker(stop_token,i);
            });
        }
    }
    ~thread_pool() noexcept
    {
        try
        {
            wait();
        }
        catch (...) {}
    }
    thread_pool(const thread_pool&) = delete;
    thread_pool(thread_pool&&) = delete;
    thread_pool& operator=(const thread_pool&) = delete;
    thread_pool& operator=(thread_pool&&) = delete;
    /////////////////////////////////////////////
    template <std::invocable F,typename R = std::invoke_result_t<std::decay_t<F>>>
    [[nodiscard]] auto submit(F&& f)
    {
        auto task = std::packaged_task<R()>(std::forward<F>(f));
        auto fut = task.get_future();
        {
            std::scoped_lock lock(tasks_mutex);
            tasks.push([task = std::move(task)]() mutable
            {
                task();
            });
        }
        task_available_cv.notify_one();
        return fut;
    }
    /////////////////////////////////////////////

    ////////////////////////////////////////////
    void wait()
    {
        std::unique_lock lock(tasks_mutex);
        waiting = true;
        task_finished_cv.wait(lock,[this]
        {
           return tasks.empty() && running_tasks == 0;
        });
        waiting = false;
    }
    ///////////////////////////////////////////
    void worker(const std::stop_token& stop_token,const std::size_t idx)
    {
        current_thread::index = idx;
        while (true)
        {
            std::unique_lock lock(tasks_mutex);
            --running_tasks;
            if (waiting && running_tasks == 0 && tasks.empty())
            {
                task_finished_cv.notify_all();
            }
            task_available_cv.wait(lock,stop_token,[this]
            {
                return !tasks.empty();
            });
            if (stop_token.stop_requested())
            {
                break;
            }
            {
                auto task = std::move(tasks.front());
                ++running_tasks;
                tasks.pop();
                lock.unlock();
                try
                {
                    task();
                }
                catch (...){}
            }
        }
        current_thread::index = std::nullopt;
    }
    ///////////////////////////////////////////
private:
    std::vector<std::jthread> threads;
    std::queue<std::move_only_function<void()>> tasks;
    std::size_t thread_count = 0;
    std::size_t running_tasks = 0;
    std::condition_variable_any task_available_cv;
    std::condition_variable task_finished_cv;
    std::mutex tasks_mutex;
    bool waiting = false;
};
#endif //XOWL_THREADPOOL_H
