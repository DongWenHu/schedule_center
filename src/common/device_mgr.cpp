#include "config_mgr.hpp"
#include "device_mgr.hpp"

namespace mpsp{

device_mgr device_mgr::device_mgr_;

device_mgr::device_mgr()
    : driver(NULL)
    , con(NULL)
    , stmt(NULL)
{

}

device_mgr::~device_mgr()
{
    if (con && !con->isClosed())
    {
        con->close();
    }

    if (stmt)
    {
        stmt->close();
    }
}

bool device_mgr::init()
{
    driver = get_driver_instance();
    if (!init_sql_conn())
        return false;

    try {
        /* Create a connection */
        sql::ResultSet* res;
        res = stmt->executeQuery("select * from device_info;");
        while (res->next()) {

            DEVICE_PRO device_pro;
            /* Access column data by alias or column name */
            //device_pro.ip = res->getString("ip").c_str();
            device_pro.status = DEVICE_STATUS_OFFLINE;/*atoi(res->getString("status").c_str())*/;
            std::string ip_str = res->getString("ip").c_str();
            devices_pro.insert(std::map<std::string, DEVICE_PRO>::value_type(ip_str, device_pro));
        }
        res->close();

    }
    catch (sql::SQLException &e) {
        printf("# ERR: %s", e.what());
        printf(" (MySQL error code: %d", e.getErrorCode());
        printf(", SQLState: %s\n", e.getSQLState().c_str());
        return false;
    }

    return true;
}

bool device_mgr::init_sql_conn()
{
    if (con && !con->isClosed())
    {
        con->close();
    }
    try
    {
        con = driver->connect(config_mgr::get().get_value("mysql", "ip").c_str(),
            config_mgr::get().get_value("mysql", "usr").c_str(),
            config_mgr::get().get_value("mysql", "pass").c_str());
        /* Connect to the MySQL test database */
        con->setSchema("schedulecenter");
        stmt = con->createStatement();
    }
    catch (sql::SQLException &e) {
        printf("# ERR: %s", e.what());
        printf(" (MySQL error code: %d", e.getErrorCode());
        printf(", SQLState: %s\n", e.getSQLState().c_str());
        return false;
    }

    return true;
}

void device_mgr::set_status(const std::string& ip, int status)
{
    writeLock lock(rwmutex_);
    if (devices_pro[ip].status == status)
        return;
    devices_pro[ip].status = status;
    char tmp_sql[640] = { 0 };
    sprintf(tmp_sql, "INSERT INTO device_info (`ip`, `status`) VALUES ('%s', '%d') ON DUPLICATE KEY UPDATE `status`='%d'", 
        ip.c_str(), status, status);
    try
    {
        
        if (!con->isValid())
        {
            init_sql_conn();
        }
        stmt->execute(tmp_sql);
    }
    catch (sql::SQLException &e) {
        printf("device_mgr::set_status: %s\n", e.what());
    }
}

int device_mgr::get_status(const std::string& ip)
{
    readLock lock(rwmutex_);
    if (devices_pro.find(ip) != devices_pro.end())
        return devices_pro[ip].status;

    return DEVICE_STATUS_OFFLINE;
}

int device_mgr::get_status_count(int status)
{
    readLock lock(rwmutex_);

    int cnt = 0;
    for (auto c : devices_pro)
    {
        if (c.second.status == status)
        {
            ++cnt;
        }
    }
    return cnt;
}

std::map<std::string, DEVICE_PRO> device_mgr::get_devices_pro_copy()
{
    readLock lock(rwmutex_);
    return devices_pro;
}

}