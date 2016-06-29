#ifndef SYNCQUEUE_H
#define SYNCQUEUE_H

#include <thread>
#include <condition_variable>
#include <list>
#include <exception>

template<typename T>
class SyncQueue {
public:
    SyncQueue() : finished(false) {}

    void put(const T& task)
    {
        std::unique_lock<std::mutex> lock(qmtx);
        cond.wait(lock, [this]{ return q.size() < 20; });
        q.push_back(task);
        cond.notify_all();
    }

    void put(T&& task)
    {
        std::unique_lock<std::mutex> lock(qmtx);
        cond.wait(lock, [this]{ return q.size() < 20; });
        q.push_back(std::move(task));
        cond.notify_all();
    }

    void get(T& dest)
    {
        std::unique_lock<std::mutex> lock(qmtx);
        cond.wait(lock, [this]{ return !q.empty() or finished; });
        if (finished) throw NoMoreTasks();
        dest = std::move(q.front());
        q.pop_front();
        cond.notify_all();
    }

    void finish()
    {
        std::unique_lock<std::mutex> lock {qmtx};
        cond.wait(lock, [this]{ return q.empty(); });
        finished = true;
        cond.notify_all();
    }

    class NoMoreTasks : public std::exception
    {
    public:
        virtual const char* what() const noexcept { return "End of Queue"; }
    };

private:
    std::mutex qmtx;
    std::condition_variable cond;
    std::list<T> q;

    bool finished;
};

#endif //SYNCQUEUE_H
