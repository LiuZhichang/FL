#pragma once

#include <ucontext.h>
#include "thread.h"

namespace FL {

class Scheduler;

/**
 * @brief 协程类
 */
class Coroutine : public std::enable_shared_from_this<Coroutine> {
    friend class Scheduler;
  public:
    typedef std::shared_ptr<Coroutine> ptr;
    typedef std::function<void()> Callback__t;

    /**
     * @brief 协程状态
     */
    enum class State {
        INIT,			// 初始化状态
        SUSPEND,		// 挂起状态
        EXEC,			// 执行状态
        TERMINATE,		// 销毁状态
        READY,			// 准备状态
        EXCEPT			// 异常状态
    };
  public:

    /**
     * @brief 无参构造
     */
    Coroutine();

    /**
     * @brief 构造函数
     *
     * @param[in] callback 回调函数
     * @param[in] stacksize 栈的大小
     */
    Coroutine(std::function<void()> callback, size_t stacksize = 0, bool usr_caller = false);

    /**
     * @brief 析构函数
     */
    ~Coroutine();

    /**
     * @brief 重置上下文以及协程状态
     *
     * @param[in] callback 函数
     */
    void reset(std::function<void()> callback);

    /**
     * @brief 切换协程执行
     */
    void swapIn();

    /**
     * @brief 切换协程到后台
     */
    void swapOut();

    /**
     * @brief 将当前线程切换到执行状态
     */
    void call();

    /**
     * @brief 将当前线程切换到后台
     */
    void back();

    /**
     * @brief 获取协程id
     *
     * @return 协程id
     */
    uint64_t getId() const {
        return m_id;
    }

    /**
     * @brief 获取协程状态
     *
     * @return 协程状态
     */
    State getState() const {
        return m_state;
    }

    /**
     * @brief 设置协程状态
     *
     * @param[in] state 协程状态
     */
    void setState(State state) {
        m_state = state;
    }
  public:

    /**
     * @brief 设置当前协程
     *
     * @param[in] Coroutine 协程
     */
    static void SetThis(Coroutine* Coroutine);

    /**
     * @brief 获取当前协程
     *
     * @return 当前协程
     */
    static Coroutine::ptr GetThis();

    /**
     * @brief 切换协程到准备状态
     */
    static void YieldToReady();

    /**
     * @brief 切换协程到悬挂状态
     */
    static void YieldToSuspend();

    /**
     * @brief 获取协程总数
     *
     * @return 协程总数
     */
    static uint64_t TotalCoroutines();

    /**
     * @brief 获取当前线程下的协程数
     *
     * @return 当前线程下的协程数
     */
    static uint64_t CurThreadCorCnt();

    /**
     * @brief 协程执行的任务
     * @post 执行完成返回到线程主协程
     */
    static void MainFunc();

    /**
     * @brief
     * @post 执行完成返回到线程调度线程
     */
    static void CallerMainFunc();

    /**
     * @brief 获取协程id
     *
     * @return 协程id
     */
    static uint64_t GetCoroutineId();
  private:
    uint64_t	m_id = 0;			// 协程id
    uint32_t	m_stacksize = 0;	// 栈的大小
    State		m_state;			// 协程状态

    void*	    m_stack = nullptr;	// 栈
    ucontext_t  m_ctx;				// 上下文
    Callback__t m_cb;				// 回调函数
};

}
