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
        _DEBUG_PRINTF("%s (phone) connected me again, I close last one!\n", conn->socket().remote_endpoint().address().to_string().c_str());
        stop(pconn);
    }
    device_mgr::get().set_status(conn->socket().remote_endpoint().address().to_string(), DEVICE_STATUS_ONLINE_IDLE);
    conn->start();
    connections_.insert(conn);
}

void pho_conn_pool::stop(boost::shared_ptr<phone_connection> conn)
{
    _DEBUG_PRINTF("%s (phone) disconnected me!\n", conn->socket().remote_endpoint().address().to_string().c_str());
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
    _DEBUG_PRINTF("disconnected all (phone)!\n");
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