#ifndef __SCHEDULE_CONNECTION_HPP_2016_04_25__
#define __SCHEDULE_CONNECTION_HPP_2016_04_25__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/array.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/bind.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/signals2.hpp>
#include "work_queue.hpp"

namespace mpsp{

class schedule_connection
    : public boost::enable_shared_from_this<schedule_connection>
    , private boost::noncopyable
{
 private:
    boost::asio::ip::tcp::socket socket_;
    std::array<char, 8192> buffer_;
    work_queue<std::array<char, 8192> > sche_work_queue_;

public:
    schedule_connection(boost::asio::io_service&);

    boost::asio::ip::tcp::socket& socket();

    void start();

    void stop();

    void run_work_queue();

    void do_task(std::stringstream& task, std::string& result);

    void handle_read(const boost::system::error_code& e,
        std::size_t bytes_transferred);
};

} // namespace mpsp

#endif // __SCHEDULE_CONNECTION_HPP_2016_04_25__