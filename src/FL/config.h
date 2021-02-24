#pragma once

#include "mutex.h"
#include "logmanager.h"

#include <functional>
#include <algorithm>
#include <memory>
#include <regex>
#include <stdexcept>
#include <string>
#include <sstream>
#include <unordered_set>
#include <unordered_map>

#include <boost/lexical_cast.hpp>
#include <yaml-cpp/yaml.h>

namespace FL {

/**
 * @brief 类型转换
 *
 * @tparam F 源类型
 * @tparam T 目的类型
 */
template <class F, class T>
class LexicalCast {
  public:
    T operator()(const F& val) {
        return boost::lexical_cast<T>(val);
    }
};

/**
 * @brief 模板偏特化,从string转换为vector
 *
 * @tparam T 任意类型
 */
template <class T>
class LexicalCast<std::string, std::vector<T> > {
  public:

    /**
     * @brief 重载()运算符
     *
     * @param[in] val 需要转换的文本
     *
     * @return 目的类型(vector)
     */
    std::vector<T> operator()(const std::string& val) {

        typename std::vector<T> vec;

        YAML::Node node = YAML::Load(val);
        std::stringstream ss;

        for(auto it : node) {
            ss.str("");
            ss << *it;
            vec.push_back(LexicalCast<std::string, T>()(ss.str()));
        }

        return vec;
    }
  private:
};

/**
 * @brief 模板偏特化,从vector转换为string
 *
 * @tparam T 任意类型
 */
template<class T>
class LexicalCast<std::vector<T>, std::string> {
  public:
    /**
     * @brief 重载()运算符
     *
     * @param[in] val 需要转换的vector
     *
     * @return 目的类型(string)
     */
    std::string operator()(const std::vector<T>& val) {

        YAML::Node node(YAML::NodeType::Sequence);

        for(auto& it : val) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(it)));
        }

        std::stringstream ss;
        ss << node;

        return ss.str();
    }
};

/**
 * @brief 模板偏特化,从string转换为list
 *
 * @tparam T 任意类型
 */
template<class T>
class LexicalCast<std::string, std::list<T> > {
  public:
    /**
     * @brief 重载()运算符
     *
     * @param[in] val 需要转换的文本
     *
     * @return 目的类型(list)
     */
    std::list<T> operator()(const std::string& val) {

        typename std::list<T> list;

        YAML::Node node = YAML::Load(val);
        std::stringstream ss;

        for(size_t i = 0; i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            list.push_back(LexicalCast<std::string, T>()(ss.str()));
        }

        return list;
    }
};

/**
 * @brief 模板偏特化,从list转换为string
 *
 * @tparam T 任意类型
 */
template<class T>
class LexicalCast<std::list<T>, std::string> {
  public:
    /**
     * @brief 重载()运算符
     *
     * @param[in] val 需要转换的list
     *
     * @return 目的类型(string)
     */
    std::string operator()(const std::list<T>& val) {

        YAML::Node node(YAML::NodeType::Sequence);

        for(auto& it : val) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(it)));
        }

        std::stringstream ss;
        ss << node;

        return ss.str();
    }
};

/**
 * @brief 模板偏特化,从string转换为set
 *
 * @tparam T 任意类型
 */
template<class T>
class LexicalCast<std::string, std::set<T> > {
  public:
    /**
     * @brief 重载()运算符
     *
     * @param[in] val 需要转换的文本
     *
     * @return 目的类型()
     */
    std::set<T> operator()(const std::string& val) {

        typename std::set<T> map;

        YAML::Node node = YAML::Load(val);
        std::stringstream ss;

        for(size_t i = 0; i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            map.insert(LexicalCast<std::string, T>()(ss.str()));
        }

        return map;
    }
};

/**
 * @brief 模板偏特化,从set转换为string
 *
 * @tparam T 任意类型
 */
template<class T>
class LexicalCast<std::set<T>, std::string> {
  public:
    /**
     * @brief 重载()运算符
     *
     * @param[in] val 需要转换的set
     *
     * @return 目的类型(string)
     */
    std::string operator()(const std::set<T>& val) {

        YAML::Node node(YAML::NodeType::Sequence);

        for(auto& it : val) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(it)));
        }

        std::stringstream ss;
        ss << node;

        return ss.str();
    }
};

