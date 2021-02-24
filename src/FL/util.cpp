#include "util.h"
#include "coroutine.h"
#include <sys/select.h>

namespace FL {

namespace UT {
pid_t GetThreadId() {
    return syscall(SYS_gettid);
//    return pthread_self();
}

std::string GetThreadName() {
    return Thread::GetName();
}

uint64_t GetCoroutineId() {
    return Coroutine::GetCoroutineId();
}

void BackTrace(std::vector<std::string>& bt, int size, int skip) {
    void** array = (void**)malloc(sizeof(void*) * size);
    size_t s = backtrace(array, size);
    char** strings = backtrace_symbols(array, s);
    if(strings == NULL) {
        std::cerr << __FILE__ << __LINE__ << "backtrace_symbols error" << std::endl;
    }

    for(size_t i = skip ; i < s ; ++i) {
        bt.push_back(strings[i]);
    }
    free(strings);
    free(array);
}

std::string BacktraceToString(int size, int skip, const std::string& prefix) {
    std::vector<std::string> bt;
    BackTrace(bt, size, skip);
    std::stringstream ss;
    for(size_t i = 0 ; i < bt.size() ; ++i)
        ss << prefix << bt[i] << std::endl;
    return ss.str();
}

uint64_t GetCurrentMs() {
    timeval tv;
    gettimeofday(&tv, 0);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

uint64_t GetCurrentUs() {
    timeval tv;
    gettimeofday(&tv, 0);
    return tv.tv_sec * 1000 * 1000ul + tv.tv_usec / 1000;
}

}

}
