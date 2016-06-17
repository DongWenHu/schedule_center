#include "a_session.hpp"

namespace mpsp{

a_session::a_session(std::string ip,
    unsigned short port_num)
    : socket_(ios_)
    , ip_(ip)
    , port_num_(port_num)
    , restart_lua_timer_(ios_, boost::posix_time::minutes(1))
    , read_timer_(ios_, boost::posix_time::minutes(3))
{

}

void a_session::start()
{
    pconn_ = pho_conn_pool::get().find_connection(ip_);
    if (pconn_ == NULL)
    {
        return;
    }

    //设置当前设备繁忙
    device_mgr::get().set_status(ip_, DEVICE_STATUS_TASK_BEGIN);

    // 绑定手机APP连接重启lua脚本结束信号
    if (!restart_lua_conn_.connected())
        bind_restart_lua_sig();

    boost::system::error_code ec;
    socket_.connect(boost::asio::ip::tcp::endpoint(
        boost::asio::ip::address_v4::from_string(ip_), port_num_),
        ec);
    if (ec)
    {
        // 连接失败处理
        pconn_->restart_lua();
        restart_lua_timer_.async_wait(boost::bind(&a_session::on_restart_lua_timeout,
            this,
            boost::asio::placeholders::error));
        ios_.run();
        ios_.reset();
    }
    else
    {
        on_connected();
        if (socket_.is_open())
            socket_.close();
    }

    //设置当前设备空闲
    device_mgr::get().set_status(ip_, DEVICE_STATUS_ONLINE_IDLE);
}

void a_session::on_restart_lua_timeout(const boost::system::error_code& e)
{
    if (e)
    {
        // timer is canceled 
    }
    else
    {
        if (restart_lua_timer_.expires_at() > boost::asio::deadline_timer::traits_type::now())
        {
            // 闹钟都没到就叫我，还让不让睡了zZ
            return;
        }

        if (restart_lua_conn_.connected())
        {
            restart_lua_conn_.disconnect();
        }
    }
}

}