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
    boost::asio::io_service& ios_;
    boost::asio::ip::tcp::socket socket_;
    std::array<char, 8192> buffer_;
    work_queue<std::string> sche_work_queue_;

public:
    schedule_connection();

    boost::asio::ip::tcp::socket& socket();

    void start();

    void stop();

    void handle_read(const boost::system::error_code& e,
        std::size_t bytes_transferred);

    void on_read(const std::string& data);
};

} // namespace mpsp

#endif // __SCHEDULE_CONNECTION_HPP_2016_04_25__