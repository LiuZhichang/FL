#pragma once

#include <unistd.h>
#include <pthread.h>
#include <stdint.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <vector>
#include <iostream>
#include <execinfo.h>
#include <sstream>

namespace FL {

namespace UT {

//#ifdef linux

/**
 * @brief 获取线程id
 *
 * @return 线程id
 */
pid_t GetThreadId();
//#endif

/**
 * @brief 获取线程名
 *
 * @return 线程名
 */
std::string GetThreadName();

/**
 * @brief 获取协程id
 *
 * @return 协程id
 */
uint64_t GetCoroutineId();

/**
 * @brief 栈回解信息
 *
 * @param[in] bt 每层的信息
 * @param[in] size 层数
 * @param[in] skip 跳过层数
 */
void BackTrace(std::vector<std::string>& bt, int size, int skip = 1);

/**
 * @brief 将栈回解的信息转换为文本形式
 *
 * @param[in] size 回解层数
 * @param[in] skip 跳过层数
 * @param[in] prefix 格式前缀
 *
 * @return 回解信息文本
 */
std::string BacktraceToString(int size = 64, int skip = 2, const std::string& prefix = "");

/**
 * @brief 获取当前毫秒数
 *
 * @return 当前毫秒数
 */
uint64_t GetCurrentMs();

/**
 * @brief 获取当前微秒数
 *
 * @return 当前微秒数
 */
uint64_t GetCurrentUs();

}
}
