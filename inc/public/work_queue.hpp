#ifndef __WORK_QUEUE_HPP_2016_05_05__
#define __WORK_QUEUE_HPP_2016_05_05__

#include <deque>
#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/condition_variable.hpp>

namespace mpsp{

template<class T>
class work_queue
{
public:
    typedef T task_type;

private:
    std::deque<task_type>   tasks_;
    boost::mutex    tasks_mutex;
    boost::condition_variable   cond_;
    bool   is_stopped_;

public:
    work_queue()
        : is_stopped_(false)
    {
    }

    void stop()
    {
        boost::unique_lock<boost::mutex> lock(tasks_mutex);
        is_stopped_ = true;
        lock.unlock();
        cond_.notify_all();
    }

    void push_task(const T& task)
    {
        boost::unique_lock<boost::mutex> lock(tasks_mutex);
        if (is_stopped_)
        {
            return;
        }
        tasks_.push_back(task);
        lock.unlock();
        cond_.notify_one();
    }

    task_type pop_task()
    {
        boost::unique_lock<boost::mutex> lock(tasks_mutex);
        while (tasks_.empty())
        {
            if (is_stopped_)
            {
                return task_type();
            }

            cond_.wait(lock);
        }

        task_type ret = tasks_.front();
        tasks_.pop_front();
        return ret;
    }

};

}

#endif  //  __WORK_QUEUE_HPP_2016_05_05__