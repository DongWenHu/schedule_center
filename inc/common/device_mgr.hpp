#ifndef __DEVICE_MGR_HPP_2016_05_06__
#define __DEVICE_MGR_HPP_2016_05_06__

#include <set>
#include <map>
#include "typedefs.h"
#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <boost/smart_ptr.hpp>

namespace mpsp{

const int DEVICE_STATUS_ONLINE_IDLE = 1;        //  ���߿���״̬
const int DEVICE_STATUS_OFFLINE = 2;            //  ����״̬
const int DEVICE_STATUS_TASK_BEGIN = 3;         //  ����ʼ״̬
const int DEVICE_STATUS_WX_VOTING = 4;          //  ΢��ͶƱ��״̬
const int DEVICE_STATUS_WX_VOTE_END = 5;        //  ΢��ͶƱ����״̬
const int DEVICE_STATUS_DAILY_TASK = 6;         //  ִ���ճ�����״̬
const int DEVICE_STATUS_DAILY_TASK_END = 7;     //  ִ���ճ��������״̬

typedef struct _DEVICE_PRO
{
    int status;
}DEVICE_PRO;

class device_mgr
{
    rwmutex rwmutex_;
    std::map<std::string, DEVICE_PRO> devices_pro;  //  key is ip
    static device_mgr device_mgr_;

protected:
    device_mgr();

public:
    ~device_mgr();

    static device_mgr& get(){ return device_mgr_; }

    bool init();

    bool init_sql_conn();

    void set_status(const std::string& ip, int status);
    
    int get_status(const std::string& ip);

    int get_status_count(int status);

    std::map<std::string, DEVICE_PRO> get_devices_pro_copy();

	sql::Driver * driver;
	sql::Connection* con;
	sql::Statement* stmt;

};

}

#endif  //  __DEVICE_MGR_HPP_2016_05_06__