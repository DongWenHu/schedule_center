#include "sche_conn_pool.hpp"
#include "typedefs.h"

namespace mpsp{

sche_conn_pool sche_conn_pool::sche_conn_pool_;

sche_conn_pool::sche_conn_pool()
{
}

void sche_conn_pool::start(boost::shared_ptr<schedule_connection> conn)
{
    if (connections_.size() > 0)
    {
        _DEBUG_PRINTF("%s (schedule) already connected me, I don't accept!\n", conn->socket().remote_endpoint().address().to_string().c_str());
        stop(conn);
        return;
    }

    conn->start();
    connections_.insert(conn);
}

void sche_conn_pool::stop(boost::shared_ptr<schedule_connection> conn)
{
    _DEBUG_PRINTF("%s (schedule) disconnected me!\n", conn->socket().remote_endpoint().address().to_string().c_str());
    connections_.erase(conn);
    conn->stop();
}

void sche_conn_pool::stop_all()
{
    std::for_each(connections_.begin(), connections_.end(),
        boost::bind(&schedule_connection::stop, _1));

    _DEBUG_PRINTF("disconnected all (schedule)!\n");
    connections_.clear();
}

} // namespace mpsp