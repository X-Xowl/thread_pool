#include <print>
#include <ranges>
#include "ThreadPool.h"


auto main() -> int
{
    thread_pool pool(4);
    auto fut1 = pool.submit([]()
    {
        for(auto i : std::views::iota(0, 5))
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::print("Task {} completed\n",i);
            std::print("Thread index: {}\n",current_thread::get_index().value_or(-1));
        }
    });
    auto fut2 = pool.submit([]()
    {
        for(auto i : std::views::iota(5, 10))
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::print("Task {} completed\n",i);
            std::print("Thread index: {}\n",current_thread::get_index().value_or(-1));
        }
    });
}