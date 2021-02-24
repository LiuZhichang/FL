#include "../src/FL/coroutine.h"
#include "../src/FL_LogManager.h"
#include "../src/FL_Thread.h"



void run_in_Coroutine() {
    FL_LOG_INFO(FL_LOG_ROOT()) << "run_in_Coroutine begin";
//    FL::Coroutine::YieldToSuspend();
    FL::Coroutine::GetThis()->back();
    FL_LOG_INFO(FL_LOG_ROOT()) << "run_in_Coroutine end";
//    FL::Coroutine::YieldToSuspend();
    FL::Coroutine::GetThis()->back();
}

void test_Coroutine() {
    FL_LOG_INFO(FL_LOG_ROOT()) << "main begin -1";
    {
        FL::Coroutine::GetThis();
        FL_LOG_INFO(FL_LOG_ROOT()) << "main begin";
        FL::Coroutine::ptr coroutine(new FL::Coroutine(run_in_Coroutine,0,true));
        coroutine->call();
        FL_LOG_INFO(FL_LOG_ROOT()) << "main after swapIn";
        coroutine->call();
        FL_LOG_INFO(FL_LOG_ROOT()) << "main after end";
        coroutine->call();
    }
    FL_LOG_INFO(FL_LOG_ROOT()) << "main after end2";
}

void func() {

}

int main(int argc, char** argv) {
    FL::Thread::SetName("main");

    std::vector<FL::Thread::ptr> thrs;

    test_Coroutine();
    for(int i = 0; i < 5; ++i) {
        thrs.push_back(FL::Thread::ptr(
                           new FL::Thread("name_" + std::to_string(i), &test_Coroutine)));
    }

    for(int i = 0 ; i < thrs.size() ; ++i)
        thrs[i]->join();

    return 0;
}
