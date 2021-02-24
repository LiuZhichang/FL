#pragma once

#include <sys/time.h>
#include <memory>
#include <functional>
#include <set>
#include <vector>
#include "mutex.h"

namespace FL {

class TimerManager;

class Timer : public std::enable_shared_from_this<Timer> {
    friend class TimerManager;
  public:
    typedef std::shared_ptr<Timer> ptr;

	/**
	 * @brief 取消定时器
	 *
	 * @return 
	 */
    bool cancel();

	/**
	 * @brief 刷新设置定时器的执行时间
	 *
	 * @return 
	 */
    bool refresh();

	/**
	 * @brief 重置定时器时间
	 *
	 * @param[in] ms		定时器执行时间间隔
	 * @param[in] from_now	是否从当前时间开始计算
	 *
	 * @return 
	 */
    bool reset(uint64_t ms, bool from_now);
  private:

	/**
	 * @brief 构造函数
	 *
	 * @param[in] ms		定时器执行间隔时间
	 * @param[in] cb		回调函数
	 * @param[in] recurring	是否循环
	 * @param[in] manager	定时器管理器
	 */
    Timer(uint64_t ms, std::function<void()> cb
          , bool recurring, TimerManager* manager);

	/**
	 * @brief 构造函数
	 *
	 * @param[in] next	执行的时间戳(ms)
	 */
    Timer(uint64_t next);
  private:
    bool m_recurring = false;		// 是否循环定时器
    uint64_t m_ms = 0;				// 执行周期
    uint64_t m_next = 0;			// 精确的执行时间
    std::function<void()> m_cb;
    TimerManager* m_manager = nullptr;
  private:
    struct Comparator {
        bool operator()(const Timer::ptr& lhs, const Timer::ptr& rhs) const;
    };
};


class TimerManager {
    friend class Timer;
  public:
    typedef FL::RWMutex Mutex_t;

	/**
	 * @brief 构造函数
	 */
    TimerManager();

	/**
	 * @brief 析构函数
	 */
    virtual ~TimerManager();

	/**
	 * @brief 添加定时器
	 *
	 * @param[in] ms		定时器执行间隔时间
	 * @param[in]			定时器回调函数
	 * @param[in] recurring 是否循环定时器
	 *
	 * @return 成功返回Timer,失败返回nullptr
	 */
    Timer::ptr addTimer(uint64_t ms, std::function<void()> cb
                        , bool recurring = false);
	
	/**
	 * @brief 添加条件定时器
	 *
	 * @param[in] ms		定时器执行间隔
	 * @param[in] cb		定时器回调函数
	 * @param[in] weak_ptr	条件
	 * @param[in] recurring	是否循环定时器
	 *
	 * @return 成功返回Timer 
	 */
    Timer::ptr addConditionTimer(uint64_t ms, std::function<void()> cb
                                 , std::weak_ptr<void> weak_cond
                                 , bool recurring = false);

	/**
	 * @brief 到最精一个定时器执行的时间间隔(ms)
	 *
	 * @return 时间间隔(ms)
	 */
    uint64_t getNextTimer();

	/**
	 * @brief 获取需要执行的定时器的回调列表
	 *
	 * @param[in] cbs	回调函数数组
	 */
    void listExpiredcb(std::vector<std::function<void()>>& cbs);

	/**
	 * @brief 是否有定时器
	 *
	 * @return 
	 */
	bool hasTimer();
  protected:

	/**
	 * @brief 当有新的定时器插入到管理器的首部，执行该函数
	 */
    virtual void onTimerInsertedAtFront() = 0;

	/**
	 * @brief 将定时器添加到管理器
	 *
	 * @param[in] val	定时器
	 * @param[in] lock	锁
	 */
	void addTimer(Timer::ptr val,Mutex_t::WriteLock& lock);
  private:

	/**
	 * @brief 检测服务器时间是否被调后
	 *
	 * @param[in] now_ms 当前时间
	 *
	 * @return 
	 */
	bool detectClockRollover(uint64_t now_ms);
  private:
    Mutex_t									m_mutex;
	bool									m_tickled = false;
	uint64_t								m_previousTime;
	std::set<Timer::ptr, Timer::Comparator> m_timers;
};


}
