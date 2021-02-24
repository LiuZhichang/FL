#pragma once

#include <fstream>
#include <memory>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <forward_list>
#include <yaml-cpp/yaml.h>
#include "mutex.h"

namespace FL {

#define implement public

class Logger;

/**
 * @brief 日志等级
 */
class LogLevel {
  public:
    enum class Level {
        UNKNOW	= 0x00,
        DEBUG	= 0x01,
        INFO	= 0x02,
        WARN	= 0x03,
        ERROR	= 0x04,
        FATAL	= 0x05
    };

    /**
     * @brief 将对应的日志等级转换为文本
     *
     * @param[in] level 日志等级
     *
     * @return 对应的日志等级文本
     */
    static const std::string toString(Level level);

    /**
     * @brief 将文本转换为对应的日志等级
     *
     * @param[in] level 日志等级
     *
     * @return 文本对应的日志等级
     */
    static const Level fromString(const std::string& level);
};

/**
 * @brief 日志事件
 */
class LogEvent {
  public:
    typedef std::shared_ptr<LogEvent> ptr;

    /**
     * @brief 日志事件类的构造函数
     *
     * @param[in] level			日志等级
     * @param[in] file			文件名
     * @param[in] funcName		函数名
     * @param[in] line			行号
     * @param[in] elapse		启动到现在的毫秒数
     * @param[in] threadId		线程id
     * @param[in] coroutineId   协程id
     * @param[in] time			当前时间
     * @param[in] threadNmae	线程名
     * @param[in] name			日志器名
     */
    LogEvent(LogLevel::Level level, const char* file, const char* funcName, uint32_t line
             , uint32_t elapse, uint32_t threadId, uint32_t coroutineId
             , uint64_t time, const std::string& threadNmae, const std::string& name);

    /**
     * @brief 获取文件名
     *
     * @return 文件名
     */
    const char* getFile() const {
        return m_file;
    }

    /**
     * @brief 获取函数名
     *
     * @return 函数名
     */
    const char* getFuncName() const {
        return m_funcName;
    }

    /**
     * @brief 获取行号
     *
     * @return 行号
     */
    uint32_t getLine() const {
        return m_line;
    }

    /**
     * @brief 获取启动到现在的毫秒数
     *
     * @return 毫秒数
     */
    uint32_t getElapse() const {
        return m_elapse;
    }

    /**
     * @brief 获取线程id
     *
     * @return 线程id
     */
    uint32_t getThreadId() const {
        return m_threadId;
    }

    /**
     * @brief 获取协程id
     *
     * @return 协程id
     */
    uint32_t getCoroutineId() const {
        return m_coroutineId;
    }

    /**
     * @brief 获取当前时间
     *
     * @return 时间
     */
    uint64_t getTime() const {
        return m_time;
    }

    /**
     * @brief 获取线程名
     *
     * @return 线程名
     */
    const std::string& getThreadName() const {
        return m_threadName;
    }

    /**
     * @brief 获取日志等级
     *
     * @return 日志等级
     */
    LogLevel::Level getLevel() const {
        return m_level;
    }

    /**
     * @brief 获取日志内容
     *
     * @return 日志内容
     */
    const std::string getContent() const {
        return m_ss.str();
    }

    /**
     * @brief 获取字符流
     *
     * @return 字符流
     */
    std::stringstream& getStrIO() {
        return m_ss;
    }

    /**
     * @brief 获取日志器名
     *
     * @return 日志器名
     */
    std::string getName() const {
        return m_name;
    }

  private:
    const char* m_file;			// 文件名
    const char* m_funcName;		// 函数名
    uint32_t   m_line;			// 行号
    uint32_t   m_elapse;		// 毫秒数
    uint32_t   m_threadId;		// 线程id
    uint32_t   m_coroutineId;	// 协程id
    uint64_t   m_time;			// 时间
    std::string m_threadName;	// 线程名
    std::string m_name;			// 日志器名
    LogLevel::Level m_level;	// 日志等级
    std::stringstream m_ss;		// 字符流
};

/**
 * @brief 日志格式器
 */
class LogFormatter {
  public:
    typedef std::shared_ptr<LogFormatter> ptr;

    /**
     * @brief 日志格式项
     */
    class FormatItem {
      public:
        typedef std::shared_ptr<FormatItem> ptr;

