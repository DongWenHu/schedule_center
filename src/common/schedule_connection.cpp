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

namespace mpsp{

const char* const kerror_ret_fmt = "{\"task_id\":%1%,\"task_cmd\":%2%,\"result_code\":%3%, \"msg\":\"%4%\"}";
const char* const kerror_ret_fmt2 = "{\"result_code\":%1%, \"msg\":\"%2%\"}";

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
    boost::thread trd(boost::bind(&schedule_connection::run_work_queue, shared_from_this()));
}

void schedule_connection::stop()
{
    if (socket_.is_open())
    {
        socket_.close();
    }
}

void schedule_connection::run_work_queue()
{
    std::string task;
    std::string result;
    while (!(task = sche_work_queue_.pop_task()).empty())
    {
        do_task(task, result);
        if (socket_.is_open())
        {
            int len = result.length();
            char* p_send = new char[len + 4];
            memcpy(p_send, &len, 4);
            memcpy(p_send + 4, result.data(), len);
            socket_.write_some(boost::asio::buffer(p_send, len + 4));
            delete p_send;
        }
    }
}

void schedule_connection::do_task(const std::string& task, std::string& result)
{
    boost::property_tree::ptree pt;

    try
    {
        std::stringstream ss;
        ss << task.c_str();
        boost::property_tree::read_json<boost::property_tree::ptree>(ss, pt);

        boost::shared_ptr<task_interface> task_i;

        if (pt.find("task_cmd") == pt.not_found() ||
            pt.find("task_id") == pt.not_found())
        {
            boost::format fmter(kerror_ret_fmt2);
            fmter % -1 % "can't find task_cmd or task_id";
            result = fmter.str();
            return;
        }

        int cmd = pt.get<int>("task_cmd");
        switch (cmd)
        {
        case CMD_SCHE_WX_VOTE:
            task_i = boost::make_shared<wx_vote>();
            break;
        default:
            boost::format fmter(kerror_ret_fmt);
            fmter % pt.get<int>("task_id")
                % pt.get<int>("task_cmd")
                % -1
                % "can't find task cmd";
            result = fmter.str();
            return;
        }

        task_i->do_task(pt, result);
    }
    catch (boost::property_tree::json_parser_error&)
    {
        boost::format fmter(kerror_ret_fmt2);
        fmter % -1 % "format error";
        result = fmter.str();
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
        sche_work_queue_.push_task(mcsr.json_data);
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

} // namespace mpsp