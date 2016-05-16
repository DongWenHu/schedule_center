#ifndef __PHONE_CONNECTION_HPP_2016_04_25__
#define __PHONE_CONNECTION_HPP_2016_04_25__

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
#include "typedefs.h"
#include "protocol_define.h"

namespace mpsp{

class phone_connection
    : public boost::enable_shared_from_this<phone_connection>
    , private boost::noncopyable
{
private:
    boost::asio::io_service& ios_;
    boost::asio::ip::tcp::socket socket_;
    std::array<char, 8192> buffer_;

_SIGNALS:
    boost::signals2::signal<void(int)> op_ok_sig_;

public:
    phone_connection();

    boost::asio::ip::tcp::socket& socket();

    void start();

    void stop();

    void handle_read(const boost::system::error_code& e,
        std::size_t bytes_transferred);

    void do_read_msg(const MSG_CMD_APP_RECV& mca);

    void handle_write(const boost::system::error_code& e);

    void restart_lua();
};

} // namespace mpsp

#endif // __PHONE_CONNECTION_HPP_2016_04_25__