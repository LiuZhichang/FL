#include "../src/FL/coroutine.h"
#include "../src/FL/logmanager.h"
#include "../src/FL/thread.h"
#include <cstdlib>

using namespace FL;

auto lg = FL_LOG_ROOT();

void test() {
    FL_LOG_INFO(lg) << "test begin";
    Coroutine::GetThis()->back();
    FL_LOG_INFO(lg) << "test end";
    Coroutine::GetThis()->back();
}

void task() {
    FL_LOG_INFO(lg) << "main end " << Coroutine::TotalCoroutines();
}

int main(int argc, char** argv) {

    Thread::SetName("main");

    atexit(task);
    FL_LOG_INFO(lg) << "main begin";
    {
        Coroutine::GetThis();
        Coroutine::ptr cor(new Coroutine(test, 0, true));
        cor->call();
        FL_LOG_INFO(lg) << "----------------";
        cor->call();
        cor->call();
    }
    exit(0);
}
