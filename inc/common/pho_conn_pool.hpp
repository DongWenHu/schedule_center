#ifndef __PHO_CONN_POOL_HPP_2016_04_25__
#define __PHO_CONN_POOL_HPP_2016_04_25__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "phone_connection.hpp"
#include <set>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>

namespace mpsp{

class pho_conn_pool
{
private:
    std::set<boost::shared_ptr<phone_connection> > connections_;
    static pho_conn_pool pho_conn_pool_;

protected:
    pho_conn_pool();

public:
    static pho_conn_pool& get(){ return pho_conn_pool_; }

    void start(boost::shared_ptr<phone_connection> conn);

    void stop(boost::shared_ptr<phone_connection> conn);

    void stop_all();

    boost::shared_ptr<phone_connection> find_connection(const std::string& ip);
};

} // namespace mpsp

#endif // __PHO_CONN_POOL_HPP_2016_04_25__