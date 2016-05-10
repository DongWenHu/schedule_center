#include "sche_conn_pool.hpp"

namespace mpsp{

sche_conn_pool sche_conn_pool::sche_conn_pool_;

sche_conn_pool::sche_conn_pool()
{
}

void sche_conn_pool::start(boost::shared_ptr<schedule_connection> conn)
{
    if (connections_.size() > 0)
    {
        conn->stop();
        return;
    }

    conn->start();
    connections_.insert(conn);
}

void sche_conn_pool::stop(boost::shared_ptr<schedule_connection> conn)
{
    connections_.erase(conn);
    conn->stop();
}

void sche_conn_pool::stop_all()
{
    std::for_each(connections_.begin(), connections_.end(),
        boost::bind(&schedule_connection::stop, _1));

    connections_.clear();
}

} // namespace mpsp