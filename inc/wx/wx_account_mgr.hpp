#ifndef __WX_ACCOUNT_MGR_HPP_2016_05_03__
#define __WX_ACCOUNT_MGR_HPP_2016_05_03__

#include <set>
#include <list>
#include "typedefs.h"
#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <boost/smart_ptr.hpp>

namespace mpsp{

const int WX_ACCOUNT_STATUS_VOTED = 1;
const int WX_ACCOUNT_STATUS_NOT_VOTE = 2;

const int WX_ACCOUNT_DAILYTASK_NOT_FINISHED = 0;
const int WX_ACCOUNT_DAILYTASK_FINISHED = 1;

const int WX_ACCOUNT_STATUS_NORMAL = 0;
const int WX_ACCOUNT_STATUS_ABNORMAL = 1;

// weixin account property 
typedef struct _WX_ACCOUNT_PRO
{
    std::string password;    //    account's password
    std::string nickname;    //    account's nickname
    //std::string ip; // this account belongs to which device's ip  //del at 2016-05-18, reason: ip地址不再和账号相关联
    std::string imei;   //  device's imei
    std::string imsi;    //    device's imsi
    std::string wxh;    // 微信号

    int everyday_task_completed;    //    whether keep account task is completed, reset will be at 24:00 
    int status; // 0: normal, 1: abnormal
    std::set<std::string> voted_url;  // votee urls
}WX_ACCOUNT_PRO;

class wx_account_mgr
{
private:
    rwmutex rwmutex_;
    static wx_account_mgr wx_account_mgr_;

protected:
    wx_account_mgr();

    std::map<std::string, WX_ACCOUNT_PRO> accounts_pro;
    //del at 2016-05-18, reason: ip地址不再和账号相关联
    //std::map<std::string, std::set<std::string> > address_pro;   //  一个ip地址对应微信账号集合
    std::list<std::string> idle_accounts;

public:
    ~wx_account_mgr();

    static wx_account_mgr& get(){ return wx_account_mgr_; }

    bool init();

    bool init_sql_conn();

    bool load_data();

    void set_everyday_task(const std::string& account, int b_done);
    
    void reset_all_everyday_task();

    std::map<std::string, WX_ACCOUNT_PRO> get_accounts_pro_copy();

    int get_vote_status(const std::string& account, const std::string& url);

    void set_vote_status(const std::string& account, const std::string& url, int status);

    void set_acc_status(const std::string& account, int status);    //set normal or abnormal

    bool get_idle_not_vote_account(std::string& acc, const std::string& vote_url);

    int get_idle_not_vote_count(const std::string& vote_url);

    bool get_idle_not_daily_task_account(std::string& acc);

    void set_idle_account(const std::string& acc);

	sql::Driver * driver;
	sql::Connection* con_acc;
    sql::Statement* stmt_acc;
    
};

};

#endif //   __WX_ACCOUNT_MGR_HPP_2016_05_03__