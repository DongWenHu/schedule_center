#ifndef __wx_vote_session_HPP_2016_04_25__
#define __wx_vote_session_HPP_2016_04_25__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "a_session.hpp"
#include "wx_vote.hpp"
#include "wx_account_mgr.hpp"

namespace mpsp{

class wx_vote_session : public a_session
{
    boost::property_tree::ptree& pt_;
    wx_vote* wv_;

_SLOTS:
    void on_restart_lua_end(int cmd);

public:
    wx_vote_session(std::string ip,
        unsigned short port_num,
        boost::property_tree::ptree& pt,
        wx_vote* wv);

    void bind_restart_lua_sig();

    void on_connected();

    void on_read_timeout(const boost::system::error_code& e);

    void handle_read(const boost::system::error_code& e,
        std::size_t bytes_transferred,
        const std::string& acc,
        const std::string& url);
};

}
#endif // __wx_vote_session_HPP_2016_04_25__