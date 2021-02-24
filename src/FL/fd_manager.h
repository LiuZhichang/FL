#pragma once

#include <sys/stat.h>
#include <sys/socket.h>
#include <memory>
#include <vector>
#include "mutex.h"
#include "thread.h"
#include "singleton.h"

namespace FL {


/**
 * @brief 文件描述符上下文类
 */
class FdCtx : public std::enable_shared_from_this<FdCtx> {
  public:
    typedef std::shared_ptr<FdCtx> ptr;

    /**
     * @brief 构造函数
     *
     * @param[in] fd 文件描述符
     */
    FdCtx(int fd);

    /**
     * @brief 析构函数
     */
    ~FdCtx();

	/**
	 * @brief 是否初始化完成
	 *
	 * @return 
	 */
    bool isInit() const {
        return m_isInit;
    };

	/**
	 * @brief 该文件描述符是否是socket
	 *
	 * @return 
	 */
    bool isSocket() const {
        return m_isSocket;
    };

	/**
	 * @brief 文件描述符是否关闭
	 *
	 * @return 
	 */
    bool isClose() const {
        return m_isClosed;
    };

	/**
	 * @brief 关闭文件描述符
	 *
	 * @return 
	 */
    bool close();

	/**
	 * @brief 设置用户主动设置非阻塞
	 *
	 * @param[in] opt 是否阻塞
	 */
    void setUserNonblock(bool opt) {
        m_userNonblock = opt;
    }

	/**
	 * @brief 获取是否是用户主动设置非阻塞
	 *
	 * @return 
	 */
    bool getUserNonblock() const {
        return m_userNonblock;
    }

	/**
	 * @brief 设置系统非阻塞
	 *
	 * @param[in] opt 是否阻塞
	 */
    void setSysNonblock(bool opt) {
        m_sysNonblock = opt;
    }

	/**
	 * @brief 获取系统是否为非阻塞
	 *
	 * @return 
	 */
    bool getSysNonblock() const {
        return m_sysNonblock;
    }

	/**
	 * @brief 设置超时时间
	 *
	 * @param[in] type 类型(SO_RCVTIMEO,SO_SNDTIMEO)
	 * @param[in] time 时间Ms
	 */
    void setTimeout(int type, uint64_t time);

	/**
	 * @brief 获取超时时间
	 *
	 * @param[in] type 类型(SO_RCVTIMEO,SO_SNDTIMEO)
	 *
	 * @return 
	 */
    uint64_t getTimeout(int type);
  private:
    /**
     * @brief 初始化
     *
     * @return 初始化成功返回,失败返回false 
     */
    bool init();
  private:
    bool	 m_isInit		= 1;
    bool	 m_isSocket		= 1;
    bool	 m_sysNonblock	= 1;
    bool	 m_userNonblock = 1;
    bool	 m_isClosed		= 1;
    int		 m_fd;
    uint64_t m_recvTimeout;
    uint64_t m_sendTimeout;
};

/**
 * @brief 文件句柄管理类
 */
class FdManager {
  public:
    typedef RWMutex Mutex_t;

	/**
	 * @brief 构造函数
	 */
    FdManager();

	/**
	 * @brief 获取或创建文件句柄
	 *
	 * @param[in] fd 文件描述符
	 * @param[in] auto_create 是否自动创建
	 *
	 * @return 对应的文件描述符上下文
	 */
    FdCtx::ptr get(int fd, bool auto_create = false);

	/**
	 * @brief 删除文件句柄
	 *
	 * @param[in] fd 文件描述符
	 */
    void del(int fd);
  private:
    Mutex_t					m_mutex;
    std::vector<FdCtx::ptr> m_datas;

};

typedef Singleton<FdManager> fd_manager;

}
