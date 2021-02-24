#include "log.h"
#include <functional>

namespace FL {
const std::string LogLevel::toString(Level level) {
    switch(level) {
#define XX(lv)		\
		case Level::lv:	\
        return #lv; \
        break;

        XX(UNKNOW);
        XX(DEBUG);
        XX(INFO);
        XX(WARN);
        XX(ERROR);
        XX(FATAL);
#undef XX
    }
    return nullptr;
}

const LogLevel::Level LogLevel::fromString(const std::string& level) {
#define XX(lv, str) \
    if(level == #str) { \
        return LogLevel::Level::lv; \
    }

    XX(DEBUG, debug);
    XX(INFO, info);
    XX(WARN, warn);
    XX(ERROR, error);
    XX(FATAL, fatal);

    XX(DEBUG, DEBUG	);
    XX(INFO, INFO);
    XX(WARN, WARN);
    XX(ERROR, ERROR);
    XX(FATAL, FATAL);
#undef XX
    return LogLevel::Level::UNKNOW;
}

LogEvent::LogEvent(LogLevel::Level level, const char* file, const char* funcName
                   , uint32_t line, uint32_t elapse, uint32_t threadId, uint32_t coroutineId
                   , uint64_t time, const std::string& threadNmae, const std::string& name)
    : m_file(file)
    , m_funcName(funcName)
    , m_line(line)
    , m_elapse(elapse)
    , m_threadId(threadId)
    , m_coroutineId(coroutineId)
    , m_time(time)
    , m_threadName(threadNmae)
    , m_name(name)
    , m_level(level) {}


LogFormatter::LogFormatter(const std::string& pattern)
    : m_pattern(pattern) {
    parse();
}

std::string LogFormatter::format(LogEvent::ptr event) {
    std::stringstream ss;
    for(auto& item : m_items) {
        item->format(ss, event);
    }
    return ss.str();
}

std::ostream& LogFormatter::format(std::ostream& ofs, LogEvent::ptr event) {
    for(auto& item : m_items) {
        item->format(ofs, event);
    }
    return ofs;
}

class MessageFormatItem : implement LogFormatter::FormatItem {
  public:
    MessageFormatItem(const std::string&) {}
    void format(std::ostream& os
                ,  LogEvent::ptr event) override {
        os << event->getContent();
    }
};
class LineFormatItem : implement LogFormatter::FormatItem {
  public:
    LineFormatItem(const std::string&) {}
    void format(std::ostream& os
                ,  LogEvent::ptr event) override {
        os << event->getLine();
    }
};
class LevelFormatItem : implement LogFormatter::FormatItem {
  public:
    LevelFormatItem(const std::string&) {}
    void format(std::ostream& os
                ,  LogEvent::ptr event) override {
        os << LogLevel::toString(event->getLevel());
    }
};
class ElapseFormatItem : implement LogFormatter::FormatItem {
  public:
    ElapseFormatItem(const std::string&) {}
    void format(std::ostream& os
                ,  LogEvent::ptr event) override {
        os << event->getElapse();
    }
};
class NameFormatItem : implement LogFormatter::FormatItem {
  public:
    NameFormatItem(const std::string&) {}
    void format(std::ostream& os
                ,  LogEvent::ptr event) override {
        os << event->getName();
    }
};
class ThreadIdFormatItem : implement LogFormatter::FormatItem {
  public:
    ThreadIdFormatItem(const std::string&) {}
    void format(std::ostream& os
                ,  LogEvent::ptr event) override {
        os << event->getThreadId();
    }
};

class FiberIdFormatItem : implement LogFormatter::FormatItem {
  public:
    FiberIdFormatItem(const std::string& str = "") {}
    ~FiberIdFormatItem() {}
    void format(std::ostream& os
                , LogEvent::ptr event) override {
        os << event->getCoroutineId();
    }
};
class DateTimeFormatItem : implement LogFormatter::FormatItem {
  public:
    DateTimeFormatItem(const std::string& format = "%Y:%m:%d %H:%M:%s"): m_format(format) {
        if(m_format.empty())
            m_format = "%Y:%m:%d %H:%M:%s";
    }
    ~DateTimeFormatItem() {}
    void format(std::ostream& os,
                LogEvent::ptr event) override {
        struct tm tm;
        time_t time = event->getTime();
        localtime_r(&time, &tm);
        char buf[64];
        strftime(buf, sizeof(buf), m_format.c_str(), &tm);
        os << buf;
    }
  private:
    std::string m_format;
};

class FileNameFormatItem : implement LogFormatter::FormatItem {
  public:
    FileNameFormatItem(const std::string& str = "") {}
    ~FileNameFormatItem() {}
    void format(std::ostream& os
                , LogEvent::ptr event) override {
        os << event->getFile();
    }
};

class ThreadNameFormatItem : implement LogFormatter::FormatItem {
  public:
    ThreadNameFormatItem(const std::string& str = "") {}
    ~ThreadNameFormatItem() {}
    void format(std::ostream& os
                , LogEvent::ptr event) override {
        os << event->getThreadName();
    }
};

class NewLineFormatItem : implement LogFormatter::FormatItem {
  public:
    NewLineFormatItem(const std::string& str = "") {}
    ~NewLineFormatItem() {}
    void format(std::ostream& os
                , LogEvent::ptr event) override {
        os << "\n";
    }
};

class TabFormatItem : implement LogFormatter::FormatItem {
  public:
    TabFormatItem(const std::string& str = "") {}
    ~TabFormatItem() {}
    void format(std::ostream& os
                , LogEvent::ptr event) override {
        os << "\t";
    }
};

class StringFormatItem : implement LogFormatter::FormatItem {
  public:
    StringFormatItem(const std::string& str)
        : m_str(str) {}
    void format(std::ostream& os
                , LogEvent::ptr event)override {
        os << m_str;
    }
  private:
    std::string m_str;
};

class FuncNameFormatItem : implement LogFormatter::FormatItem {
  public:
    FuncNameFormatItem(const std::string& str) {}
    void format(std::ostream& os
                , LogEvent::ptr event) override {
        os << event->getFuncName();
    }
};


void LogFormatter::parse() {

    static std::unordered_map<std::string, std::function<LogFormatter::FormatItem::ptr(const std::string& str)> > s_format_items = {
#define XX(des,type) \
	{#des,[](const std::string& str){return LogFormatter::FormatItem::ptr(new type(str));}}
        XX(m, MessageFormatItem),           //m:消息
        XX(p, LevelFormatItem),             //p:日志级别
        XX(r, ElapseFormatItem),            //r:累计毫秒数
        XX(c, NameFormatItem),              //c:日志名称
        XX(t, ThreadIdFormatItem),          //t:线程id
        XX(n, NewLineFormatItem),           //n:换行
        XX(d, DateTimeFormatItem),          //d:时间
        XX(f, FileNameFormatItem),          //f:文件名
        XX(l, LineFormatItem),              //l:行号
        XX(T, TabFormatItem),               //T:Tab
        XX(F, FiberIdFormatItem),           //F:协程id
        XX(N, ThreadNameFormatItem),        //N:线程名称
        XX(w, FuncNameFormatItem)          //w:函数名
#undef XX
    };

    if(m_pattern.empty()) {
        m_error = true;
        std::cerr << "[ERROR]" << "format pattern is empty" << std::endl;
        return;
    }
    enum class TYPE {
        FLAG,
        ITEM,
        REGEX,
        PENDING
    };
    TYPE type;
    char sym = '\0';
    int start = -1;
    for(int i = 0 ; i < m_pattern.size() ; ++i) {
        char ch = m_pattern[i];
        if(ch == '%' && type != TYPE::REGEX) {
            if(i + 1 < m_pattern.size()) {
                if(m_pattern[i + 1] == '%') {
                    type = TYPE::FLAG;
                    continue;
                } else {
                    type = TYPE::ITEM;
                    continue;
                }
            }
            if(type == TYPE::FLAG)
                m_items.push_back(FormatItem::ptr(new StringFormatItem("%")));
        } else if(std::isalpha(ch) && type == TYPE::ITEM) {
            if(i + 1 < m_pattern.size()) {
                if(m_pattern[i + 1] == '{') {
                    type = TYPE::PENDING;
                    sym = ch;
                    continue;
                }
            }
            std::string key(1, ch);
            auto it = s_format_items.find(key);
            if(it != s_format_items.end())
                m_items.push_back(it->second(""));
        } else {
            if(ch == '{')
                if(i + 1 < m_pattern.size()) {
                    if(m_pattern[i + 1] == '%') {
                        type = TYPE::REGEX;
                        start = i;
                        continue;
                    } else if(sym != '\0') {
                        std::string key(1, sym);
                        auto it = s_format_items.find(key);
                        if(it != s_format_items.end())
                            m_items.push_back(it->second(""));
                        m_items.push_back(FormatItem::ptr(new StringFormatItem("{")));
                    }
                }
            if(ch == '}' && type == TYPE::REGEX && sym != '\0') {
                std::string key(1, sym);
                auto it = s_format_items.find(key);
                if(it != s_format_items.end() && start) {
                    std::string tmp = m_pattern.substr(start + 1, i - start - 1);
                    if(tmp.empty()) {
                        m_error = true;
                        std::cerr << "[ERROR]" << "format not valid" << std::endl;
                        return;
                    }
                    m_items.push_back(it->second(tmp));
                }
                sym = '\0';
                start = -1;
                type = TYPE::PENDING;
                continue;
            }
            if(type != TYPE::REGEX) {
                std::string tmp(1, ch);
                m_items.push_back(FormatItem::ptr(new StringFormatItem(tmp)));
            }
        }
    }
}

void StdOutLogAppender::log(LogLevel::Level level, LogEvent::ptr event) {
    if(level >= m_level) {
        Mutex_t::Lock lock(m_mutex);
        if(!m_formatter->format(std::cout, event)) {
            std::cerr << "[ERROR]" << "format error" << std::endl;
        }
    }
}

std::string StdOutLogAppender::configString() {
    Mutex_t::Lock lock(m_mutex);
    std::stringstream ss;
    YAML::Node node;
    node["type"] = "StdOutLogAppender";
    if(m_level != LogLevel::Level::UNKNOW)
        node["level"] = LogLevel::toString(m_level);
    if(hasFromatter())
        node["formatter"] = m_formatter->getPattern();

    ss << node;
    return ss.str();
}

FileLogAppender::FileLogAppender(const std::string & filename)
    : m_filename(filename) {
    reopen();
}

void FileLogAppender::log(LogLevel::Level level, LogEvent::ptr event) {
    if(level >= m_level) {
        Mutex_t::Lock lock(m_mutex);
        if(!m_formatter->format(m_filestream, event)) {
            std::cerr << "[ERROR]" << "format error" << std::endl;
        }
    }
}

bool FileLogAppender::reopen() {
    Mutex_t::Lock lock(m_mutex);
    if(m_filestream) {
        m_filestream.close();
    }
    m_filestream.open(m_filename, std::ios::app);
    return !!m_filestream;
}

std::string FileLogAppender::configString() {
    Mutex_t::Lock lock(m_mutex);
    std::stringstream ss;
    YAML::Node node;
    node["type"] = "FileLogAppender";
    node["file"] = m_filename;
    if(m_level != LogLevel::Level::UNKNOW)
        node["level"] = LogLevel::toString(m_level);
    if(hasFromatter())
        node["formatter"] = m_formatter->getPattern();

    ss << node;
    return ss.str();
}

Logger::Logger(std::string name)
    : m_name(name)
    , m_level(LogLevel::Level()) {
    m_formatter.reset(new LogFormatter("[%d{%Y-%m-%d %H:%M:%S}]%T%t%T%N%T%F%T[%p]%T[%c]%T[%f:%l:%w]%T%m%n"));
}

void Logger::log(LogLevel::Level level, LogEvent::ptr event) {
    if(level >= m_level) {
        Mutex_t::Lock lock(m_mutex);
        if(!m_appenders.empty())
            for(auto& appender : m_appenders)
                appender->log(level, event);
        else if(m_root != nullptr)
            m_root->log(level, event);
        else {
            std::cerr << "[FATAL]" << "\t"
                      << "Runtime Error : log appender is empty" << "\t"
                      << __FILE__ << "\t"
                      << __LINE__ << "\n";
        }
    }
}

void Logger::debug(LogEvent::ptr event) {
    log(LogLevel::Level::DEBUG, event);
}

void Logger::info(LogEvent::ptr event) {
    log(LogLevel::Level::INFO, event);
}

void Logger::warn(LogEvent::ptr event) {
    log(LogLevel::Level::WARN, event);
}

void Logger::error(LogEvent::ptr event) {
    log(LogLevel::Level::ERROR, event);
}

void Logger::fatal(LogEvent::ptr event) {
    log(LogLevel::Level::FATAL, event);
}

void Logger::addAppender(LogAppender::ptr appender) {
    Mutex_t::Lock lock(m_mutex);
    if(!appender->getFormatter()) {
        appender->setFormatter(m_formatter);
    }
    m_appenders.push_front(appender);
}

void Logger::delAppender(LogAppender::ptr appender) {
    Mutex_t::Lock lock(m_mutex);
    m_appenders.remove(appender);
}

void Logger::clearAppenders() {
    m_appenders.clear();
}

void Logger::setFormatter(LogFormatter::ptr formatter) {
    Mutex_t::Lock lock(m_mutex);
    m_formatter = formatter;

    for(auto& appender : m_appenders) {
        Mutex_t::Lock lock(m_mutex);
        m_formatter = formatter;
        if(!appender->hasFromatter()) {
            appender->setFormatter(formatter);
        }
    }
}

void Logger::setFormatter(const std::string & foramtter) {
    LogFormatter::ptr new_formatter = std::make_shared<LogFormatter>(foramtter);
    if(new_formatter->isError()) {
        std::cerr << "Logger setFormatter name=" << m_name
                  << " value=" << foramtter << " invalid formatter"
                  << std::endl;
        return;
    }
    setFormatter(new_formatter);
}

LogFormatter::ptr Logger::getFormatter() {
    Mutex_t::Lock lock(m_mutex);
    return m_formatter;
}

std::string Logger::configString() {
    Mutex_t::Lock lock(m_mutex);
    std::stringstream ss;
    YAML::Node node;
    node["name"] = m_name;
    if(m_level != LogLevel::Level::UNKNOW)
        node["level"] = LogLevel::toString(m_level);
    node["formatter"] = m_formatter->getPattern();

    for(auto appender : m_appenders)
        node["appenders"].push_back(YAML::Load(appender->configString()));

    ss << node;
    return ss.str();
}

}
