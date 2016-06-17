#ifndef __KEEP_WX_ACCOUNT_SESSION_HPP_2016_05_06__
#define __KEEP_WX_ACCOUNT_SESSION_HPP_2016_05_06__

#include "a_session.hpp"
#include "wx_account_mgr.hpp"

namespace mpsp{

class keep_wx_account_session : public a_session
{
    std::map<std::string, int> login_failed_cnt_;

_SLOTS:
    void on_restart_lua_end(int cmd);

public:
    keep_wx_account_session(std::string ip,
        unsigned short port_num);

    void bind_restart_lua_sig();

    void on_connected();

    void on_read_timeout(const boost::system::error_code& e);

    void handle_read(const boost::system::error_code& e,
        std::size_t bytes_transferred,
        const std::string& acc);

};

}


#endif  //  __KEEP_WX_ACCOUNT_SESSION_HPP_2016_05_06__