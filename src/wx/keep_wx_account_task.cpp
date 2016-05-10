#include <boost/random/mersenne_twister.hpp>
#include "keep_wx_account_task.hpp"
#include "device_mgr.hpp"
#include "task_processor.hpp"
#include "wx_account_mgr.hpp"
#include "keep_wx_account_session.hpp"

namespace mpsp{

keep_wx_account_task::keep_wx_account_task()
{

}

void keep_wx_account_task::start()
{
    wx_account_mgr::get().reset_all_everyday_task();

    // 每日24:00重置养账号任务
    boost::posix_time::ptime p(boost::gregorian::day_clock::local_day() + boost::gregorian::days(1),
        boost::posix_time::hours(0 - 8));
    mpsp::task_processor::get().run_at(p, boost::bind(&keep_wx_account_task::start, this));

    // 每日时间(8:00 - 20:00)随机时间段执行任务，并每隔1个小时重新执行一次直至23:00
    boost::mt19937  rng((const unsigned int)time(0));
    int h = rng() % 12 + 8 - 8;
    for (size_t i = h; i < 24; ++i)
    {
        boost::posix_time::ptime p2(boost::gregorian::day_clock::local_day(),
            boost::posix_time::hours(i) +
            boost::posix_time::minutes(rng() % 60) +
            boost::posix_time::seconds(rng() % 60));
        mpsp::task_processor::get().run_at(p2, boost::bind(&keep_wx_account_task::do_task, this));
    }

}

void keep_wx_account_task::do_task()
{
    boost::thread_group threads;
    std::vector<boost::shared_ptr<keep_wx_account_session> > keep_wx_acc_seses;

    for (auto c : device_mgr::get().get_devices_pro_copy())
    {
        if (c.second.status != DEVICE_STATUS_ONLINE)
            continue;

        boost::shared_ptr<keep_wx_account_session> keep_wx_acc_ses(new keep_wx_account_session(c.first, SCRIPT_PORT));
        keep_wx_acc_seses.push_back(keep_wx_acc_ses);
        threads.create_thread(boost::bind(&keep_wx_account_session::start, keep_wx_acc_ses));
    }

    threads.join_all();
}

}