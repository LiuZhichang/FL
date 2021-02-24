#pragma once

#include <iostream>
#include <memory>
#include <vector>
#include <map>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <arpa/inet.h>

namespace FL {

class IPAddress;

/**
 * @brief 网络地址基类
 */
class Address {
  public:
    typedef std::shared_ptr<Address> ptr;

	/**
	 * @brief 析构函数
	 */
    virtual ~Address() {}

	/**
	 * @brief 获取协议簇
	 *
	 * @return 网络地址对应的协议簇 
	 */
    int getFamily() const;

	/**
	 * @brief 创建Address
	 *
	 * @param[in] address sockaddr指针
	 * @param[in] addrlen sockaddr的长度	
	 *
	 * @return 返回与sockaddr相匹配的Address,失败返回nullptr
	 */
    static Address::ptr		Create(const sockaddr* address, socklen_t addrlen);

	/**
	 * @brief 通过host地址返回对应的所有Address
	 *
	 * @param[in] result   结果集
	 * @param[in] host	   域名,服务器名称等
	 * @param[in] family   协议簇
	 * @param[in] type	   socket类型
	 * @param[in] protocol 协议
	 *
	 * @return 是否转换成功
	 */
    static bool				Lookup(std::vector<Address::ptr>& result
                                   , const std::string& host
                                   , int family = AF_UNSPEC
                                   , int type = 0
                                   , int protocol = 0);

	/**
	 * @brief 通过host地址返回对应的任意一个Address
	 *
	 * @param[in] host		域名,服务器名称等
	 * @param[in] family	协议簇
	 * @param[in] type		socket类型
	 * @param[in] protocol	协议
	 *
	 * @return 返回满足条件的任意Address,否则返回false
	 */
    static Address::ptr	 LookupAny(const std::string& host
                                   , int family = AF_UNSPEC
                                   , int type = 0
                                   , int protocol = 0);

	/**
	 * @brief 通过host地址返回对应的任意一个Address
	 *
	 * @param[in] host		域名,服务器名称等
	 * @param[in] family	协议簇
	 * @param[in] type		socket类型
	 * @param[in] protocol	协议
	 *
	 * @return 返回满足条件的任意Address,否则返回false
	 */
    static std::shared_ptr<IPAddress> LookupAnyIPAddress(const std::string& host
								   , int family = AF_UNSPEC
         						   , int type = 0
         						   , int protocol = 0);

	/**
	 * @brief 返回本机所有网卡的[网卡名,地址,子网掩码位数]
	 *	
	 * @param[in] result	结果集
	 * @param[in] family	协议簇
	 *
	 * @return 是否获取成功
	 */
    static bool GetInterfaceAddress(std::multimap<std::string
                                    , std::pair<Address::ptr, uint32_t>>& result
                                    , int family = AF_UNSPEC);

	/**
	 * @brief 获取指定网卡的地址和子网掩码位数
	 *
	 * @param[in] result	结果集
	 * @param[in] iface		网卡名称
	 * @param[in] family	协议簇
	 *
	 * @return 是否获取成功
	 */
    static bool GetInterfaceAddress(std::vector<std::pair<Address::ptr, uint32_t>>& result
                                    , const std::string& iface
                                    , int family = AF_UNSPEC);

	/**
	 * @brief 获取sockaddr指针
	 *
	 * @return 
	 */
    virtual const sockaddr* getAddr()				 const = 0;
	
	/**
	 * @brief 获取sockaddr的长度
	 *
	 * @return 
	 */
    virtual socklen_t		getAddrLen()			 const = 0;

	/**
	 * @brief 将地址信息输入到指定的输出流中
	 *
	 * @param[in] os	输出流
	 *
	 * @return 包含地址信息的输出流
	 */
    virtual std::ostream&	insert(std::ostream& os) const = 0;

	/**
	 * @brief 将地址转换为字符串
	 *
	 * @return 地址字符串
	 */
    std::string toString() const;

    bool operator <  (const Address& rhs) const;
    bool operator == (const Address& rhs) const;
    bool operator != (const Address& rhs) const;
};

/**
 * @brief IP地址
 */
class IPAddress : public Address {
  public:
    typedef std::shared_ptr<IPAddress> ptr;

	/**
	 * @brief 通过域名,IP,服务器名称创建Address
	 *
	 * @param[in] address	域名,IP,服务器名称
	 * @param[in] port		端口号
	 *
	 * @return 成功返回IPAddress，失败返回nullptr
	 */
    static IPAddress::ptr  Create(const char* address, uint16_t port = 0);

	/**
	 * @brief 获取广播地址
	 *
	 * @param[in] prefix_len	子网掩码位数
	 *
	 * @return	成功返回IPAddress，失败返回nullptr
	 */
    virtual IPAddress::ptr broadcastAddress(uint32_t prefix_len) = 0;

