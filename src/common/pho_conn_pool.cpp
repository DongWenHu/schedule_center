#include "pho_conn_pool.hpp"
#include "device_mgr.hpp"

namespace mpsp{

pho_conn_pool pho_conn_pool::pho_conn_pool_;

pho_conn_pool::pho_conn_pool()
{
}

void pho_conn_pool::start(boost::shared_ptr<phone_connection> conn)
{
    boost::shared_ptr<phone_connection> pconn = find_connection(conn->socket().remote_endpoint().address().to_string());
    if (pconn != NULL)
    {
        stop(pconn);
    }
    device_mgr::get().set_status(conn->socket().remote_endpoint().address().to_string(), DEVICE_STATUS_ONLINE);
    conn->start();
    connections_.insert(conn);
}

void pho_conn_pool::stop(boost::shared_ptr<phone_connection> conn)
{
    device_mgr::get().set_status(conn->socket().remote_endpoint().address().to_string(), DEVICE_STATUS_OFFLINE);
    connections_.erase(conn);
    conn->stop();
}

void pho_conn_pool::stop_all()
{
    for (auto c : connections_)
    {
        device_mgr::get().set_status(c->socket().remote_endpoint().address().to_string(), DEVICE_STATUS_OFFLINE);
        c->stop();
    }

    connections_.clear();
}

boost::shared_ptr<phone_connection> pho_conn_pool::find_connection(const std::string& ip)
{
    for (auto c: connections_)
    {
        if ((*c).socket().remote_endpoint().address().to_string() == ip)
        {
            return c;
        }
    }

    return NULL;
}

} // namespace mpsp