#ifndef __wx_vote_session_HPP_2016_04_25__
#define __wx_vote_session_HPP_2016_04_25__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/lexical_cast.hpp>
#include <boost/system/error_code.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/read.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <sstream>
#include <iostream>
#include "task_processor.hpp"
#include "wx_account_mgr.hpp"
#include "protocol_define.h"
#include "wx_vote.hpp"
#include "typedefs.h"
#include "pho_conn_pool.hpp"

namespace mpsp{

class wx_vote_session
{
    boost::array<char, 8192> buffer_;
    boost::asio::io_service ios_;
    boost::asio::ip::tcp::socket socket_;
    std::string ip_;
    unsigned short port_num_;
    boost::property_tree::ptree& pt_;
    wx_vote* wv_;
    boost::asio::deadline_timer restart_lua_timer_;
    boost::asio::deadline_timer read_timer_;
    boost::shared_ptr<phone_connection> pconn_;
    boost::signals2::connection restart_lua_conn_;

_SLOTS:
    void on_restart_lua_end(int cmd);

public:
    wx_vote_session(std::string ip,
        unsigned short port_num,
        boost::property_tree::ptree& pt,
        wx_vote* wv);

    void start();

    void on_restart_lua_timeout();

    void on_read_timeout();

    void handle_read(const boost::system::error_code& e,
        std::size_t bytes_transferred,
        const std::string& acc,
        const std::string& url);
};

}
#endif // __wx_vote_session_HPP_2016_04_25__