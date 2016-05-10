#ifndef __DEVICE_MGR_HPP_2016_05_06__
#define __DEVICE_MGR_HPP_2016_05_06__

#include <set>
#include <map>
#include "typedefs.h"

namespace mpsp{

const int DEVICE_STATUS_ONLINE = 1;
const int DEVICE_STATUS_OFFLINE = 2;
const int DEVICE_STATUS_BUSY = 3;
const int DEVICE_STATUS_IDLE = 1;

typedef struct _DEVICE_PRO
{
    int status;
}DEVICE_PRO;

class device_mgr
{
    rwmutex rwmutex_;
    std::map<std::string, DEVICE_PRO> devices_pro;
    static device_mgr device_mgr_;

protected:
    device_mgr();

public:
    static device_mgr& get(){ return device_mgr_; }

    void set_status(const std::string& ip, int status);
    
    int get_status(const std::string& ip);

    std::map<std::string, DEVICE_PRO> get_devices_pro_copy();

};

}

#endif  //  __DEVICE_MGR_HPP_2016_05_06__