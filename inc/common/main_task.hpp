#ifndef __SERVER_HPP_2016_04_25__
#define __SERVER_HPP_2016_04_25__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "phone_connection.hpp"
#include "pho_conn_pool.hpp"
#include "schedule_connection.hpp"
#include "sche_conn_pool.hpp"
#include "keep_wx_account_task.hpp"
#include <boost/noncopyable.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/signals2.hpp>


namespace mpsp{

class main_task : private boost::noncopyable
{
private:
    boost::asio::io_service& io_service_;
    boost::asio::signal_set signals_;

    boost::asio::ip::tcp::acceptor phone_acceptor_;
    boost::asio::ip::tcp::acceptor schedule_acceptor_;

    boost::shared_ptr<phone_connection> new_phone_connection_;
    boost::shared_ptr<schedule_connection> new_sche_connection_;

    keep_wx_account_task keep_wx_account_task_;

public:
    main_task(boost::asio::io_service& ios);
    void start();

protected:
    void start_others();
    void listen_(boost::asio::ip::tcp::acceptor& acceptor_, const std::string& port_num);
    void start_phone_accept();
    void start_schedule_accept();
    void handle_phone_accept(const boost::system::error_code& e);
    void handle_schedule_accept(const boost::system::error_code& e);
    void handle_stop();
    void start_work_queue();
    void do_work_queue(const std::string& task, std::string& result);

};

} // namespace mpsp

#endif // __SERVER_HPP_2016_04_25__