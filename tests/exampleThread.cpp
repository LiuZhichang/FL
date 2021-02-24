#include "../src/FL_Thread.h"
#include "../src/FL/util.h"
#include "../src/FL_LogManager.h"
#include <iostream>
#include <memory>
#include <string>
#include <vector>
void func() {
    FL_LOG_INFO(FL_LOG_ROOT()) << FL::Thread::GetName()
                               << " " << FL::UT::GetThreadId()
                               << " task" << std::endl;
}
int main(int argc, char* argv[]) {

    FL::Thread::SetName("main");
    FL_LOG_INFO(FL_LOG_ROOT()) << FL::UT::GetThreadName() << " start";
    std::vector<FL::Thread::ptr> vec;
    for(int i = 0 ; i < 5 ; ++i) {
        FL::Thread::ptr th = std::make_shared<FL::Thread>("name_" + std::to_string(i), func);
        vec.push_back(th);
    }

    for(int i = 0 ; i < 5 ; ++i) {
        vec[i]->join();
    }

    FL_LOG_INFO(FL_LOG_ROOT()) << FL::UT::GetThreadName() << " end";
    return 0;
}
