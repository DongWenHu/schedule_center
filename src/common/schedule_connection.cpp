#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include "schedule_connection.hpp"
#include "task_processor.hpp"
#include "task_interface.h"
#include "protocol_define.h"
#include "wx_vote.hpp"
#include "sche_conn_pool.hpp"
#include "typedefs.h"
#include "tasks_mgr.hpp"

namespace mpsp{

schedule_connection::schedule_connection()
    : ios_(mpsp::task_processor::get().get_io_service())
    , socket_(ios_)
{
}

boost::asio::ip::tcp::socket& schedule_connection::socket()
{
    return socket_;
}

void schedule_connection::start()
{
    socket_.async_read_some(boost::asio::buffer(buffer_),
        boost::bind(&schedule_connection::handle_read,
        shared_from_this(),
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));
}

void schedule_connection::stop()
{
    if (socket_.is_open())
    {
        socket_.close();
    }
}

void schedule_connection::handle_read(const boost::system::error_code& e,
    std::size_t bytes_transferred)
{
    if (!e)//接收时没有发生错误
    {
        MSG_CMD_SCHE_RECV mcsr;
        memcpy(&mcsr.len, buffer_.data(), 4);
        mcsr.json_data = buffer_.data() + 4;
        mcsr.json_data[mcsr.len] = 0;

        _DEBUG_PRINTF("%s (schedule) says: %s\n", socket_.remote_endpoint().address().to_string().c_str(), mcsr.json_data.c_str());
        on_read(mcsr.json_data);
        socket_.async_read_some(boost::asio::buffer(buffer_),
            boost::bind(&schedule_connection::handle_read,
            shared_from_this(),
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    }
    else if (e != boost::asio::error::operation_aborted)
    {
        //printf("%s offline\n", socket_.remote_endpoint().address().to_string().c_str());
        if (socket_.is_open())
        {
            sche_conn_pool::get().stop(shared_from_this());
        }
    }
}

void schedule_connection::on_read(const std::string& data)
{
    boost::property_tree::ptree pt;

    try
    {
        std::stringstream ss;
        ss << data.c_str();
        boost::property_tree::read_json<boost::property_tree::ptree>(ss, pt);

        boost::shared_ptr<task_interface> task_i;

        if (pt.find("task_cmd") == pt.not_found() ||
            pt.find("task_id") == pt.not_found())
        {
            return;
        }

        int cmd = pt.get<int>("task_cmd");
        switch (cmd)
        {
        case CMD_SCHE_WX_VOTE:
        {
            TASKS_INFO ti;
            ti.id = pt.get<int>("task_id");
            ti.description = data;
            ti.status = TASK_STATUS_QUEUING;

            tasks_mgr::get().add_task(ti);
            break;
        }
        case CMD_SCHE_WX_GET_RESULT:
        {
            std::string ret;
            tasks_mgr::get().get_result(pt.get<int>("task_id"), ret);
            int len = ret.length();
            char* p_send = new char[len + 4];
            memcpy(p_send, &len, 4);
            memcpy(p_send + 4, ret.data(), len);
            socket_.write_some(boost::asio::buffer(p_send, len + 4));
            delete p_send;
            break;
        }
        default:
            return;
        }

    }
    catch (boost::property_tree::json_parser_error&)
    {

    }
}


} // namespace mpsp