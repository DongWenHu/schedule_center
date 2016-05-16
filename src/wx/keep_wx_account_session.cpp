#include "keep_wx_account_session.hpp"

namespace mpsp{

// 登录->添加附近的人->发朋友圈->退出
const char* const kkeep_wx_acc_cmd_format = "["
    "[\"%1%\",\"%2%\",\"%3%\",\"%4%\"],"
    "[\"%5%\"],"
    "[\"%6%\"],"
    "[\"%7%\"]]";

keep_wx_account_session::keep_wx_account_session(std::string ip,
    unsigned short port_num)
    : a_session(ip, port_num)
{

}

void keep_wx_account_session::handle_read(const boost::system::error_code& e,
    std::size_t bytes_transferred,
    const std::string& acc)
{
    read_timer_.cancel();
    if (!e)//接收时没有发生错误
    {
        char len[5] = { 0 };
        memcpy(&len, buffer_.data(), 4);
        int ilen = boost::lexical_cast<int>(len);
        char* json_data = new char[ilen + 1];
        memcpy(json_data, buffer_.data() + 4, ilen);
        json_data[ilen] = 0;

        static std::stringstream ss;
        ss.clear();
        ss << json_data;

        boost::property_tree::ptree pt;
        boost::property_tree::read_json<boost::property_tree::ptree>(ss, pt);
        _DEBUG_PRINTF("%s (phone dailytask) says: %s\n", socket_.remote_endpoint().address().to_string().c_str(), json_data);
        if (0 == pt.get<int>("result"))
        {
            wx_account_mgr::get().set_everyday_task(acc, WX_ACCOUNT_DAILYTASK_FINISHED);
        }
        delete json_data;

    }
    else if (e != boost::asio::error::operation_aborted)
    {

    }
}

void keep_wx_account_session::bind_restart_lua_sig()
{
    restart_lua_conn_ = pconn_->op_ok_sig_.connect(boost::bind(&keep_wx_account_session::on_restart_lua_end, this, _1));
}

void keep_wx_account_session::on_connected()
{
    boost::system::error_code ec;
    std::set<std::string> acs =
        wx_account_mgr::get().get_address_pro_copy().at(ip_);

    for (auto c : acs)
    {
        std::map<std::string, WX_ACCOUNT_PRO> acc = wx_account_mgr::get().get_accounts_pro_copy();
        if (acc[c].everyday_task_completed == WX_ACCOUNT_DAILYTASK_FINISHED)
        {
            continue;
        }

        boost::format fmterA(kkeep_wx_acc_cmd_format);
        fmterA%CMD_WX_LOGIN
            %c
            %acc[c].password
            %acc[c].imei
            %CMD_WX_ADD_NEAR_FRIEND
            %CMD_WX_RANDOM_SEND_SNS
            %CMD_WX_LOGOUT;
        boost::format fmterB("%|1$04d|%2%");
        fmterB % fmterA.str().length() % fmterA.str();

        socket_.write_some(boost::asio::buffer(fmterB.str()), ec);
        if (ec)
        {
        }
        else
        {
            //设置异步超时
            read_timer_.expires_from_now(boost::posix_time::minutes(4));
            read_timer_.async_wait(boost::bind(&keep_wx_account_session::on_read_timeout,
                this,
                boost::asio::placeholders::error));
            socket_.async_read_some(boost::asio::buffer(buffer_),
                boost::bind(&keep_wx_account_session::handle_read,
                this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred,
                c));
            ios_.run();
            ios_.reset();
        }

    }
}

void keep_wx_account_session::on_restart_lua_end(int cmd)
{
    if (cmd != CMD_APP_RESTART_LUA)
    {
        return;
    }

    restart_lua_timer_.cancel();
    device_mgr::get().set_status(ip_, DEVICE_STATUS_IDLE);
    start();
}

void keep_wx_account_session::on_read_timeout(const boost::system::error_code& e)
{
    if (e)
    {
        // timer is canceled 
    }
    else
    {
        if (read_timer_.expires_at() > boost::asio::deadline_timer::traits_type::now())
        {
            // 闹钟都没到就叫我，还让不让睡了zZ
            return;
        }

        if (socket_.is_open())
            socket_.close();

        pconn_->restart_lua();
        restart_lua_timer_.expires_from_now(boost::posix_time::minutes(1));
        restart_lua_timer_.async_wait(boost::bind(&keep_wx_account_session::on_restart_lua_timeout,
            this,
            boost::asio::placeholders::error));
    }
}

}