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

    std::set<std::string> acs =
        wx_account_mgr::get().get_address_pro_copy().at(ip_);
    // 重复执行3次
    int repeat_cnt = 3;
    do
    {
        for (auto c : acs)
        {
            if (wx_account_mgr::get().get_vote_status(c, vote_url) == WX_ACCOUNT_STATUS_VOTED)
            {
                continue;
            }

            if (!wv_->need_vote())
            {
                break;
            }

            std::map<std::string, WX_ACCOUNT_PRO> apc = wx_account_mgr::get().get_accounts_pro_copy();
            boost::format fmterA(kvote_cmd_format);
            fmterA%CMD_WX_LOGIN
                %c
                %apc[c].password
                %apc[c].imei
                %CMD_WX_ADD_MP
                %vote_mp
                %CMD_WX_CLEAR_MSG_RECORD
                %apc[c].nickname
                %CMD_WX_SEND_MSG
                %apc[c].nickname
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
                    boost::asio::placeholders::bytes_transferred, c, vote_url));
                ios_.run();
                ios_.reset();
            }

        }
        repeat_cnt--;
        if (repeat_cnt == 0)
        {
            break;
        }
    } while (true);
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
                wx_account_mgr::get().set_vote_status(acc, url, WX_ACCOUNT_STATUS_VOTED);
                wv_->add_suc_count(1);
                read_timer_.cancel();
            }
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
}

void wx_vote_session::on_restart_lua_end(int cmd)
{
    if (cmd != CMD_APP_RESTART_LUA)
    {
        return;
    }

    restart_lua_timer_.cancel();
    device_mgr::get().set_status(ip_, DEVICE_STATUS_IDLE);
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