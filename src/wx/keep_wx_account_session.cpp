#include "keep_wx_account_session.hpp"
#include "device_mgr.hpp"
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

namespace mpsp{

// 登录->发朋友圈->添加附近的人
const char* const kkeep_wx_acc_cmd_format = "[[\"%1%\",\"%2%\",\"%3%\",\"%4%\"],[\"%5%\"],[\"%6%\"]]";

keep_wx_account_session::keep_wx_account_session(std::string ip,
    unsigned short port_num)
    : socket_(ios_)
    , ip_(ip)
    , port_num_(port_num)
    , restart_lua_timer_(ios_, boost::posix_time::seconds(60))
    , read_timer_(ios_, boost::posix_time::minutes(4))
{

}

void keep_wx_account_session::start()
{
    pconn_ = pho_conn_pool::get().find_connection(ip_);
    if (pconn_ == NULL)
    {
        return;
    }

    //设置当前设备繁忙
    device_mgr::get().set_status(ip_, DEVICE_STATUS_BUSY);

    // 绑定手机APP连接重启lua脚本结束信号
    if (!restart_lua_conn_.connected())
        restart_lua_conn_ = pconn_->op_ok_sig_.connect(boost::bind(&keep_wx_account_session::on_restart_lua_end, this, _1));

    boost::system::error_code ec;
    socket_.connect(boost::asio::ip::tcp::endpoint(
        boost::asio::ip::address_v4::from_string(ip_), port_num_),
        ec);
    if (ec)
    {
        // 连接失败处理
        pconn_->restart_lua();
        restart_lua_timer_.async_wait(boost::bind(&keep_wx_account_session::on_restart_lua_timeout, this));
        ios_.run();
    }
    else
    {
        for (auto c : wx_account_mgr::get().get_address_pro_copy()[ip_])
        {
            boost::format fmterA(kkeep_wx_acc_cmd_format);
            fmterA%CMD_WX_LOGIN
                %c
                %wx_account_mgr::get().get_accounts_pro_copy()[c].password
                %wx_account_mgr::get().get_accounts_pro_copy()[c].imei
                %CMD_WX_RANDOM_SEND_SNS
                %CMD_WX_ADD_NEAR_FRIEND;
            boost::format fmterB("%|1$04d|");
            fmterB % fmterA.str().length();

            MSG_CMD_LUA mcl;
            mcl.len = fmterB.str();
            mcl.json_data = fmterA.str();

            std::ostringstream  oarchiveStream;
            boost::archive::binary_oarchive  oarchive(oarchiveStream);
            oarchive << mcl;

            socket_.write_some(boost::asio::buffer(oarchiveStream.str()), ec);
            if (ec)
            {
                pconn_->restart_lua();
                restart_lua_timer_.async_wait(boost::bind(&keep_wx_account_session::on_restart_lua_timeout, this));
                ios_.run();
            }
            else
            {
                //设置异步超时
                read_timer_.async_wait(boost::bind(&keep_wx_account_session::on_read_timeout, this));
                socket_.async_read_some(boost::asio::buffer(buffer_),
                    boost::bind(&keep_wx_account_session::handle_read,
                    this,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
                ios_.run();
            }

        }
        socket_.close();
    }

    //设置当前设备空闲
    device_mgr::get().set_status(ip_, DEVICE_STATUS_IDLE);
}

void keep_wx_account_session::handle_read(const boost::system::error_code& e,
    std::size_t bytes_transferred)
{
    if (!e)//接收时没有发生错误
    {
        MSG_CMD_LUA mcl;
        std::istringstream iarchiveStream(buffer_.data());
        boost::archive::binary_iarchive iarchive(iarchiveStream);
        iarchive >> mcl;

        static std::stringstream ss;
        ss.clear();
        ss << mcl.json_data;

        boost::property_tree::ptree pt;
        boost::property_tree::read_json<boost::property_tree::ptree>(ss, pt);

        if (0 == pt.get<int>("result"))
        {
            wx_account_mgr::get().set_everyday_task(ip_, true);
        }

    }
    else if (e != boost::asio::error::operation_aborted)
    {
        pconn_->restart_lua();
        restart_lua_timer_.async_wait(boost::bind(&keep_wx_account_session::on_restart_lua_timeout, this));
        ios_.run();
    }
}

void keep_wx_account_session::on_restart_lua_end(int cmd)
{
    if (cmd != CMD_APP_RESTART_LUA)
    {
        return;
    }

    restart_lua_timer_.cancel();
    start();
}

void keep_wx_account_session::on_restart_lua_timeout()
{
    if (restart_lua_conn_.connected())
    {
        restart_lua_conn_.disconnect();
    }
}

void keep_wx_account_session::on_read_timeout()
{

}

}