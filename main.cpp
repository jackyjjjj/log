#include "log.h"
#include <pthread.h>

using namespace mylog;

int main()
{

    mylog::Logger::ptr logger(new mylog::Logger);
    logger->addAppender(mylog::Appender::ptr(new mylog::ConsoleAppender));

    // 使用 LOG 宏记录日志
    MAKELOG(logger, DEBUG);
    return 0;
}