	/**
	 * @brief 获取网段
	 *
	 * @param[in] prefix_len	子网掩码位数
	 *
	 * @return 成功返回IPAddress，失败返回nullptr
	 */
    virtual IPAddress::ptr networkAddress  (uint32_t prefix_len) = 0;

	/**
	 * @brief 获取子网掩码地址
	 *
	 * @param[in] prefix_len	子网掩慢位数
	 *
	 * @return 成功返回IPAddress，失败返回nullptr
	 */
    virtual IPAddress::ptr subNetMask      (uint32_t prefix_len) = 0;

	/**
	 * @brief 获取端口号
	 *
	 * @return 端口号
	 */
    virtual uint32_t getPort() const		= 0;

	/**
	 * @brief 设置端口号
	 *
	 * @param[in] port 端口号
	 */
    virtual void	 setPort(uint16_t port) = 0;
};

/**
 * @brief IPv4地址
 */
class IPv4Address : public IPAddress {
  public:
    typedef std::shared_ptr<IPv4Address> ptr;

	/**
	 * @brief 构造函数
	 *
	 * @param[in] addr sockaddr_in结构体
	 */
    IPv4Address(const sockaddr_in& addr);

	/**
	 * @brief 通过二进制地址构造IPv4Address
	 *
	 * @param[in] address	二进制地址
	 * @param[in] port		端口号
	 */
    IPv4Address(uint32_t address = INADDR_ANY
                                   , uint16_t port = 0);

	/**
	 * @brief 使用点分十进制地址创建IPv4Address
	 *
	 * @param[in] address	点分十进制地址
	 * @param[in] port		端口号
	 *
	 * @return 成功返回IPv4Address,失败返回nullptr
	 */
    static ptr Create(const char* address
                      , uint16_t port = 0);

    const sockaddr* getAddr	  ()			 const override;
    socklen_t		getAddrLen()			 const override;
    std::ostream&   insert(std::ostream& os) const override;

    IPAddress::ptr broadcastAddress(uint32_t prefix_len)	override;
    IPAddress::ptr networkAddress  (uint32_t prefix_len)	override;
    IPAddress::ptr subNetMask      (uint32_t prefix_len)	override;

    uint32_t getPort()		  const override;
    void	 setPort(uint16_t port) override;

  private:
    sockaddr_in m_addr;
};

/**
 * @brief IPv6地址
 */
class IPv6Address : public IPAddress {
  public:
    typedef std::shared_ptr<IPv6Address> ptr;

	/**
	 * @brief 无参构造
	 */
    IPv6Address();

	/**
	 * @brief 通过sockaddr_in6串构造IPv6Address
	 *
	 * @param[in] addr sockaddr_in6结构体
	 */
    IPv6Address(const sockaddr_in6& addr);

	/**
	 * @brief 通过IPv6二进制地址构造IPv6Address
	 *
	 * @param[in] address	二进制地址
	 * @param[in] port		端口号
	 */
    IPv6Address(const uint8_t address[16]
		    	, uint16_t port = 0);

	/**
	 * @brief 通过IPv6地址字符串构造IPv6Address
	 *
	 * @param[in] address	地址字符串
	 * @param[in] port		端口号
	 *
	 * @return 成功返回IP6Address，失败返回nullptr
	 */
    static IPv6Address::ptr Create(const char* address
								  , uint16_t port = 0);

    const sockaddr* getAddr()				 const override;
    socklen_t		getAddrLen()			 const override;
    std::ostream&	insert(std::ostream& os) const override;


    IPAddress::ptr broadcastAddress(uint32_t prefix_len) override;
    IPAddress::ptr networkAddress  (uint32_t prefix_len) override;
    IPAddress::ptr subNetMask      (uint32_t prefix_len) override;

    uint32_t getPort()		  const override;
    void	 setPort(uint16_t port) override;

  private:
    sockaddr_in6 m_addr;
};

/**
 * @brief UunixSocket地址
 */
class UnixAddress : public Address {
  public:
    typedef std::shared_ptr<UnixAddress> ptr;

	/**
	 * @brief 无参构造
	 */
    UnixAddress();

	/**
	 * @brief 通过路径构造UnixAddress
	 *
	 * @param[in] path 路径
	 */
    UnixAddress(const std::string& path);

    const sockaddr* getAddr()				 const override;
    socklen_t		getAddrLen()			 const override;
    std::ostream&	insert(std::ostream& os) const override;

    void setAddrLen(uint32_t len);

  private:
    sockaddr_un m_addr;
    socklen_t m_len;
};

class UnkonwAddress : public Address {
  public:
    typedef std::shared_ptr<UnkonwAddress> ptr;

    UnkonwAddress(int family);
    UnkonwAddress(const sockaddr& addr);

    const sockaddr* getAddr()				 const override;
    socklen_t		getAddrLen()			 const override;
    std::ostream&	insert(std::ostream& os) const override;
  private:
    sockaddr m_addr;
};

std::ostream& operator << (std::ostream& os,const FL::Address& addr);

}
