#include "../src/FL_LogManager.h"
#include "../src/FL_Thread.h"
#include "../src/FL/scheduler.h"

using namespace FL;

void test_fiber() {
    static int s_count = 5;
    FL_LOG_INFO(FL_LOG_ROOT()) << "test in fiber s_count=" << s_count;
    FL_LOG_INFO(FL_LOG_ROOT()) << "test in fiber ";

//    sleep(1);

    if(--s_count >= 0) {
        FL::Scheduler::GetThis()->schedule(&test_fiber, FL::UT::GetThreadId());
    }
}

void test() {
    FL::Thread::SetName("main");
    FL_LOG_INFO(FL_LOG_ROOT()) << "main start";
    {
        Scheduler::GetThis();
        FL::Scheduler sc(2, true, "test");
        sc.start();
        sleep(1);
        FL_LOG_INFO(FL_LOG_ROOT()) << "schedule";
        sc.schedule(&test_fiber);
        sc.stop();
    }
    FL_LOG_INFO(FL_LOG_ROOT()) << "main end";
}

void task() {
    FL_LOG_INFO(FL_LOG_ROOT()) << "task in fiber ";
}

int main(int argc, char** argv) {
    Scheduler sc(2);
    sc.start();
    sc.schedule(task);
    sc.stop();
    return 0;
}
