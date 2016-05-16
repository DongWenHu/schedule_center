#include "wx_account_mgr.hpp"
#include "config_mgr.hpp"

namespace mpsp{

wx_account_mgr wx_account_mgr::wx_account_mgr_;

wx_account_mgr::wx_account_mgr()
{
    
}

wx_account_mgr::~wx_account_mgr()
{
    if (con_acc && !con_acc->isClosed())
    {
        con_acc->close();
    }
}

bool wx_account_mgr::init()
{
    // read from database
    try {
        /* Create a connection */
        driver = get_driver_instance();
        con_acc = driver->connect(config_mgr::get().get_value("mysql", "ip").c_str(),
            config_mgr::get().get_value("mysql", "usr").c_str(),
            config_mgr::get().get_value("mysql", "pass").c_str());
        /* Connect to the MySQL test database */
        con_acc->setSchema("schedulecenter");
        stmt_acc = con_acc->createStatement();
        res_acc = stmt_acc->executeQuery("select * from wx_accounts_info;");
        while (res_acc->next()) {

            WX_ACCOUNT_PRO account_pro;
            /* Access column data by alias or column name */
            account_pro.imei = res_acc->getString("imei").c_str();
            account_pro.imsi = res_acc->getString("imsi").c_str();
            account_pro.ip = res_acc->getString("ip").c_str();
            //account_pro.mac = res_acc->getString("mac").c_str();
            account_pro.nickname = res_acc->getString("nick").c_str();
            account_pro.password = res_acc->getString("passwd").c_str();
            account_pro.wxh = res_acc->getString("wxh").c_str();
            account_pro.everyday_task_completed = res_acc->getBoolean("dailytask");

            std::string account_str = res_acc->getString("acc").c_str();
            address_pro[account_pro.ip].insert(account_str);

            sql::PreparedStatement  *prep_stmt;
            prep_stmt = con_acc->prepareStatement("select * from wx_vote_url_status where acc = ?");
            prep_stmt->setString(1, account_str.c_str());
            res_vote_url = prep_stmt->executeQuery();
            while (res_vote_url->next()) {
                account_pro.voted_url.insert(res_vote_url->getString("url").c_str());
            }
            delete prep_stmt;


            accounts_pro.insert(std::map<std::string, WX_ACCOUNT_PRO>::value_type(account_str, account_pro));

            /* Access column fata by numeric offset, 1 is the first column */
            //res->getString(1);
        }

    }
    catch (sql::SQLException &e) {
        printf("# ERR: %s", e.what());
        printf(" (MySQL error code: %d", e.getErrorCode());
        printf(", SQLState: %s\n", e.getSQLState());
        return false;
    }

    return true;
}

void wx_account_mgr::insert(const std::string& account, WX_ACCOUNT_PRO& account_pro)
{
    writeLock lock(rwmutex_);
    // write to database
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
        stmt_acc->execute(tmp_sql);
    }
    catch (sql::SQLException &e) {
        printf("wx_account_mgr::set_vote_status:  %s\n", e.what());
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
    char tmp_sql[6400] = { 0 };
    sprintf(tmp_sql, "update `wx_accounts_info` set `dailytask` = '%d' where `acc` = '%s';", b_done, account.c_str());
    try{
        stmt_acc->execute(tmp_sql);
    }
    catch (sql::SQLException &e) {
        printf("wx_account_mgr::set_vote_status:  %s\n", e.what());
    }
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