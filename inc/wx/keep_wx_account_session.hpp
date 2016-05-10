#ifndef __KEEP_WX_ACCOUNT_SESSION_HPP_2016_05_06__
#define __KEEP_WX_ACCOUNT_SESSION_HPP_2016_05_06__

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
#include "task_processor.hpp"
#include "wx_account_mgr.hpp"
#include "protocol_define.h"
#include "typedefs.h"
#include "pho_conn_pool.hpp"

namespace mpsp{

class keep_wx_account_session
{
    boost::array<char, 8192> buffer_;
    boost::asio::io_service ios_;
    boost::asio::ip::tcp::socket socket_;
    std::string ip_;
    unsigned short port_num_;

    boost::asio::deadline_timer restart_lua_timer_;
    boost::asio::deadline_timer read_timer_;
    boost::shared_ptr<phone_connection> pconn_;

    boost::signals2::connection restart_lua_conn_;

_SLOTS:
    void on_restart_lua_end(int cmd);

public:
    keep_wx_account_session(std::string ip,
        unsigned short port_num);

    void start();

    void on_restart_lua_timeout();

    void on_read_timeout();

    void handle_read(const boost::system::error_code& e,
        std::size_t bytes_transferred);

};

}


#endif  //  __KEEP_WX_ACCOUNT_SESSION_HPP_2016_05_06__