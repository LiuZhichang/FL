#include "../src/FL/address.h"
#include "../src/FL_LogManager.h"
#include "../src/FL_Thread.h"
#include <iostream>
#include <thread>
#include <type_traits>
#include <utility>

auto lg = FL_LOG_ROOT();

void test(const char* addr) {
    std::vector<FL::Address::ptr> addrs;

    bool res = FL::Address::Lookup(addrs, addr);
    if(!res) {
        FL_LOG_ERROR(lg) << "lookup failed";
        return;
    }

    for(int i = 0 ; i < addrs.size() ; ++i) {
        FL_LOG_INFO(lg) << i << " - " << addrs[i]->toString();
    }
}

void test_iface() {
    std::multimap<std::string, std::pair<FL::Address::ptr, uint32_t>> results;
    bool res = FL::Address::GetInterfaceAddress(results);

    if(!res) {
        FL_LOG_ERROR(lg) << "getinterfaceaddress failed";
        return;
    }

    for(auto& i : results) {
        FL_LOG_INFO(lg) << i.first << " - " << i.second.first->toString() << " - " << i.second.second;
    }
}

void test_ipv4() {
    FL::Address::ptr addr = FL::IPv4Address::Create("127.0.0.8");
	FL::IPAddress::ptr bdaddr = FL::IPAddress::LookupAnyIPAddress("www.baidu.com");
	bdaddr->setPort(80);
    if(addr) {
        FL_LOG_INFO(lg) << addr->toString();
        FL_LOG_INFO(lg) << bdaddr->toString();
    }
}

class A {
  public:
    template <typename... Args>
    //A(std::function<void(Args... args)> f, Args... args) {
    A(std::string name, void(*fun)(Args... args), Args... args) {
        fun(std::forward<Args>(args)...);
        std::cout << name << std::endl;
    }
};

int main(int argc, char** argv) {
    FL::Thread::SetName("main");
//    A a("a", test, "www.baidu.com:ftp");

//    test_iface();
    test_ipv4();
    return 0;
}
