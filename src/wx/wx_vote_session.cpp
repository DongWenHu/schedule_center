#include "wx_vote_session.hpp"
#include <boost/archive/iterators/base64_from_binary.hpp>  
#include <boost/archive/iterators/binary_from_base64.hpp>  
#include <boost/archive/iterators/transform_width.hpp>
#include <iostream>

namespace mpsp{

// 登录->添加公众号->清除自己聊天记录->发送投票URL给自己->下载投票脚本->投票->登出
const char* const kvote_cmd_format = "["
    "[\"%1%\",\"%2%\",\"%3%\",\"%4%\"],"
    "[\"%5%\",\"%6%\"],"
    "[\"%7%\",\"%8%\"],"
    "[\"%9%\",\"%10%\",\"%11%\"],"
    "[\"%12%\",\"%13%\",\"%14%\"],"
    "[\"%15%\"],"
    "[\"%16%\"]"
    "]";
const char* const kdownload_to_path = "/mnt/sdcard/TouchSprite/lua/wx/wx_vote.lua";

typedef boost::archive::iterators::transform_width<boost::archive::iterators::binary_from_base64<std::string::const_iterator>, 8, 6> Base64DecodeIterator;

wx_vote_session::wx_vote_session(std::string ip,
    unsigned short port_num,
    boost::property_tree::ptree& pt,
    wx_vote* wv)
    : a_session(ip, port_num)
    , pt_(pt)
    , wv_(wv)
{

}

void wx_vote_session::bind_restart_lua_sig()
{
    restart_lua_conn_ = pconn_->op_ok_sig_.connect(boost::bind(&wx_vote_session::on_restart_lua_end, this, _1));
}

void wx_vote_session::on_connected()
{
    boost::system::error_code ec;
    std::string script = pt_.get<std::string>("script");
    std::string vote_url = pt_.get<std::string>("url");
    std::string vote_mp = pt_.get<std::string>("MP");

    std::stringstream vote_url_dec;
    copy(Base64DecodeIterator(vote_url.begin()), Base64DecodeIterator(vote_url.end()), std::ostream_iterator<char>(vote_url_dec));

    std::string acc; 
    while (wx_account_mgr::get().get_idle_not_vote_account(acc, vote_url))
    {
        while (wv_->need_wait_vote())
        {
            if (!wv_->need_vote())
            {
                return;
            }
            boost::this_thread::sleep(boost::posix_time::seconds(3));
        }

        device_mgr::get().set_status(ip_, DEVICE_STATUS_WX_VOTING);
        std::map<std::string, WX_ACCOUNT_PRO> apc = wx_account_mgr::get().get_accounts_pro_copy();
        boost::format fmterA(kvote_cmd_format);
        fmterA%CMD_WX_LOGIN
            %acc
            %apc[acc].password
            %apc[acc].imei
            %CMD_WX_ADD_MP
            %vote_mp
            %CMD_WX_CLEAR_MSG_RECORD
            %apc[acc].nickname
            %CMD_WX_SEND_MSG
            %apc[acc].nickname
            %vote_url_dec.str()
            % CMD_DOWNLOAD_FILE
            %script
            %kdownload_to_path
            %CMD_WX_VOTE
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
            read_timer_.async_wait(boost::bind(&wx_vote_session::on_read_timeout,
                this,
                boost::asio::placeholders::error));
            socket_.async_read_some(boost::asio::buffer(buffer_),
                boost::bind(&wx_vote_session::handle_read,
                this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred, acc, vote_url));
            ios_.run();
            ios_.reset();
        }

    }
}

void wx_vote_session::handle_read(const boost::system::error_code& e,
    std::size_t bytes_transferred,
    const std::string& acc,
    const std::string& url)
{
    read_timer_.cancel();
    if (!e)//接收时没有发生错误
    {
        try
        {
            char len[5] = {0};
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
            _DEBUG_PRINTF("%s (phone vote) says: %s\n", socket_.remote_endpoint().address().to_string().c_str(), json_data);
            if (0 == pt.get<int>("result"))
            {
                login_failed_cnt_.clear();
                wx_account_mgr::get().set_vote_status(acc, url, WX_ACCOUNT_STATUS_VOTED);
                wv_->add_suc_count(1);
            }
            else
            {
                if (pt.get<std::string>("cmd") == CMD_WX_LOGIN)
                {
                    if (pt.get<std::string>("message") == "Illegal account" ||
                        pt.get<std::string>("message") == "User or password wrong" ||
                        pt.get<std::string>("message") == "Need unlock" ||
                        pt.get<std::string>("message") == "Need SMS verify" || 
                        pt.get<std::string>("message") == "Need codes")
                    {
                        wx_account_mgr::get().set_acc_status(acc, WX_ACCOUNT_STATUS_ABNORMAL);
                    }
                    //else
                    //{
                    //    if (login_failed_cnt_.find(acc) == login_failed_cnt_.end())
                    //    {
                    //        login_failed_cnt_[acc] = 1;
                    //    }
                    //    else
                    //    {
                    //        login_failed_cnt_[acc]++;
                    //        if (login_failed_cnt_[acc] > 3)
                    //        {
                    //            wx_account_mgr::get().set_acc_status(acc, WX_ACCOUNT_STATUS_ABNORMAL);
                    //        }
                    //    }
                    //}
                }
            }
            wx_account_mgr::get().set_idle_account(acc);
            delete json_data;
        }
        catch (boost::property_tree::json_parser_error&)
        {
            _DEBUG_PRINTF("%s (phone vote) json parse error\n", socket_.remote_endpoint().address().to_string().c_str());
        }
    }
    else if (e != boost::asio::error::operation_aborted)
    {

    }
    device_mgr::get().set_status(ip_, DEVICE_STATUS_WX_VOTE_END);
}

void wx_vote_session::on_restart_lua_end(int cmd)
{
    if (cmd != CMD_APP_RESTART_LUA)
    {
        return;
    }

    restart_lua_timer_.cancel();
    device_mgr::get().set_status(ip_, DEVICE_STATUS_ONLINE_IDLE);
    start();
}

void wx_vote_session::on_read_timeout(const boost::system::error_code& e)
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
        restart_lua_timer_.async_wait(boost::bind(&wx_vote_session::on_restart_lua_timeout,
            this,
            boost::asio::placeholders::error));
    }
}

}