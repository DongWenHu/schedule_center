#ifndef __WX_VOTE_HPP_2016_05_03__
#define __WX_VOTE_HPP_2016_05_03__

#include <sstream>
#include <string>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/thread/mutex.hpp>
#include "task_interface.h"

namespace mpsp{

class wx_vote : public task_interface
{
private:
    boost::mutex   suc_cnt_mutex;
    int success_count;

public:
    wx_vote();

    void do_task(boost::property_tree::ptree& pt, std::string& result);

    void add_suc_count(int);
};

}
#endif  //  __WX_VOTE_HPP_2016_05_03__