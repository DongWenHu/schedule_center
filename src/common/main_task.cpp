#include "main_task.hpp"
#include "protocol_define.h"
#include "tasks_mgr.hpp"
#include "wx_vote.hpp"
#include "task_processor.hpp"
#include <iostream>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/make_shared.hpp>

namespace mpsp{

main_task::main_task(boost::asio::io_service& ios)
    : io_service_(ios)
    , signals_(io_service_)
    , phone_acceptor_(io_service_)
    , schedule_acceptor_(io_service_)
{
    signals_.add(SIGINT);
    signals_.add(SIGTERM);
    signals_.add(SIGSEGV);
    signals_.add(SIGABRT);
#if defined(SIGHUP)
    signals_.add(SIGHUP);
#endif  //  defined(SIGHUP)
#if defined(SIGBREAK)
    signals_.add(SIGBREAK);
#endif  //  defined(SIGBREAK)
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
    boost::asio::deadline_timer::duration_type td(0, 0, 20, 0);
    task_processor::get().run_after(td, boost::bind(&main_task::start_others, this));
}

void main_task::start_others()
{
    keep_wx_account_task_.start();
    boost::thread trd(boost::bind(&main_task::start_work_queue, this));
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
    io_service_.stop();
}

void main_task::start_work_queue()
{
    std::string result;
    while (true)
    {
        TASKS_INFO ti;
        ti = tasks_mgr::get().work_queue_.pop_task();
        do_work_queue(ti.description, result);
        tasks_mgr::get().set_result(ti.id, result);
        tasks_mgr::get().set_status(ti.id, TASK_STATUS_FINISHED);
    }
}

void main_task::do_work_queue(const std::string& task, std::string& result)
{
    boost::property_tree::ptree pt;

    std::stringstream ss;
    ss << task.c_str();
    boost::property_tree::read_json<boost::property_tree::ptree>(ss, pt);

    boost::shared_ptr<task_interface> task_i;

    int cmd = pt.get<int>("task_cmd");
    switch (cmd)
    {
    case CMD_SCHE_WX_VOTE:
        task_i = boost::make_shared<wx_vote>();
        break;

    default:
        return;
    }

    task_i->do_task(pt, result);

}

} // namespace mpsp