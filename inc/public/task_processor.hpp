#ifndef __TASK_PROCESSOR_HPP_2016_04_25__
#define __TASK_PROCESSOR_HPP_2016_04_25__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/io_service.hpp>
#include "task.hpp"

namespace mpsp{

class task_processor : private boost::noncopyable
{
    typedef boost::asio::deadline_timer::time_type time_type;
    typedef boost::asio::deadline_timer::duration_type duration_type;

    boost::asio::io_service ios_;
    boost::asio::io_service::work work_;
    task_processor()
        : ios_()
        , work_(ios_)
    {

    }

public:
    static task_processor& get()
    {
        static task_processor task_processor_;
        return task_processor_;
    }

    boost::asio::io_service& get_io_service()
    {
        return ios_;
    }

    template<class T>
    void push_task(const T& task_unwrapped)
    {
        ios_.post(mpsp::make_task_wrapped(task_unwrapped));
    }

    template<class Functor>
    void run_at(time_type time, const Functor& f)
    {
        mpsp::make_timer_task(ios_, time, f).push_task();
    }

    template<class Functor>
    void run_after(duration_type duration, const Functor& f)
    {
        mpsp::make_timer_task(ios_, duration, f).push_task();
    }

    void start()
    {
        ios_.run();
    }

    void stop()
    {
        ios_.stop();
    }
};

} // namespace mpsp
#endif