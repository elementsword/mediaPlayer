#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>

template <typename T>
class ThreadSafeQueue
{
public:
    ThreadSafeQueue() : stopped(false) {}

    // 添加数据，通知等待线程
    void push(const T &item)
    {
        {
            std::lock_guard<std::mutex> lock(mtx);
            queue.push(item);
        }
        cv.notify_one();
    }

    // 支持移动语义，减少拷贝
    void push(T &&item)
    {
        {
            std::lock_guard<std::mutex> lock(mtx);
            queue.push(std::move(item));
        }
        cv.notify_one();
    }

    // 弹出数据，如果队列为空则阻塞等待，返回 optional<T>
    // 返回空optional表示队列停止且无数据
    std::optional<T> pop()
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this]()
                { return !queue.empty() || stopped; });

        if (queue.empty())
            return std::nullopt; // 停止且无数据

        T item = std::move(queue.front());
        queue.pop();
        return item;
    }

    // 非阻塞尝试弹出，有数据返回true，否则false
    bool tryPop(T &out)
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (queue.empty())
            return false;
        out = std::move(queue.front());
        queue.pop();
        return true;
    }

    // 返回队列大小
    size_t size() const
    {
        std::lock_guard<std::mutex> lock(mtx);
        return queue.size();
    }

    // 停止队列，唤醒所有等待线程，后续pop返回nullopt
    void stop()
    {
        {
            std::lock_guard<std::mutex> lock(mtx);
            stopped = true;
        }
        cv.notify_all();
    }

    // 是否停止
    bool isStopped() const
    {
        std::lock_guard<std::mutex> lock(mtx);
        return stopped;
    }

private:
    mutable std::mutex mtx;
    std::condition_variable cv;
    std::queue<T> queue;
    bool stopped;
};