/**
 * @brief 模板偏特化,从string转换为set
 *
 * @tparam T 任意类型
 */
template<class T>
class LexicalCast<std::string, std::unordered_set<T> > {
  public:
    /**
     * @brief 重载()运算符
     *
     * @param[in] val 需要转换的文本
     *
     * @return 目的类型(unordered_set)
     */
    std::unordered_set<T> operator()(const std::string& val) {

        typename std::unordered_set<T> uset;

        YAML::Node node = YAML::Load(val);
        std::stringstream ss;

        for(auto it : node) {
            ss.str("");
            ss << it;
            uset.insert(LexicalCast<std::string, T>()(ss.str()));
        }

        return uset;
    }
};

/**
 * @brief 模板偏特化,从unordered_set转换为string
 *
 * @tparam T 任意类型
 */
template<class T>
class LexicalCast<std::unordered_set<T>, std::string> {
  public:
    /**
     * @brief 重载()运算符
     *
     * @param[in] val 需要转换的set
     *
     * @return 目的类型(string)
     */
    std::string operator()(const std::unordered_set<T>& val) {

        YAML::Node node(YAML::NodeType::Sequence);

        for(auto& it : val) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(it)));
        }

        std::stringstream ss;
        ss << node;

        return ss.str();
    }
};

/**
 * @brief 模板偏特化,从string转换为map
 *
 * @tparam T 任意类型
 */
template<class T>
class LexicalCast<std::string, std::map<std::string, T> > {
  public:
    /**
     * @brief 重载()运算符
     *
     * @param[in] val 需要转换的文本
     *
     * @return 目的类型(map)
     */
    std::map<std::string, T> operator()(const std::string& val) {

        typename std::map<std::string, T> map;

        YAML::Node node = YAML::Load(val);
        std::stringstream ss;

        for(auto it : node) {
            ss.str("");
            ss << it.second;
            map.insert(std::make_pair(it.first.Scalar(),
                                      LexicalCast<std::string, T>()(ss.str())));
        }

        return map;
    }
};

/**
 * @brief 模板偏特化,从string转换为vector
 *
 * @tparam T 任意类型
 */
template<class T>
class LexicalCast<std::map<std::string, T>, std::string> {
  public:
    /**
     * @brief 重载()运算符
     *
     * @param[in] val 需要转换的map
     *
     * @return 目的类型(string)
     */
    std::string operator()(const std::map<std::string, T>& val) {

        YAML::Node node(YAML::NodeType::Map);

        for(auto& it : val) {
            node[it.first] = YAML::Load(LexicalCast<T, std::string>()(it.second));
        }

        std::stringstream ss;
        ss << node;

        return ss.str();
    }
};

/**
 * @brief 模板偏特化,从string转换为unordered_map
 *
 * @tparam T 任意类型
 */
template<class T>
class LexicalCast<std::string, std::unordered_map<std::string, T> > {
  public:
    /**
     * @brief 重载()运算符
     *
     * @param[in] val 需要转换的文本
     *
     * @return 目的类型(unordered_map)
     */
    std::unordered_map<std::string, T> operator()(const std::string& val) {

        typename std::unordered_map<std::string, T> umap;

        YAML::Node node = YAML::Load(val);
        std::stringstream ss;

        for(auto it : node) {
            ss.str("");
            ss << it.second;
            umap.insert(std::make_pair(it.first.Scalar(),
                                       LexicalCast<std::string, T>()(ss.str())));
        }

        return umap;
    }
};

/**
 * @brief 模板偏特化,从unordered_map转换为string
 *
 * @tparam T 任意类型
 */
template<class T>
class LexicalCast<std::unordered_map<std::string, T>, std::string> {
  public:
    /**
     * @brief 重载()运算符
     *
     * @param[in] val 需要转换的unordered_map
     *
     * @return 目的类型(string)
     */
    std::string operator()(const std::unordered_map<std::string, T>& val) {

        YAML::Node node(YAML::NodeType::Map);

        for(auto& it : val) {
            node[it.first] = YAML::Load(LexicalCast<T, std::string>()(it.second));
        }

        std::stringstream ss;
        ss << node;

        return ss.str();
    }
};

/**
 * @brief 配置值类,记录每一项配置
 */
