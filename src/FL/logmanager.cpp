#include "logmanager.h"
#include "config.h"
#include "log.h"

namespace FL {

/**
 * @brief 日志输出地定义
 */
struct LogAppenderDefine {

    int type = 0;
    LogLevel::Level level = LogLevel::Level::UNKNOW;
    std::string formatter;
    std::string file;

    void setType(const std::string& str) {
        if(str == "FileLogAppender")
            type = 1;
        else if(str == "StdOutLogAppender")
            type = 2;
    }

    bool levelIsUnkonw() const {
        return level == LogLevel::Level::UNKNOW;
    }

    bool typeIsFile() const {
        return type == 1;
    }

    bool typeIsStdout() const {
        return type == 2;
    }

    bool fmtEmpty() const {
        return formatter.empty();
    }

    bool fileEmpty() const {
        return file.empty();
    }

    bool operator == (const LogAppenderDefine& oth) const {
        return type == oth.type
               && level == oth.level
               && formatter == oth.formatter
               && file == oth.file;
    }
};

/**
 * @brief 日志定义
 */
struct LogDefine {

    std::string name;
    LogLevel::Level level = LogLevel::Level::UNKNOW;
    std::string formatter;
    std::vector<LogAppenderDefine> appenders;

    bool isValid() const {
        return !name.empty();
    }
    bool levelIsUnkonw() const {
        return level == LogLevel::Level::UNKNOW;
    }

    bool fmtEmpty() const {
        return formatter.empty();
    }

    bool appendEmpty() const {
        return appenders.empty();
    }

    bool operator == (const LogDefine& oth) const {
        return name == oth.name
               && level == oth.level
               && formatter == oth.formatter
               && appenders == appenders;
    }

