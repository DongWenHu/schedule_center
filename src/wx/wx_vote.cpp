#include "wx_vote.hpp"
#include "device_mgr.hpp"
#include "wx_vote_session.hpp"
#include "protocol_define.h"
#include <boost/format.hpp>

namespace mpsp{

const char* const kvote_ret_fmt = "{\"task_id\":%1%,\"task_cmd\":%2%,\"result_code\":%3%,\"success count\":%4%,\"msg\":\"%5%\"}";

wx_vote::wx_vote()
    : success_count(0)
{

}

void wx_vote::do_task(boost::property_tree::ptree& pt, std::string& result)
{
    target_count = pt.get<int>("votes_cnt");
    std::string vote_url = pt.get<std::string>("url");
    boost::format fmter(kvote_ret_fmt);

    if (wx_account_mgr::get().get_idle_not_vote_count(vote_url) < target_count)
    {
        fmter % pt.get<int>("task_id")
            % pt.get<int>("task_cmd")
            % -1
            % 0
            % "There's no too many accounts.";
        result = fmter.str();
        return;
    }

    boost::thread_group threads;
    for (auto c : device_mgr::get().get_devices_pro_copy())
    {
        if (c.second.status != DEVICE_STATUS_ONLINE_IDLE)
            continue;

        boost::shared_ptr<wx_vote_session> wx_vote_ses(new wx_vote_session(c.first, SCRIPT_PORT, pt, this));
        threads.create_thread(boost::bind(&wx_vote_session::start, wx_vote_ses));
    }

    threads.join_all();

    fmter % pt.get<int>("task_id")
        % pt.get<int>("task_cmd")
        % 0
        % success_count
        % "ok";
    result = fmter.str();
    success_count = 0;
}

void wx_vote::add_suc_count(int n)
{
    writeLock lock(suc_cnt_mutex);
    success_count += n;
}

int wx_vote::get_suc_count()
{
    readLock lock(suc_cnt_mutex);
    return success_count;
}

bool wx_vote::need_vote()
{
    readLock lock(suc_cnt_mutex);
    return target_count > success_count;
}

bool wx_vote::need_wait_vote()
{
    readLock lock(suc_cnt_mutex);

    int _vc = device_mgr::get().get_status_count(DEVICE_STATUS_WX_VOTING);
    return (success_count + _vc) == target_count;
}

}