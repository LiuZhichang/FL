#pragma once

#include <memory>
#include <functional>
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <string>
#include <cstring>
#include "mutex.h"
#include "logmanager.h"
#include "noncopyable.h"

namespace FL {

/**
 * @brief 线程类
 */
//class Thread : NonCopyable {
class Thread : NonCopyable {
  public:
    typedef std::shared_ptr<Thread> ptr;
    typedef std::function<void()> Callback_t;
    /**
     * @brief 构造函数,生成线程
     *
     * @param[in] cb 回调函数
     * @param[in] name 线程名称
     */
    Thread(const std::string&name, std::function<void()> cb);

    //  template <class... Args>
    //  Thread(const std::string&name, std::function<void(Args... args)> cb, Args... args);

    /**
     * @brief 析构函数
     */
    ~Thread();

    /**
     * @brief 获取线程id
     *
     * @return 线程id
     */
    pid_t getId() const {
        return m_id;
    };

    /**
     * @brief 获取线程名称
     *
     * @return 线程名称
     */
    const std::string& getName() const {
        return m_name;
    };

    /**
     * @brief 执行线程
     */
    void join();

    /**
     * @brief 获取当前线程
     *
     * @return 当前线程
     */
    static Thread* GetThis();

    /**
     * @brief 获取线程名称
     *
     * @return 线程名称
     */
    static const std::string& GetName();

    /**
     * @brief 设置线程名称
     *
     * @param[in] name 线程名称
     */
    static void SetName(const std::string& name);

  private:
    /**
     * @brief 线程执行的任务
     *
     * @param[in] val 参数
     *
     * @return 任务返回值
     */
    static void* run(void* val);
  private:
    pid_t					m_id ;			// 线程id
    pthread_t				m_thread ;		// 线程
    Callback_t				m_callback;		// 回调函数
    std::string				m_name;			// 线程名称
    Semaphore				m_semaphore;	// 信号量
};

}
