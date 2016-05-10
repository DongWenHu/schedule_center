#include "phone_connection.hpp"
#include "pho_conn_pool.hpp"
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

namespace mpsp{

phone_connection::phone_connection( boost::asio::io_service& io_service)
    : socket_(io_service)
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
        ////在此处处理来的数据
        //MSG_CMD_APP mca;
        //std::istringstream iarchiveStream(buffer_.data());
        //boost::archive::text_iarchive iarchive(iarchiveStream);
        //iarchive >> mca;
        //do_read_msg(mca);

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

void phone_connection::do_read_msg(const MSG_CMD_APP& mca)
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
    MSG_CMD_APP mca;
    mca.cmd = CMD_APP_RESTART_LUA;

    std::ostringstream  oarchiveStream;
    boost::archive::text_oarchive  oarchive(oarchiveStream);
    oarchive << mca;

    boost::system::error_code ec;
    socket_.write_some(boost::asio::buffer(oarchiveStream.str()), ec);
}

} // namespace mpsp