#include "wx_vote.hpp"
#include "device_mgr.hpp"
#include "wx_vote_session.hpp"
#include "protocol_define.h"
#include <boost/format.hpp>

namespace mpsp{

const char* const kvote_ret_fmt = "{\"task_id\":%1%,\"task_cmd\":%2%,\"result_code\":%3%,\"success count\":%4%}";

wx_vote::wx_vote()
    : success_count(0)
{

}

void wx_vote::do_task(boost::property_tree::ptree& pt, std::string& result)
{
    // 重复执行3次
    int repeat_cnt = 3;
    do
    {
        boost::thread_group threads;
        std::vector<boost::shared_ptr<wx_vote_session> > wx_vote_seses;
        for (auto c : device_mgr::get().get_devices_pro_copy())
        {
            if (c.second.status != DEVICE_STATUS_ONLINE)
                continue;

            boost::shared_ptr<wx_vote_session> wx_vote_ses(new wx_vote_session(c.first, SCRIPT_PORT, pt, this));
            wx_vote_seses.push_back(wx_vote_ses);
            threads.create_thread(boost::bind(&wx_vote_session::start, wx_vote_ses));
        }

        threads.join_all();
        --repeat_cnt;

        // 达到投票指定数目或者重复执行了3次就退出循环
        if (pt.get<int>("votes_cnt") <= success_count || repeat_cnt == 0)
        {
            break;
        }
    } while (true);

    boost::format fmter(kvote_ret_fmt);
    fmter % pt.get<int>("task_id")
        % pt.get<int>("task_cmd")
        % 0
        % success_count;
    result = fmter.str();
}

void wx_vote::add_suc_count(int n)
{
    boost::unique_lock<boost::mutex> lock(suc_cnt_mutex);
    success_count += n;
    lock.unlock();
}

}