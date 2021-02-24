#include "config.h"
#include "logmanager.h"

#include <exception>

namespace FL {

static Logger::ptr syslog = FL_SYS_LOG();

ConfigVarBase::ptr Config::LookupBase(const std::string& name) {
    Mutex_t::ReadLock lock(GetMutex());
    auto it = GetDatas().find(name);
    return it == GetDatas().end() ? nullptr : it->second;
}

void ListAllMember(const std::string& prefix
                   , const YAML::Node& node
                   , std::list<std::pair<std::string, const YAML::Node> > &output) {

    if(!std::regex_match(prefix, Config::GetRegex()) && !prefix.empty()) {
        FL_LOG_ERROR(syslog) << "Config invalid name: " << prefix << " : " << node;
        return;
    }

    if(!prefix.empty()) {
        output.push_back(std::make_pair(prefix, node));
    }

    if(node.IsMap())
        for(auto it : node) {
            ListAllMember((prefix.empty() ? it.first.Scalar() : (prefix + "." + it.first.Scalar()))
                          , it.second
                          , output);
        }
}

void Config::LoadFromYaml(const YAML::Node & root) {

    std::list<std::pair<std::string, const YAML::Node> >  all_nodes;
    ListAllMember("", root, all_nodes);

    for(auto& node : all_nodes) {

        std::string key = node.first;

        ConfigVarBase::ptr var = LookupBase(key);

        std::stringstream ss;
        if(var) {
            if(node.second.IsScalar()) {
                var->fromString(node.second.Scalar());
            } else {
                ss << node.second;
                var->fromString(ss.str());
                ss.str("");
            }
        }
    }

}

void Config::Visit(std::function<void(ConfigVarBase::ptr)> callback) {
    Mutex_t::ReadLock lock(GetMutex());
    Map_t& m = GetDatas();
    for(const auto& it : m) {
        callback(it.second);
    }
}

}
