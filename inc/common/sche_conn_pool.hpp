#ifndef __SCHE_CONN_POOL_HPP_2016_05_04__
#define __SCHE_CONN_POOL_HPP_2016_05_04__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "schedule_connection.hpp"
#include <set>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>

namespace mpsp{

class sche_conn_pool
{
private:
    std::set<boost::shared_ptr<schedule_connection> > connections_;
    static sche_conn_pool sche_conn_pool_;

protected:
    sche_conn_pool();

public:
    static sche_conn_pool& get(){ return sche_conn_pool_; }

    void start(boost::shared_ptr<schedule_connection> conn);

    void stop(boost::shared_ptr<schedule_connection> conn);

    void stop_all();

};

} // namespace mpsp

#endif // __SCHE_CONN_POOL_HPP_2016_05_04__