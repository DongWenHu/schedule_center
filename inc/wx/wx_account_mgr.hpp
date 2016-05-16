#ifndef __WX_ACCOUNT_MGR_HPP_2016_05_03__
#define __WX_ACCOUNT_MGR_HPP_2016_05_03__

#include <set>
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

// weixin account property 
typedef struct _WX_ACCOUNT_PRO
{
    std::string password;    //    account's password
    std::string nickname;    //    account's nickname
    std::string ip; // this account belongs to which device's ip
    std::string imei;   //  device's imei
    std::string imsi;    //    device's imsi
    std::string wxh;    // 微信号

    int everyday_task_completed;    //    whether keep account task is completed, reset will be at 24:00 
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
    std::map<std::string, std::set<std::string> > address_pro;   //  一个ip地址对应微信账号集合

public:
    ~wx_account_mgr();

    static wx_account_mgr& get(){ return wx_account_mgr_; }

    bool init();

    void insert(const std::string& account, WX_ACCOUNT_PRO& account_pro);

    int get_vote_status(const std::string& account, const std::string& url);

    void set_vote_status(const std::string& account, const std::string& url, int status);

    void set_everyday_task(const std::string& account, int b_done);
    
    void reset_all_everyday_task();

    std::map<std::string, WX_ACCOUNT_PRO> get_accounts_pro_copy();

    std::map<std::string, std::set<std::string> > get_address_pro_copy();

	sql::Driver * driver;
	sql::Connection* con_acc;
    sql::Statement* stmt_acc;
    sql::ResultSet* res_acc;
    sql::Connection* con_vote_url;
    sql::ResultSet* res_vote_url;
};

};

#endif //   __WX_ACCOUNT_MGR_HPP_2016_05_03__