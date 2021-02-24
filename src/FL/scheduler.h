#pragma once

#include <functional>
#include <vector>
#include <list>
#include <atomic>
#include "coroutine.h"

namespace FL {

/**
 * @brief 协程调度器
 */
class Scheduler {
  public:
    typedef std::shared_ptr<Scheduler> ptr;
    typedef FL::Mutex Mutex_t;

    /**
     * @brief 构造函数
     *
     * @param[in] thread_size 线程数量
     * @param[in] use_caller 是否使用当前调用线程
     * @param[in] name 协程调度器名
     */
    Scheduler(size_t thread_size = 1, bool use_caller = true, const std::string& name = "th");

    /**
     * @brief 析构函数
     */
    virtual ~Scheduler();

    /**
     * @brief 获取协程调度器名
     *
     * @return 协程调度器名
     */
    const std::string& getName() const {
        return m_name;
    }

    /**
     * @brief 获取当前协程调度器
     *
     * @return 当前协程调度器
     */
    static Scheduler* GetThis();

    /**
     * @brief 获取主协程
     *
     * @return 主协程
     */
    static FL::Coroutine* GetMainCoroutine();

    /**
     * @brief 启动协程调度器
     */
    void start();

    /**
     * @brief 停止协程调度器
     */
    void stop();

    /**
     * @brief 批量调度协程
     *
     * @tparam CoroutineOrCallback 协程或者回调函数
     * @param[in] coc 协程或者回调函数
     * @param[in] thread_id 线程id,-1标识任意线程
     */
    template <typename CoroutineOrCallback>
    void schedule(CoroutineOrCallback coc, int thread_id = -1) {
        bool need_tickle = false;
        {
            Mutex_t::Lock lock(m_mutex);
            need_tickle = scheduleNoLock(coc, thread_id);
        }
        if(need_tickle) {
            tickle();
        }
    }

    /**
     * @brief 批量调度协程
     *
     * @tparam InputIterator 迭代器
     * @param[in] begin 协程数组的开始
     * @param[in] end 协程数组的结束
     */
    template <class InputIterator>
    void schedule(InputIterator begin, InputIterator end) {
        bool need_tickle = false;
        {
            Mutex_t::Lock lock(m_mutex);
            while(begin != end) {
                need_tickle = scheduleNoLock(&*begin, -1) || need_tickle;
                ++begin;
            }
        }
        if(need_tickle) {
            tickle();
        }
    }

  protected:

    /**
     * @brief 通知协程调度器有任务了
     */
    virtual void tickle();

    /**
     * @brief 执行调度任务
     */
    void run();

    /**
     * @brief 返回是否可以停止
     *
     * @return 是否可以停止
     */
    virtual bool stopping();

    /**
     * @brief 协程无任务时执行空闲协程
     */
    virtual void idle();

    /**
     * @brief 设置当前的协程调度器
     */
    void setThis();

    /**
     * @brief 是否有空闲协程
     *
     * @return 有无空闲协程
     */
    bool hasIdleThreads() {
        return m_idleThreadCount > 0;
    }


  private:

    /**
     * @brief 协程调度器启动(无锁)
     *
     * @tparam CoroutineOrCallback 协程或者回调函数
     * @param[in] coc 协程或者回调函数
     * @param[in] thread_id 线程id
     *
     * @return 是否需要通知调度器有任务了
     */
    template <typename CoroutineOrCallback>
    bool scheduleNoLock(CoroutineOrCallback coc, int thread_id) {
        bool need_tickle = m_coroutines.empty();
        SchedulerDetails sd(coc, thread_id);
        if(sd.coroutine || sd.callback)
            m_coroutines.push_back(sd);
        return need_tickle;
    }
  private:

    /**
     * @brief 调度细节(协程 函数 线程组)
     */
    struct SchedulerDetails {
        Coroutine::ptr coroutine;
        std::function<void()> callback;
        int thread_id;

        /**
         * @brief 构造函数
         *
         * @param[in] cor 协程
         * @param[in] tid 线程id
         */
        SchedulerDetails(FL::Coroutine::ptr cor, int tid)
            : coroutine(cor)
            , thread_id(tid) {}

        /**
         * @brief 构造函数
         *
         * @param[in] pcor 协程指针
         * @param[in] tid 线程id
         */
        SchedulerDetails(FL::Coroutine::ptr* pcor, int tid)
            : thread_id(tid) {
            coroutine.swap(*pcor);
        }

        /**
         * @brief 构造函数
         *
         * @param[in] cb 回调函数
         * @param[in] tid 线程id
         */
        SchedulerDetails(std::function<void()> cb, int tid)
            : callback(cb)
            , thread_id(tid) {}

        /**
         * @brief 构造函数
         *
         * @param[in] cb 回调函数指针
         * @param[in] tid 线程id
         */
        SchedulerDetails(std::function<void()>* cb, int tid)
            : thread_id(tid) {
            callback.swap(*cb);
        }

        /**
         * @brief 无参构造
         */
        SchedulerDetails()
            : thread_id(-1) {}

        /**
         * @brief 重置数据
         */
        void reset() {
            coroutine = nullptr;
            callback = nullptr;
            thread_id = -1;
        }
    };
  private:
    Mutex_t m_mutex;								// 互斥锁
    std::vector<FL::Thread::ptr> m_threads; 		// 线程池
    std::list<SchedulerDetails> m_coroutines;		// 待执行的协程队列
    FL::Coroutine::ptr m_mainCoroutine;				// use_caller为true时有效,调度协程
    std::string m_name;								// 协程调度器名
  protected:
    std::vector<pid_t> m_threadIds;					// 协程下的线程id数组
    size_t m_threadCount = 0;						// 线程数量
    std::atomic<size_t> m_activeThreadCount = {0};  // 工作线程数量
    std::atomic<size_t> m_idleThreadCount = {0};    // 空闲线程数量
    bool m_stopping = true;							// 是否正在停止
    bool m_autoStop = false;						// 是否自动停止
    int m_mainThread = 0;							// 主线程id(use_caller)
};

}
