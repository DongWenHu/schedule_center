#include "phone_connection.hpp"
#include "pho_conn_pool.hpp"
#include "typedefs.h"
#include "task_processor.hpp"
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

namespace mpsp{

phone_connection::phone_connection()
    : ios_(mpsp::task_processor::get().get_io_service())
    , socket_(ios_)
{
}

boost::asio::ip::tcp::socket& phone_connection::socket()
{
    return socket_;
}

void phone_connection::start()
{
    socket_.async_read_some(boost::asio::buffer(buffer_),
        boost::bind(&phone_connection::handle_read,
        shared_from_this(),
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));
}

void phone_connection::stop()
{
    if (socket_.is_open())
    {
        socket_.close();
    }
}

void phone_connection::handle_read(const boost::system::error_code& e,
    std::size_t bytes_transferred)
{
    if (!e)//接收时没有发生错误
    {
        //在此处处理来的数据
        MSG_CMD_APP_RECV mca;
        memcpy(&mca.cmd, buffer_.data(), sizeof(mca));
        do_read_msg(mca);
        socket_.async_read_some(boost::asio::buffer(buffer_),
            boost::bind(&phone_connection::handle_read,
            shared_from_this(),
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    }
    else if (e != boost::asio::error::operation_aborted)
    {
        //printf("%s offline\n", socket_.remote_endpoint().address().to_string().c_str());
        if (socket_.is_open())
        {
            pho_conn_pool::get().stop(shared_from_this());
        }
    }
}

void phone_connection::do_read_msg(const MSG_CMD_APP_RECV& mca)
{
    switch (mca.cmd)
    {
    case CMD_APP_HEARTBEAT:
        break;

    case CMD_APP_START_LUA:
    case CMD_APP_STOP_LUA:
    case CMD_APP_RESTART_LUA:
    case CMD_APP_REBOOT:
    {
        if (mca.result == 0)
        {
            op_ok_sig_(mca.cmd);
        }
        break;
    }

    default:
        break;
    }
}

void phone_connection::handle_write(const boost::system::error_code& e)
{
    if (!e)//如果发送响应数据成功
    {
    }
    if (e != boost::asio::error::operation_aborted)
    {
        pho_conn_pool::get().stop(shared_from_this());
    }
}

void phone_connection::restart_lua()
{
    MSG_CMD_APP_SEND mca;
    mca.cmd = CMD_APP_RESTART_LUA;

    boost::system::error_code ec;
    if (socket_.is_open())
    {
        socket_.write_some(boost::asio::buffer(&mca, sizeof(mca)), ec);
    }
}

} // namespace mpsp