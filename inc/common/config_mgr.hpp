#ifndef __CONFIG_MGR_HPP_2016_05_16__
#define __CONFIG_MGR_HPP_2016_05_16__

#include <map>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace mpsp{

class config_mgr
{
private:
    static config_mgr config_mgr_;
    boost::property_tree::ptree pt_;

protected:
    config_mgr();

public:
    ~config_mgr();

    static config_mgr& get(){ return config_mgr_; }

    bool init();

    std::string get_value(const std::string& type, const std::string& key);
};

}
#endif //   __CONFIG_MGR_HPP_2016_05_16__