        /**
         * @brief 格式化输出日志
         *
         * @param[in] os 输出流
         * @param[in] event 日志事件
         */
        virtual void format(std::ostream& os, LogEvent::ptr event) = 0;
    };

  public:

    /**
     * @brief 日志格式器构造函数
     *
     * @param[in] pattern 日志格式模式串
     */
    LogFormatter(const std::string& pattern);

    /**
     * @brief 格式化输出日志
     *
     * @param[in] event 日志事件
     *
     * @return 日志文本
     */
    std::string format(LogEvent::ptr event);

    /**
     * @brief 格式化输出日志
     *
     * @param[in] ofs 输出流
     * @param[in] event 日志事件
     *
     * @return 输出流
     */
    std::ostream& format(std::ostream& ofs, LogEvent::ptr event);

    /**
     * @brief 模式串解析
     */
    void parse();

    /**
     * @brief 格式串格式是否错误
     *
     * @return 是否发生错误
     */
    bool isError() const {
        return m_error;
    }

    /**
     * @brief 获取模式串
     *
     * @return 模式串
     */
    const std::string getPattern()const {
        return m_pattern;
    }

  private:
    std::string m_pattern;					// 模式串
    std::vector<FormatItem::ptr>  m_items;	// 格式项
    bool m_error;							// 错误
};

/**
 * @brief 日志输出地
 */
class LogAppender {
  public:
    typedef std::shared_ptr<LogAppender> ptr;
    typedef Spinlock Mutex_t;

    /**
     * @brief 生成日志
     *
     * @param[in] level 日志等级
     * @param[in] event 日志事件
     */
    virtual void log(LogLevel::Level level, LogEvent::ptr event) = 0;

    /**
     * @brief 获取日志输地的详细配置
     *
     * @return 配置文本
     */
    virtual std::string configString() = 0;

    /**
     * @brief 设置日志格式器
     *
     * @param[in] formatter 日志格式器
     */
    void setFormatter(LogFormatter::ptr formatter) {
        Mutex_t::Lock lovk(m_mutex);
        m_formatter = formatter;
        m_hasFormatter = m_formatter ? true : false;
    }

    /**
     * @brief 获取日志格式器
     *
     * @return 日志格式器
     */
    LogFormatter::ptr getFormatter() {
        Mutex_t::Lock lock(m_mutex);
        return m_formatter;
    }

    /**
     * @brief 是否有日志格式器
     *
     * @return 是否有格式器
     */
    bool hasFromatter() const {
        return m_hasFormatter;
    }

    /**
     * @brief 设置日志等级
     *
     * @param[in] level 日志等级
     */
    void setLevel(LogLevel::Level level) {
        m_level = level;
    }

    /**
     * @brief 获取日志等级
     *
     * @return 日志等级
     */
    LogLevel::Level getLevel() const {
        return m_level;
    }
  protected:
    LogFormatter::ptr m_formatter;                      // 日志格式器
    LogLevel::Level m_level = LogLevel::Level::UNKNOW;  // 日志等级
    bool m_hasFormatter = false;						// 默认没有日志格式器
    Mutex_t m_mutex;									// 互斥量
};

/**
 * @brief 标准输出流日志输出地
 */
class StdOutLogAppender : implement LogAppender {
  public:
    typedef std::shared_ptr<StdOutLogAppender> ptr;

    /**
     * @brief 生成日志
     *
     * @param[in] level 日志等级
     * @param[in] event 日志事件
     */
    void log(LogLevel::Level level, LogEvent::ptr event) override;

    /**
     * @brief 获取标准输出流日志输出地的配置
     *
     * @return 配置文本
     */
    virtual std::string configString() override;
  private:
};

/**
 * @brief 文件流日志输出地
 */
class FileLogAppender : implement LogAppender {
  public:
    typedef std::shared_ptr<FileLogAppender> ptr;

    /**
     * @brief 文件流日志输出地构造函数
     *
     * @param[in] filename 文件名
     */
    FileLogAppender(const std::string& filename);

    /**
     * @brief 生成日志
     *
     * @param[in] level 日志等级
     * @param[in] event 日志事件
     */
    void log(LogLevel::Level level, LogEvent::ptr event) override;

