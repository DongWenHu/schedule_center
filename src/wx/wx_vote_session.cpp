#include "wx_vote_session.hpp"
#include "device_mgr.hpp"
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

namespace mpsp{

const char* const kvote_cmd_format = "[[\"%1%\",\"%2%\",\"%3%\",\"%4%\"],[\"%5%\",\"%6%\",\"%7%\"],[\"%8%\"]]";
const char* const kdownload_to_path = "/mnt/sdcard/TouchSprite/lua/wx/wx_vote.lua";

wx_vote_session::wx_vote_session(std::string ip,
    unsigned short port_num,
    boost::property_tree::ptree& pt,
    wx_vote* wv)
    : socket_(ios_)
    , ip_(ip)
    , port_num_(port_num)
    , pt_(pt)
    , wv_(wv)
    , restart_lua_timer_(ios_, boost::posix_time::seconds(60))
    , read_timer_(ios_, boost::posix_time::minutes(4))
{

}

void wx_vote_session::start()
{
    //pconn_ = pho_conn_pool::get().find_connection(ip_);
    //if (pconn_ == NULL)
    //{
    //    return;
    //}

    //设置当前设备繁忙
    device_mgr::get().set_status(ip_, DEVICE_STATUS_BUSY);

    // 绑定手机APP连接重启lua脚本结束信号
    //if (!restart_lua_conn_.connected())
    //    restart_lua_conn_ = pconn_->op_ok_sig_.connect(boost::bind(&wx_vote_session::on_restart_lua_end, this, _1));

    boost::system::error_code ec;
    socket_.connect(boost::asio::ip::tcp::endpoint(
        boost::asio::ip::address_v4::from_string(ip_), port_num_),
        ec);
    if (ec)
    {
        //// 连接失败处理
        //pconn_->restart_lua();
        //restart_lua_timer_.async_wait(boost::bind(&wx_vote_session::on_restart_lua_timeout, this));
        //ios_.run();
    }
    else
    {
        std::string script = pt_.get<std::string>("script");
        std::string vote_url = pt_.get<std::string>("url");

        std::set<std::string> acs =
            wx_account_mgr::get().get_address_pro_copy().at(ip_);
        for (auto c : acs)
        {
            if (wx_account_mgr::get().get_vote_status(c, vote_url) == WX_ACCOUNT_STATUS_VOTED)
            {
                continue;
            }

            std::map<std::string, WX_ACCOUNT_PRO> apc = wx_account_mgr::get().get_accounts_pro_copy();
            boost::format fmterA(kvote_cmd_format);
            fmterA%CMD_WX_LOGIN
                %c
                %apc[c].password
                %apc[c].imei
                %CMD_DOWNLOAD_FILE
                %script
                %kdownload_to_path
                %CMD_WX_VOTE;
            boost::format fmterB("%|1$04d|%2%");
            fmterB % fmterA.str().length() % fmterA.str();

            //MSG_CMD_LUA mcl;
            //mcl.len = fmterB.str();
            //mcl.json_data = fmterA.str();

            //std::ostringstream  oarchiveStream;
            //boost::archive::text_oarchive  oarchive(oarchiveStream);
            //oarchive << mcl;

            socket_.write_some(boost::asio::buffer(fmterB.str()), ec);
            if (ec)
            {
                //pconn_->restart_lua();
                //restart_lua_timer_.async_wait(boost::bind(&wx_vote_session::on_restart_lua_timeout, this));
                //ios_.run();
            }
            else
            {
                //设置异步超时
                read_timer_.async_wait(boost::bind(&wx_vote_session::on_read_timeout, this));
                socket_.async_read_some(boost::asio::buffer(buffer_),
                    boost::bind(&wx_vote_session::handle_read,
                    this,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred, c, vote_url));
                ios_.run();
            }

        }
        socket_.close();
    }

    //设置当前设备空闲
    device_mgr::get().set_status(ip_, DEVICE_STATUS_IDLE);
}

void wx_vote_session::handle_read(const boost::system::error_code& e,
    std::size_t bytes_transferred,
    const std::string& acc,
    const std::string& url)
{
    if (!e)//接收时没有发生错误
    {
        MSG_CMD_LUA mcl;
        mcl.json_data = buffer_.data() + 4;

        static std::stringstream ss;
        ss.clear();
        ss << mcl.json_data;

        boost::property_tree::ptree pt;
        boost::property_tree::read_json<boost::property_tree::ptree>(ss, pt);

        if (0 == pt.get<int>("result"))
        {
            wx_account_mgr::get().set_vote_status(acc, url, WX_ACCOUNT_STATUS_VOTED);
            wv_->add_suc_count(1);
        }

    }
    else if (e != boost::asio::error::operation_aborted)
    {
        pconn_->restart_lua();
        restart_lua_timer_.async_wait(boost::bind(&wx_vote_session::on_restart_lua_timeout, this));
        ios_.run();
    }
}

void wx_vote_session::on_restart_lua_end(int cmd)
{
    if (cmd != CMD_APP_RESTART_LUA)
    {
        return;
    }

    restart_lua_timer_.cancel();
    start();
}

void wx_vote_session::on_restart_lua_timeout()
{
    if (restart_lua_conn_.connected())
    {
        restart_lua_conn_.disconnect();
    }
}

void wx_vote_session::on_read_timeout()
{

}

}