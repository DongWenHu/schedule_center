#ifndef __TASK_INTERFACE_HPP_2016_05_03__
#define __TASK_INTERFACE_HPP_2016_05_03__

namespace mpsp{

class task_interface
{
public:
    virtual void do_task(boost::property_tree::ptree& pt, std::string& result) = 0;
};

}

#endif  //  __TASK_INTERFACE_HPP_2016_05_03__