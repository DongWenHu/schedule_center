#include "device_mgr.hpp"

namespace mpsp{

device_mgr device_mgr::device_mgr_;

device_mgr::device_mgr()
{
    DEVICE_PRO dp;
    dp.status = true;
    devices_pro["192.168.1.36"] = dp;
}

void device_mgr::set_status(const std::string& ip, int status)
{
    writeLock lock(rwmutex_);
    if (devices_pro.find(ip) != devices_pro.end())
        devices_pro[ip].status = status;
}

int device_mgr::get_status(const std::string& ip)
{
    readLock lock(rwmutex_);
    if (devices_pro.find(ip) != devices_pro.end())
        return devices_pro[ip].status;

    return DEVICE_STATUS_OFFLINE;
}

std::map<std::string, DEVICE_PRO> device_mgr::get_devices_pro_copy()
{
    readLock lock(rwmutex_);
    return devices_pro;
}

}