#ifndef __WX_VOTE_HPP_2016_05_03__
#define __WX_VOTE_HPP_2016_05_03__

#include <sstream>
#include <string>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/thread/mutex.hpp>
#include "task_interface.h"
#include "typedefs.h"

namespace mpsp{

class wx_vote : public task_interface
{
private:
    rwmutex suc_cnt_mutex;
    int success_count;
    int target_count;

public:
    wx_vote();

    void do_task(boost::property_tree::ptree& pt, std::string& result);

    void add_suc_count(int);

    int get_suc_count();

    bool need_vote();

    bool need_wait_vote();
};

}
#endif  //  __WX_VOTE_HPP_2016_05_03__