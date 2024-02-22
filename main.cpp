#include "log.h"
#include <pthread.h>

using namespace mylog;

int main()
{

    mylog::Logger::ptr logger(new Logger);
    logger->addAppender(Appender::ptr(new ConsoleAppender));

    LogEvent::ptr event(new LogEvent(__FILE__, __LINE__, pthread_self(), 0, 0));

    logger->log(event, LogLevel::Level::DEBUG);
    return 0;
}