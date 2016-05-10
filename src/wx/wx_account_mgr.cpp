#include "wx_account_mgr.hpp"

namespace mpsp{

wx_account_mgr wx_account_mgr::wx_account_mgr_;

wx_account_mgr::wx_account_mgr()
{
    // read from database
    WX_ACCOUNT_PRO wap;
    wap.password = "h12345687";
    wap.imei = "";
    wap.ip = "192.168.1.36";
    wap.imei = "866980028373153";
    wap.everyday_task_completed = false;
    accounts_pro["wxid_iefihiaixr9352"] = wap;

    wap.password = "12qwaszx!@";
    accounts_pro["162740596"] = wap;
    accounts_pro["162740619"] = wap;

    std::set<std::string> acs;
    acs.insert("wxid_iefihiaixr9352");
    acs.insert("162740596");
    acs.insert("162740619");
    address_pro["192.168.1.36"] = acs;


}

void wx_account_mgr::insert(const std::string& account, WX_ACCOUNT_PRO& account_pro)
{
    writeLock lock(rwmutex_);
    // write to database
}

bool wx_account_mgr::is_everyday_task_completed(const std::string& account)
{
    readLock lock(rwmutex_);
    return false;
}

int wx_account_mgr::get_vote_status(const std::string& account, const std::string& url)
{
    readLock lock(rwmutex_);
    if (accounts_pro.find(account) == accounts_pro.end())
    {
        return WX_ACCOUNT_STATUS_VOTED;
    }

    if (accounts_pro[account].vote_status.find(url) == accounts_pro[account].vote_status.end())
    {
        return WX_ACCOUNT_STATUS_NOT_VOTE;
    }
    return WX_ACCOUNT_STATUS_VOTED;
}

void wx_account_mgr::set_vote_status(const std::string& account, const std::string& url, int status)
{
    writeLock lock(rwmutex_);
    if (accounts_pro.find(account) == accounts_pro.end())
    {
        return;
    }

    if (status == WX_ACCOUNT_STATUS_VOTED)
    {
        accounts_pro[account].vote_status.insert(url);
    }
    else
    {
        accounts_pro[account].vote_status.erase(url);
    }

}

void wx_account_mgr::set_everyday_task(const std::string& account, bool b_done)
{
    writeLock lock(rwmutex_);
    if (accounts_pro.find(account) == accounts_pro.end())
    {
        return;
    }

    accounts_pro[account].everyday_task_completed = b_done;
}

void wx_account_mgr::reset_all_everyday_task()
{
    writeLock lock(rwmutex_);
    for each (auto var in accounts_pro)
    {
        var.second.everyday_task_completed = false;
    }
}

std::map<std::string, WX_ACCOUNT_PRO> wx_account_mgr::get_accounts_pro_copy()
{
    readLock lock(rwmutex_);
    return accounts_pro;
}

std::map<std::string, std::set<std::string> > wx_account_mgr::get_address_pro_copy()
{
    readLock lock(rwmutex_);
    return address_pro;
}

}