class ConfigVarBase {
  public:
    typedef std::shared_ptr<ConfigVarBase> ptr;

    /**
     * @brief 构造函数,生成配置名称及其描述
     *
     * @param[in] name 名称
     * @param[in] description 描述
     */
    ConfigVarBase(const std::string& name, const std::string& description = "")
        : m_name(name)
        , m_description(description) {

        std::transform(m_name.begin(), m_name.end(), m_name.begin(), tolower);
    }

    /**
     * @brief 析构函数
     */
    virtual ~ConfigVarBase() {}

    /**
     * @brief 获取配置的名
     *
     * @return 配置名
     */
    const std::string& getName() const {
        return m_name;
    }

    /**
     * @brief 获取配置的描述
     *
     * @return 配置的描述
     */
    const std::string& getDescription() const {
        return m_description;
    }

    /**
     * @brief 把配置内容转换为对应文本
     *
     * @return 文本
     */
    virtual std::string toString() = 0;

    /**
     * @brief 从文本生成对应配置
     *
     * @param[in] str 配置文本
     *
     * @return 生成是否成功
     */
    virtual bool fromString(const std::string& str) = 0;

    /**
     * @brief 获取配置值的类型名
     *
     * @return 类型名
     */
    virtual std::string getTypeName() const = 0;

  protected:

    std::string m_name;			//配置名称
    std::string m_description;	//描述
};


/**
 * @brief 配置值
 *
 * @tparam T 值类型
 * @tparam FromStr 转换类
 * @tparam T 任意类型
 */
template< class T, class FromStr = LexicalCast<std::string, T>
          , class ToStr = LexicalCast<T, std::string>>
class ConfigVar : public ConfigVarBase {
  public:
    typedef std::shared_ptr<ConfigVar> ptr;
    typedef std::function<void(const T& old_value, const T& new_value)> Callback_t;
    typedef std::map<uint64_t, Callback_t> Map_t;
    typedef RWMutex Mutex_t;

    /**
     * @brief 构造函数,生成配置
     *
     * @param[in] name 配置名称
     * @param[in] default_val 值
     * @param[in] description 描述
     */
    ConfigVar(const std::string& name
              , const T& default_val
              , const std::string& description = "")
        : ConfigVarBase(name, description)
        , m_val(default_val) {}

    /**
     * @brief 将对应的配置项细节转换为文本
     *
     * @return 配置文本
     */
    std::string toString()override {
        try {
            return ToStr()(getVal());
        } catch(std::exception& e) {
            FL_LOG_ERROR(FL_SYS_LOG()) << "ConfigVa:toString exception "
                                       << e.what() << " convert: " << typeid(m_val).name() << "to string";
        }
        return "";
    }

    /**
     * @brief 从文本生成对应配置
     *
     * @param[in] str 配置文本
     *
     * @return
     */
    bool fromString(const std::string& str) override {
        try {
            setVal(FromStr()(str));
        } catch(std::exception& e) {
            FL_LOG_ERROR(FL_SYS_LOG()) << "ConfigVar:toString exception "
                                       << e.what() << " convert: string to" << typeid(m_val).name();
        }
        return false;
    }

    /**
     * @brief 获取配置项的值
     *
     * @return 配置项的值
     */
    const T& getVal() {
        Mutex_t::ReadLock lock(m_mutex);
        return m_val;
    }

    /**
     * @brief 设置配置项的值
     *
     * @param[in] val
     */
    void setVal(const T& val) {
        {
            Mutex_t::ReadLock rlock(m_mutex);
            if(val == m_val)
                return ;
            for(auto it : m_cbs) {
                it.second(m_val, val);
            }
        }
        Mutex_t::WriteLock wlock(m_mutex);
        m_val = val;
    }

    /**
     * @brief 获取配置值的类型名
     *
     * @return 配置值类型名
     */
    std::string getTypeName() const override {
        return typeid(T).name();
    }

    /**
     * @brief 添加监听函数
     *
     * @param[in] callback 监听函数
     *
     * @return 监听函数id
     */
    uint64_t addListener(Callback_t callback) {
        static uint64_t func_id = 0;
        Mutex_t::WriteLock lock(m_mutex);
        ++ func_id;
        m_cbs[func_id] = callback;
        return func_id;
    }