    /**
     * @brief 获取文件流日志输出地的配置
     *
     * @return 配置文本
     */
    virtual std::string configString() override;

    /**
     * @brief 重新打开文件
     *
     * @return 是否成功
     */
    bool reopen();
  private:
    std::string m_filename;    // 文件名
    std::ofstream m_filestream;// 文件流
    uint64_t m_lastTime = 0;   // 最后打开时间
};

/**
 * @brief 日志器
 */
class Logger {
  public:
    typedef std::shared_ptr<Logger> ptr;
    typedef Spinlock Mutex_t;

    /**
     * @brief 日志器构造函数
     *
     * @param[in] name 日志器名
     */
    Logger(std::string name = "root");

    /**
     * @brief 生成日志
     *
     * @param[in] level 日志等级
     * @param[in] event 日志事件
     */
    void log(LogLevel::Level level, LogEvent::ptr event);

    /**
     * @brief 生成debug等级的日志
     *
     * @param[in] event 日志事件
     */
    void debug(LogEvent::ptr event);

    /**
     * @brief 生成info等级的日志
     *
     * @param[in] event 日志事件
     */
    void info(LogEvent::ptr event);

    /**
     * @brief 生成warn等级的日志
     *
     * @param[in] event 日志事件
     */
    void warn(LogEvent::ptr event);

    /**
     * @brief 生成error等级的日志
     *
     * @param[in] event 日志事件
     */
    void error(LogEvent::ptr event);

    /**
     * @brief 生成fatal等级的日志
     *
     * @param[in] event 日志事件
     */
    void fatal(LogEvent::ptr event);

    /**
     * @brief 添加日志输出地
     *
     * @param[in] appender 日志输出地
     */
    void addAppender(LogAppender::ptr appender);

    /**
     * @brief 删除日志输出地
     *
     * @param[in] appender
     */
    void delAppender(LogAppender::ptr appender);

    /**
     * @brief 清除日志输出地
     */
    void clearAppenders();

    /**
     * @brief 获取日志器配置
     *
     * @return 配置文本
     */
    std::string configString();

    /**
     * @brief 获取日志等级
     *
     * @return 日志等级
     */
    LogLevel::Level getLevel() const {
        return m_level;
    }

    /**
     * @brief 设置日志等级
     *
     * @param[in] val 日志等级
     */
    void setLevel(LogLevel::Level level) {
        m_level = level;
    }

    /**
     * @brief 设置日志格式器
     *
     * @param[in] formatter 日志格式器
     */
    void setFormatter(LogFormatter::ptr formatter);

    /**
     * @brief 设置日志格式器
     *
     * @param[in] foramtter 日志格式模式串
     */
    void setFormatter(const std::string& foramtter);

    /**
     * @brief 设置日志格式器
     *
     * @return 日志格式器
     */
    LogFormatter::ptr getFormatter();

    /**
     * @brief 获取日志器名
     *
     * @return 日志器名
     */
    std::string getName() const {
        return m_name;
    }
  private:
    std::string m_name;								// 日志器名
    LogLevel::Level m_level;						// 日志等级
    std::forward_list<LogAppender::ptr> m_appenders;// 日志输出地
    LogFormatter::ptr m_formatter;					// 日志格式器
    Logger::ptr m_root;								// 主日志器
    Mutex_t m_mutex;								// 互斥量
};

/**
 * @brief 日志外覆器
 */
class LogWrap {
  public:

    /**
     * @brief 外覆器构造函数
     *
     * @param[in] logger 日志器
     * @param[in] event 日志事件
     */
    LogWrap(Logger::ptr logger, LogEvent::ptr event)
        : m_logger(logger), m_event(event) {};

    /**
     * @brief 外覆器析构函数
     */
    ~LogWrap() {
        m_logger->log(m_event->getLevel(), m_event);
    }

    /**
     * @brief 获取日志事件
     *
     * @return 日志事件
     */
    LogEvent::ptr getEvent() const {
        return m_event;
    }

    /**
     * @brief 获取字符流
     *
     * @return 字符流
     */
    std::stringstream& getStrIO() {
        return m_event->getStrIO();
    }

  public:
    Logger::ptr   m_logger;  // 日志器
    LogEvent::ptr m_event;   // 日志事件
};

}
