#pragma once
#include "log.h"
#include "singleton.h"
#include "util.h"
	
#define FL_LOG_LEVEL(logger,level) \
	if(logger->getLevel() <= level)\
		FL::LogWrap(logger,FL::LogEvent::ptr(\
			new FL::LogEvent(level, __FILE__,__func__,__LINE__ ,0\
				,FL::UT::GetThreadId(), FL::UT::GetCoroutineId(), time(0),FL::UT::GetThreadName(),logger->getName()))\
				).getStrIO() 

#define FL_LOG_DEBUG(logger) FL_LOG_LEVEL(logger,FL::LogLevel::Level::DEBUG)
#define FL_LOG_INFO(logger)  FL_LOG_LEVEL(logger,FL::LogLevel::Level::INFO)
#define FL_LOG_WARN(logger)  FL_LOG_LEVEL(logger,FL::LogLevel::Level::WARN)
#define FL_LOG_ERROR(logger) FL_LOG_LEVEL(logger,FL::LogLevel::Level::ERROR)
#define FL_LOG_FATAL(logger) FL_LOG_LEVEL(logger,FL::LogLevel::Level::FATAL)

#define FL_LOG_ROOT() FL::LogManager::GetInstance()->getRoot()
#define FL_LOG_NAME(name) FL::LogManager::GetInstance()->getLogger(name)
#define FL_SYS_LOG() FL::LogManager::GetInstance()->getLogger("system")

namespace FL {

/**
 * @brief 日志管理器
 */
class LoggerManager {
  public:
    typedef Spinlock Mutex_t;

    /**
     * @brief 日志管理器构造函数
     */
    LoggerManager();

    /**
     * @brief 获取日志器
     *
     * @param[in] name 日志器名
     *
     * @return 日志器
     */
    Logger::ptr getLogger(const std::string& name);

    /**
     * @brief 获取主日志器
     *
     * @return 主日志器
     */
    Logger::ptr getRoot() const {
        return m_root;
    }


    /**
     * @brief 获取所有日志器的配置
     *
     * @return 配置文本
     */
    std::string configString();

  private:
    Logger::ptr m_root;								// 主日志器
    std::map<std::string, Logger::ptr> m_loggers;   // 所有日志器
    Mutex_t		m_mutex;							// 互斥量
};



typedef SingletonPtr<LoggerManager> LogManager;


}
