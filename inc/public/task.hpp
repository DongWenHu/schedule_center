#ifndef __TASK_HPP_2016_04_25__
#define __TASK_HPP_2016_04_25__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/thread/thread.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/make_shared.hpp>

namespace mpsp{

template<class T>
struct task_wrapped
{
private:
    T task_unwrapped_;

public:
    explicit task_wrapped(const T& task_unwrapped)
        : task_unwrapped_(task_unwrapped)
    {

    }

    void operator()() const
    {
        //try
        //{
        //    boost::this_thread::interruption_point();
        //}
        //catch (const boost::thread_interrupted&)
        //{
        //}

        try
        {
            task_unwrapped_();
        }
        catch (const std::exception& e)
        {
            printf("Exception: %s\n", e.what());
        }
        catch (const boost::thread_interrupted&)
        {
            printf("Thread interrupted\n");
        }
        catch (...)
        {
            printf("Unknown exception\n");
        }
    }
};

typedef boost::asio::deadline_timer::duration_type duration_type;

template<class Functor>
struct timer_task : public task_wrapped<Functor>
{
private:
    typedef task_wrapped<Functor> base_t;
    boost::shared_ptr<boost::asio::deadline_timer> timer_;

public:
    template<class Timer>
    explicit timer_task(
        boost::asio::io_service& ios,
        const Timer& duration_or_time,
        const Functor& task_unwrapped)
        : base_t(task_unwrapped)
        , timer_(boost::make_shared<boost::asio::deadline_timer>(boost::ref(ios), duration_or_time))
    {

    }

    void push_task() const
    {
        timer_->async_wait(*this);
    }

    void operator()(const boost::system::error_code& error) const
    {
        if (!error)
        {
            base_t::operator()();
        }
        else
        {
            std::cerr << error << '\n';
        }
    }
};

template<class T>
inline task_wrapped<T> make_task_wrapped(const T& task_unwrapped)
{
    return task_wrapped<T>(task_unwrapped);
}

template<class Time, class Functor>
inline timer_task<Functor> make_timer_task(
    boost::asio::io_service& ios,
    const Time& duration_or_time,
    const Functor& task_unwrapped)
{
    return timer_task<Functor>(ios, duration_or_time, task_unwrapped);
}

}    //    namespace mpsp

#endif