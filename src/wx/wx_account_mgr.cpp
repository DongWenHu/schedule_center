#include "wx_account_mgr.hpp"
#include "config_mgr.hpp"

namespace mpsp{

wx_account_mgr wx_account_mgr::wx_account_mgr_;

wx_account_mgr::wx_account_mgr()
    : driver(NULL)
    , con_acc(NULL)
    , stmt_acc(NULL)
{
    
}

wx_account_mgr::~wx_account_mgr()
{
    if (con_acc && !con_acc->isClosed())
    {
        con_acc->close();
    }

    if (stmt_acc)
    {
        stmt_acc->close();
    }
}

bool wx_account_mgr::init()
{
    driver = get_driver_instance();

    if (!init_sql_conn() ||
        !load_data())
        return false;

    return true;
}

bool wx_account_mgr::init_sql_conn()
{
    if (con_acc && !con_acc->isClosed())
    {
        con_acc->close();
    }
    try
    {
        con_acc = driver->connect(config_mgr::get().get_value("mysql", "ip").c_str(),
            config_mgr::get().get_value("mysql", "usr").c_str(),
            config_mgr::get().get_value("mysql", "pass").c_str());
        /* Connect to the MySQL test database */
        con_acc->setSchema("schedulecenter");
        if (stmt_acc)
            stmt_acc->close();
        stmt_acc = con_acc->createStatement();
    }
    catch (sql::SQLException &e) {
        printf("# ERR: %s", e.what());
        printf(" (MySQL error code: %d", e.getErrorCode());
        printf(", SQLState: %s\n", e.getSQLState().c_str());
        return false;
    }

    return true;
}

bool wx_account_mgr::load_data()
{
    accounts_pro.clear();
    idle_accounts.clear();
    try
    {
        sql::ResultSet* res_acc;
        res_acc = stmt_acc->executeQuery("select * from wx_accounts_info;");
        while (res_acc->next())
        {
            WX_ACCOUNT_PRO account_pro;
            /* Access column data by alias or column name */
            account_pro.imei = res_acc->getString("imei").c_str();
            account_pro.imsi = res_acc->getString("imsi").c_str();
            account_pro.nickname = res_acc->getString("nick").c_str();
            account_pro.password = res_acc->getString("passwd").c_str();
            account_pro.wxh = res_acc->getString("wxh").c_str();
            account_pro.everyday_task_completed = res_acc->getInt("dailytask");
            account_pro.status = res_acc->getInt("status");

            std::string account_str = res_acc->getString("acc").c_str();
            idle_accounts.push_back(account_str);

            sql::PreparedStatement  *prep_stmt;
            sql::ResultSet* res_vote_url;
            prep_stmt = con_acc->prepareStatement("select * from wx_vote_url_status where acc = ?");
            prep_stmt->setString(1, account_str.c_str());
            res_vote_url = prep_stmt->executeQuery();
            while (res_vote_url->next()) {
                account_pro.voted_url.insert(res_vote_url->getString("url").c_str());
            }
            prep_stmt->close();
            res_vote_url->close();

            accounts_pro.insert(std::map<std::string, WX_ACCOUNT_PRO>::value_type(account_str, account_pro));

        }
        res_acc->close();

    }
    catch (sql::SQLException &e) {
        printf("# ERR: %s", e.what());
        printf(" (MySQL error code: %d", e.getErrorCode());
        printf(", SQLState: %s\n", e.getSQLState().c_str());
        return false;
    }

    return true;
}

int wx_account_mgr::get_vote_status(const std::string& account, const std::string& url)
{
    readLock lock(rwmutex_);

    if (accounts_pro.find(account) == accounts_pro.end())
    {
        return WX_ACCOUNT_STATUS_VOTED;
    }

    if (accounts_pro[account].voted_url.find(url) == accounts_pro[account].voted_url.end())
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

    accounts_pro[account].voted_url.insert(url);
	char tmp_sql[6400] = { 0 };
	sprintf(tmp_sql, "insert into wx_vote_url_status set acc = '%s' ,url = '%s';", account.c_str(), url.c_str());
    try{
        if (!con_acc->isValid())
        {
            init_sql_conn();
        }
        stmt_acc->execute(tmp_sql);
    }
    catch (sql::SQLException &e) {
        printf("wx_account_mgr::set_vote_status:  %s\n", e.what());
    }
}

void wx_account_mgr::set_acc_status(const std::string& account, int status)
{
    writeLock lock(rwmutex_);

    if (accounts_pro.find(account) == accounts_pro.end())
    {
        return;
    }

    accounts_pro[account].status = status;
    char tmp_sql[640] = { 0 };
    sprintf(tmp_sql, "update `wx_accounts_info` set `status` = '%d' where `acc` = '%s';", status, account.c_str());
    try{
        if (!con_acc->isValid())
        {
            init_sql_conn();
        }
        stmt_acc->execute(tmp_sql);
    }
    catch (sql::SQLException &e) {
        printf("wx_account_mgr::set_acc_status:  %s\n", e.what());
    }

}

void wx_account_mgr::set_everyday_task(const std::string& account, int b_done)
{
    writeLock lock(rwmutex_);

    if (accounts_pro.find(account) == accounts_pro.end())
    {
        return;
    }

    accounts_pro[account].everyday_task_completed = b_done;
    char tmp_sql[640] = { 0 };
    sprintf(tmp_sql, "update `wx_accounts_info` set `dailytask` = '%d' where `acc` = '%s';", b_done, account.c_str());
    try{
        if (!con_acc->isValid())
        {
            init_sql_conn();
        }
        stmt_acc->execute(tmp_sql);
    }
    catch (sql::SQLException &e) {
        printf("wx_account_mgr::set_vote_status:  %s\n", e.what());
    }
}

void wx_account_mgr::reset_all_everyday_task()
{
    //writeLock lock(rwmutex_);
    for (auto var : accounts_pro)
    {
        set_everyday_task(var.first, WX_ACCOUNT_DAILYTASK_NOT_FINISHED);
        //var.second.everyday_task_completed = false;
    }
}

std::map<std::string, WX_ACCOUNT_PRO> wx_account_mgr::get_accounts_pro_copy()
{
    readLock lock(rwmutex_);
    return accounts_pro;
}

bool wx_account_mgr::get_idle_not_vote_account(std::string& acc, const std::string& vote_url)
{
    readLock rlock(rwmutex_);
    std::list<std::string>::iterator itor = idle_accounts.begin();
    for (; itor != idle_accounts.end(); ++itor)
    {
        if (get_vote_status(*itor, vote_url) == WX_ACCOUNT_STATUS_VOTED ||
            accounts_pro[*itor].status == WX_ACCOUNT_STATUS_ABNORMAL)
            continue;

        rlock.unlock();
        acc = *itor;
        writeLock wlock(rwmutex_);
        idle_accounts.erase(itor);
        return true;
    }

    return false;
}

int wx_account_mgr::get_idle_not_vote_count(const std::string& vote_url)
{
    readLock lock(rwmutex_);

    int cnt = 0;
    for (auto c : idle_accounts)
    {
        if (get_vote_status(c, vote_url) == WX_ACCOUNT_STATUS_NOT_VOTE &&
            accounts_pro[c].status == WX_ACCOUNT_STATUS_NORMAL)
        {
            ++cnt;
        }
    }
    return cnt;
}

bool wx_account_mgr::get_idle_not_daily_task_account(std::string& acc)
{
    readLock rlock(rwmutex_);
    std::list<std::string>::iterator itor = idle_accounts.begin();
    for (; itor != idle_accounts.end(); ++itor)
    {
        if (accounts_pro[*itor].everyday_task_completed == WX_ACCOUNT_DAILYTASK_FINISHED ||
            accounts_pro[*itor].status == WX_ACCOUNT_STATUS_ABNORMAL)
            continue;

        rlock.unlock();
        acc = *itor;
        writeLock wlock(rwmutex_);
        idle_accounts.erase(itor);
        return true;
    }

    return false;
}

void wx_account_mgr::set_idle_account(const std::string& acc)
{
    writeLock lock(rwmutex_);
    idle_accounts.push_back(acc);
}

}