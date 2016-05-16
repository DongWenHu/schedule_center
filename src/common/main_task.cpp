#include "main_task.hpp"
#include "protocol_define.h"
#include <iostream>
#include <boost/lexical_cast.hpp>

namespace mpsp{

main_task::main_task(boost::asio::io_service& ios)
    : io_service_(ios)
    , signals_(io_service_)
    , phone_acceptor_(io_service_)
    , schedule_acceptor_(io_service_)
{
    signals_.add(SIGINT);
    signals_.add(SIGTERM);
#if defined(SIGQUIT)
    signals_.add(SIGQUIT);
#endif // defined(SIGQUIT)
    signals_.async_wait(boost::bind(&main_task::handle_stop, this));

    listen_(phone_acceptor_, boost::lexical_cast<std::string>(PHONE_PORT));
    listen_(schedule_acceptor_, boost::lexical_cast<std::string>(SCHEDULE_PORT));
}

void main_task::listen_(boost::asio::ip::tcp::acceptor& acceptor_, const std::string& port_num)
{
    boost::asio::ip::tcp::resolver resolver(io_service_);
    boost::asio::ip::tcp::resolver::query query("0.0.0.0", port_num);
    boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);
    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen();
}

void main_task::start()
{
    start_phone_accept();
    start_schedule_accept();
    keep_wx_account_task_.start();
}

void main_task::start_phone_accept()
{
    new_phone_connection_.reset(new phone_connection());

    phone_acceptor_.async_accept(new_phone_connection_->socket(),
        boost::bind(&main_task::handle_phone_accept, this,
        boost::asio::placeholders::error));
}

void main_task::start_schedule_accept()
{
    new_sche_connection_.reset(new schedule_connection());

    schedule_acceptor_.async_accept(new_sche_connection_->socket(),
        boost::bind(&main_task::handle_schedule_accept, this,
        boost::asio::placeholders::error));
}

void main_task::handle_phone_accept(const boost::system::error_code& e)
{
    if (!phone_acceptor_.is_open())
    {
        return;
    }
    if (!e)
    {
        _DEBUG_PRINTF("%s (phone) connected me!\n", new_phone_connection_->socket().remote_endpoint().address().to_string().c_str());
        pho_conn_pool::get().start(new_phone_connection_);
    }
    start_phone_accept();
}

void main_task::handle_schedule_accept(const boost::system::error_code& e)
{
    if (!schedule_acceptor_.is_open())
    {
        return;
    }
    if (!e)
    {
        _DEBUG_PRINTF("%s (schedule) connected me!\n", new_sche_connection_->socket().remote_endpoint().address().to_string().c_str());
        sche_conn_pool::get().start(new_sche_connection_);
    }
    start_schedule_accept();
}

void main_task::handle_stop()
{
    phone_acceptor_.close();
    schedule_acceptor_.close();
    pho_conn_pool::get().stop_all();
    sche_conn_pool::get().stop_all();
}

} // namespace mpsp