    bool operator < (const LogDefine& oth) const {
        return name < oth.name;
    }

};


/**
 * @brief 从string转化为logdefine的类型转换类的偏特化
 */
template <>
class LexicalCast<std::string, LogDefine> {
  public:
    LogDefine operator()(const std::string& val) {

        YAML::Node node = YAML::Load(val);

        LogDefine  logdef;

        if(!node["name"].IsDefined()) {
            FL_LOG_ERROR(FL_SYS_LOG()) <<  "log config error: name is empty, " << node;
            throw std::logic_error("log config name is empty");
        }

        logdef.name = node["name"].as<std::string>();
        logdef.level = LogLevel::fromString(node["level"].IsDefined() ? node["level"].as<std::string>() : "");
        if(node["formatter"].IsDefined())
            logdef.formatter = node["formatter"].as<std::string>();


        if(node["appenders"].IsDefined()) {
            for(const auto& it : node["appenders"]) {
                if(!it["type"].IsDefined()) {
                    FL_LOG_ERROR(FL_SYS_LOG()) << "log config error: appender type is epmty, " << it;
                    continue;
                }
                std::string type = it["type"].as<std::string>();

                LogAppenderDefine lad;

                lad.setType(type);

                if(lad.type == 0) {
                    FL_LOG_ERROR(FL_SYS_LOG()) << "log config error: appender type is invalid, " << it;
                    continue;
                }

                if(lad.typeIsFile()) {
                    if(!it["file"].IsDefined()) {
                        FL_LOG_ERROR(FL_SYS_LOG()) << "log config error: fileappender file is empty, " << it;
                        continue;
                    }
                    lad.file = it["file"].as<std::string>();
                }

                if(it["formatter"].IsDefined())
                    lad.formatter = it["formatter"].as<std::string>();

                logdef.appenders.push_back(lad);
            }
        }
        return logdef;
    }
};

/**
 * @brief 从logdefine转化为string的类型转换类的偏特化
 */
template <>
class LexicalCast<LogDefine, std::string> {
  public:
    std::string operator()(const LogDefine& logdef) {

        YAML::Node node;
        node["name"] = logdef.name;

        if(!logdef.levelIsUnkonw())
            node["level"] = LogLevel::toString(logdef.level);
        if(!logdef.fmtEmpty())
            node["formatter"] = logdef.formatter;

        for(auto& lad : logdef.appenders) {
            YAML::Node node_a;
            if(lad.typeIsFile()) {
                node_a["type"] = "FileLogAppender";
                node_a["file"] = lad.file;
            } else if(lad.typeIsStdout()) {
                node_a["type"] = "StdOutLogAppender";
            }

            if(!lad.levelIsUnkonw())
                node_a["level"] = LogLevel::toString(lad.level);
            if(!lad.fmtEmpty())
                node_a["formatter"] = lad.formatter;

            node["appenders"].push_back(node_a);
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

ConfigVar<std::set<LogDefine> >::ptr log_defines  = Config::Lookup("logs", std::set<LogDefine>(), "log config");

struct LogIniter {
    LogIniter() {
        FL::LogAppender::ptr appender(new FileLogAppender("system.log"));
        appender->setLevel(LogLevel::Level::ERROR);
        FL_SYS_LOG()->addAppender(appender);
        FL_SYS_LOG()->addAppender(FL::StdOutLogAppender::ptr(new StdOutLogAppender));


        log_defines->addListener([](const std::set<LogDefine>& old_val
        , const std::set<LogDefine>& new_val) {
            FL_LOG_INFO(FL_LOG_ROOT()) << "log config changed";
            for(auto& logdef : new_val) {
                auto it = old_val.find(logdef);
                Logger::ptr logger;
                if(it == old_val.end())
                    logger = FL_LOG_NAME(logdef.name);
                else {
                    if(!(logdef == *it))
                        logger = FL_LOG_NAME(logdef.name);
                    else
                        continue;
                }

                logger->setLevel(logdef.level);

                if(!logdef.fmtEmpty())
                    logger->setFormatter(logdef.formatter);

                logger->clearAppenders();
                for(auto& lad : logdef.appenders) {
                    LogAppender::ptr appender;
                    if(lad.typeIsFile()) {
                        if(!lad.fileEmpty())
                            appender = std::make_shared<FileLogAppender>(lad.file);
                        else
                            FL_LOG_ERROR(FL_SYS_LOG()) << "log config error: fileappener file is empty";
                    } else if(lad.typeIsStdout()) {
                        appender = std::make_shared<StdOutLogAppender>();
                    }
                    if(!lad.levelIsUnkonw()) {
                        appender->setLevel(lad.level);
                    }

                    if(!lad.fmtEmpty()) {
                        LogFormatter::ptr fmt = std::make_shared<LogFormatter>(lad.formatter);
                        if(!fmt->isError())
                            appender->setFormatter(fmt);
                        else
                            FL_LOG_ERROR(FL_SYS_LOG()) << "log.name=" << logdef.name
                                                       << " appender type=" << lad.type
                                                       << " formatter=" << lad.formatter << " is invalid";
                    }
                    logger->addAppender(appender);
                }
            }

            for(auto& logdef : old_val) {
                auto it = new_val.find(logdef);
                if(it == new_val.end()) {
                    auto logger = FL_LOG_NAME(logdef.name);
                    logger->setLevel(LogLevel::Level());
                    logger->clearAppenders();
                }
            }

        });
    }
};

static LogIniter _log_init;

LoggerManager::LoggerManager() {
    m_root.reset(new Logger);
    m_root->addAppender(std::make_shared<StdOutLogAppender>());
    m_loggers[m_root->getName()] = m_root;
}

Logger::ptr LoggerManager::getLogger(const std::string &name) {
    Mutex_t::Lock lock(m_mutex);
    auto it = m_loggers.find(name);
    if(it != m_loggers.end())
        return it->second;
    Logger::ptr logger = std::make_shared<Logger>(name);
    m_loggers[name] = logger;
    return logger;
}

std::string LoggerManager::configString() {
    Mutex_t::Lock lock(m_mutex);
    std::stringstream ss;
    YAML::Node node;
    for(const auto& it : m_loggers)
        node.push_back(YAML::Load(it.second->configString()));
    ss << node;
    return ss.str();
}

}
