#include "../src/FL/uri.h"
#include <iostream>

int main(int argc, char** argv) {

    FL::Uri::ptr uri = FL::Uri::Create("http://www.sylar.top/test/uri?id=100&name=sylar#frg");
    std::cout << *uri << std::endl;
	
	FL::Address::ptr addr = uri->createAddress();

	std::cout << *addr << std::endl;

    return 0;
}
