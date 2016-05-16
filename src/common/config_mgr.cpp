#include "config_mgr.hpp"
#include <boost/filesystem/path.hpp> 
#include <boost/filesystem/operations.hpp> 

namespace mpsp{

config_mgr config_mgr::config_mgr_;

config_mgr::config_mgr()
{

}

config_mgr::~config_mgr()
{

}

bool config_mgr::init()
{
    try
    {
        std::string path = boost::filesystem::initial_path<boost::filesystem::path>().string() + "/config/config.json";
        boost::property_tree::read_json<boost::property_tree::ptree>(path, pt_);
    }
    catch (boost::property_tree::json_parser_error&)
    {
        printf("config parse error!\n");
        return false;
    }

    return true;
}

std::string config_mgr::get_value(const std::string& type, const std::string& key)
{
    if (pt_.find(type) == pt_.not_found())
    {
        return "";
    }

    boost::property_tree::ptree pt1;
    pt1 = pt_.get_child(type);
    if (pt1.find(key) == pt1.not_found())
    {
        return "";
    }

    return pt1.get<std::string>(key);
}

}