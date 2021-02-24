#pragma once

#include "mutex.h"
#include "scheduler.h"
#include "timer.h"

namespace FL {

class IOManager : public Scheduler, public TimerManager {
  public:
    typedef std::shared_ptr<IOManager> ptr;
    typedef RWMutex Mutex_t;

    /**
     * @brief IO事件
     */
    enum Event : int {
        NONE  = 0x000,
        READ  = 0x001,
        WRITE = 0X004
    };
  private:

    /**
     * @brief 描述符上下文
     */
    struct FdContext {
        typedef Mutex Mutex_t;

        /**
         * @brief 事件上下文
         */
        struct EventContext {
            Scheduler* scheduler;			// 待执行的scheduler
            Coroutine::ptr coroutine;		// 事件协程
            std::function<void()> callback; // 事件的回调函数

            /**
             * @brief 触发事件
             *
             * @param[in] event IO事件
             */
            void triggerEvent(Event event);
        };

        /**
         * @brief 获取事件上下文
         *
         * @param[in] event IO事件
         *
         * @return  对应事件上下文
         */
        EventContext& getContext(Event event);

        /**
         * @brief 重置事件上下文
         *
         * @param[in] ctx 带重置的事件上下文
         */
        void resetContext(EventContext& ctx);

        /**
         * @brief 触发事件并调度
         *
         * @param[in] event IO事件
         */
        void triggerEvent(IOManager::Event event);

        EventContext read;					// 读事件
        EventContext write;					// 写事件
        int fd;								// 事件关联的句柄
        Event m_events;						// 已经注册事件
        Mutex_t mutex;						// 互斥锁
    };

  public:

    /**
     * @brief 构造函数
     *
     * @param[in] threads 线程数量
     * @param[in] use_caller 是否包含调用线程
     * @param[in] name	调度器名称
     */
    IOManager(size_t threads = 1, bool use_caller = true, const std::string& name = "th");

    /**
     * @brief 析构函数
     */
    ~IOManager();

    /**
     * @brief 添加事件
     *
     * @param[in] fd 文件描述符
     * @param[in] event IO事件
     * @param[in] cb 事件的回调函数
     *
     * @return 添加成功返回0,失败返回-1
     */
    int addEvent(int fd, Event event, std::function<void()> cb = nullptr);

    /**
     * @brief 删除事件
     *
     * @param[in] fd 文件描述符
     * @param[in] event IO事件
     *
     * @return 删除成功返回true,失败返回false
     */
    bool delEvent(int fd, Event event);

    /**
     * @brief 取消事件
     *
     * @param[in] fd 文件描述符
     * @param[in] event IO事件
     *
     * @return 删除成功返回true,失败返回false
     */
    bool cancelEvent(int fd, Event event);

    /**
     * @brief 取消所有事件
     *
     * @param[in] fd 文件描述符
     *
     * @return  删除成功返回true,失败返回false
     */
    bool cancelAll(int fd);

    /**
     * @brief 返回当前的调度器
     *
     * @return 当前的调度器
     */
    static IOManager* GetThis();

  protected:

    void tickle()				  override;
    bool stopping() 			  override;
    void idle()					  override;
    void onTimerInsertedAtFront() override;

    /**
     * @brief 重置Socket句柄上下文容器的大小
     *
     * @param[in] size 容量大小
     */
    void contextResize(size_t size);
    bool stopping(uint64_t& timeout);
  private:
    int						m_epfd = 0;
    int						m_fds[2];
    Mutex_t					m_mutex;
    std::atomic<size_t> 	m_pending_event_count = {0};
    std::vector<FdContext*> m_fdContexts;
};

}