    /**
     * @brief 根据id删除对应的监听函数
     *
     * @param[in] fid 监听函数id
     */
    void delListener(uint64_t fid) {
        Mutex_t::WriteLock lock(m_mutex);
        m_cbs.erase(fid);
    }

    /**
     * @brief 根据id获取对应的监听函数
     *
     * @param[in] fid 监听函数id
     *
     * @return 监听函数
     */
    Callback_t getListener(uint64_t fid) {
        Mutex_t::ReadLock lock(m_mutex);
        auto it = m_cbs.find(fid);
        return it == m_cbs.end() ? nullptr : it->second;
    }

    /**
     * @brief 清楚所有的监听函数
     */
    void clearListener() {
        Mutex_t::WriteLock lock(m_mutex);
        m_cbs.clear();
    }

  private:
    T		 m_val;		// 配置值
    Map_t	 m_cbs;		// 监听函数组
    Mutex_t	 m_mutex;	// 互斥锁
};

/**
 * @brief 配置类,生成配置
 */
class Config {
  public:
    typedef std::map<std::string, ConfigVarBase::ptr> Map_t;
    typedef std::regex Regex_t;
    typedef RWMutex Mutex_t;

    /**
     * @brief 查找配置项,如果没有则生成
     *
     * @tparam T 配置项值的类型
     * @param[in] name 配置项的名称
     * @param[in] default_val 配置项的值
     * @param[in] description 配置项的描述
     *
     * @return 配置项
     */
    template<class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string& name
            , const T& default_val, const std::string& description = "") {

        Mutex_t::WriteLock lock(GetMutex());
        auto it = GetDatas().find(name);

        if(it != GetDatas().end()) {

            auto tmp = std::dynamic_pointer_cast<ConfigVar<T> >(it->second);

            if(tmp) {
                FL_LOG_INFO(FL_SYS_LOG()) << "Lookup name = " <<  name << " exists";
                return tmp;
            } else {
                FL_LOG_ERROR(FL_SYS_LOG()) << __FILE__ << ":" << __LINE__ << "Lookup name = " << name << " exists but type not"
                                           << typeid(T).name() << " real type = " << it->second->getTypeName();
                return nullptr;
            }
        }

        if(!std::regex_match(name, GetRegex())) {
            FL_LOG_ERROR(FL_SYS_LOG()) << "Lookup name invalid " << name;
            throw std::invalid_argument(name);
        }

        typename ConfigVar<T>::ptr cvar(std::make_shared <ConfigVar<T> >(name, default_val, description));
        GetDatas()[name] = cvar;

        return cvar;
    }

    /**
     * @brief 查找配置项,如果没有则返回空
     *
     * @tparam T 配置项值的类型
     * @param[in] name 配置项的名称
     *
     * @return 配置项
     */
    template <class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string & name) {

        Mutex_t::ReadLock lock(GetMutex());
        auto it = GetDatas().find(name);

        if(it == GetDatas().end()) {
            return nullptr;
        }

        return std::dynamic_pointer_cast<ConfigVar<T> >(it->second);
    }

    /**
     * @brief 加载配置文件,并生成对应配置
     *
     * @param[in] root 配置文件
     */
    static void LoadFromYaml(const YAML::Node & root);

    /**
     * @brief 查找配置项
     *
     * @param[in] name 配置项的名称
     *
     * @return 配置项
     */
    static ConfigVarBase::ptr LookupBase(const std::string & name);

    /**
     * @brief 调用回调函数
     *
     * @param[in] callback 回调函数
     */
    static void Visit(std::function<void(ConfigVarBase::ptr)> callback);

    /**
     * @brief 获取命名规则的正则表达式
     *
     * @return 正则表达式
     */
    static Regex_t& GetRegex() {
        static Regex_t pattern("^[0-9a-z._]*$");
        return pattern;
    }
  private:

    /**
     * @brief 获取回调函数组
     *
     * @return 回调函数组
     */
    static Map_t& GetDatas() {
        static Map_t s_datas;
        return s_datas;
    }

    /**
     * @brief 获取互斥锁
     *
     * @return 互斥锁
     */
    static Mutex_t& GetMutex() {
        static Mutex_t s_mutex;
        return s_mutex;
    }
};

}
