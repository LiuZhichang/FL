#include "../src/FL_LogManager.h"
#include <memory>
#include <unistd.h>

int main(int argc, char** argv) {

    FL::Logger::ptr logger = FL_LOG_NAME("lzc");
    FL::StdOutLogAppender::ptr console = std::make_shared<FL::StdOutLogAppender>();
    FL::FileLogAppender::ptr file = std::make_shared<FL::FileLogAppender>("examplelog.log");
    logger->addAppender(console);
    logger->addAppender(file);

    // FL_LOG_DEBUG(logger) << "level : debug";
    // FL_LOG_INFO(logger) << "level : info";
    // FL_LOG_ERROR(logger) << "level : error";
    // FL_LOG_FATAL(logger) << "level : fatal";

    FL_LOG_INFO(FL_SYS_LOG()) << "system log";

    return 0;
}
