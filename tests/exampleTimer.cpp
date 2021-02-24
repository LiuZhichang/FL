#include "../src/FL/iomanager.h"
#include "../src/FL_LogManager.h"
#include "../src/FL/timer.h"

FL::Timer::ptr g_timer;
void test_timer() {
    FL::IOManager iom(2);
    g_timer = iom.addTimer(1000, []() {
        static int i = 0;
        FL_LOG_INFO(FL_LOG_ROOT()) << "timer";
        if(++i == 3) {
            bool flag =	g_timer->reset(2000, true);
            FL_LOG_INFO(FL_LOG_ROOT()) << "reset: " << flag;
        }
    }, true);


}

int main(int argc, char** argv) {
	FL::Thread::SetName("main");
    test_timer();


    return 0;
}
