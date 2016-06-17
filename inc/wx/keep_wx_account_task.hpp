#ifndef __KEEP_WX_ACCOUNT_TASK_HPP_2016_05_03__
#define __KEEP_WX_ACCOUNT_TASK_HPP_2016_05_03__

#include <boost/noncopyable.hpp>
#include <boost/asio/io_service.hpp>

namespace mpsp{

class keep_wx_account_task : private boost::noncopyable
{
public:
    keep_wx_account_task();

    void start();

    void reset_task();

    void do_task();
};

}

#endif //   __KEEP_WX_ACCOUNT_TASK_HPP_2016_05_03__