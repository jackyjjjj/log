#pragma once

#include <string>
#include <stdint.h>
#include <memory>
#include <iostream>
#include <fstream>
#include <list>
#include <sstream>
#include <vector>
#include <stdarg.h>

using namespace std;

#define MAKELOG(logger, level)                                                                     \
    do                                                                                             \
    {                                                                                              \
        mylog::LogEvent::ptr event(new mylog::LogEvent(__FILE__, __LINE__, pthread_self(), 0, 0)); \
        (logger)->log(event, mylog::LogLevel::Level::level);                                       \
    } while (0)

namespace mylog
{
    class Logger;
    class Formatter;
    class Appender;
    class LogEvent;
    class LogLevel
    {
    public:
        enum Level
        {
            UNKNOWN = 0,
            DEBUG,
            INFO,
            WARN,
            ERROR,
            FATAL
        };
        static string toString(Level level);
    };

    class LogEvent
    {
    public:
        typedef shared_ptr<LogEvent> ptr;
        LogEvent(const string &filename, int32_t line, uint32_t threadId, uint32_t elapsed,
                 uint64_t timestamp);
        ~LogEvent();
        const string &getFilename() const { return m_filename; }
        string getContent() const { return m_ss.str(); }
        int32_t getLine() const { return m_line; }
        uint32_t getThreadId() const { return m_threadId; }
        uint32_t getElapsed() const { return m_elapsed; }
        uint64_t getTimestamp() const { return m_timestamp; }
        stringstream &getSS() { return m_ss; }

    private:
        const string m_filename;
        int32_t m_line;
        uint32_t m_threadId;
        uint32_t m_elapsed;
        uint64_t m_timestamp;
        stringstream m_ss;
    };

    class Formatter
    {
    public:
        typedef shared_ptr<Formatter> ptr;
        Formatter(const string &pattern) : m_pattern(pattern) { parsePattern(); }
        string format(shared_ptr<Logger> logger, shared_ptr<LogEvent> event, LogLevel::Level level);
        void setPattern(const string &pattern)
        {
            m_pattern = pattern;
        }

        class FormatItem
        {
        public:
            typedef shared_ptr<FormatItem> ptr;
            FormatItem(const string &fmt = "") {}
            virtual void format(ostream &os, shared_ptr<Logger> logger, shared_ptr<LogEvent> event, LogLevel::Level level) = 0;
        };

        void parsePattern();
        bool m_error = false;

    private:
        string m_pattern;
        vector<FormatItem::ptr> m_items;
    };

    class Appender
    {
    public:
        typedef shared_ptr<Appender> ptr;
        virtual void log(shared_ptr<Logger>, shared_ptr<LogEvent>, LogLevel::Level) = 0;
        Formatter::ptr getFormtter() { return m_formatter; }
        void setFormatter(shared_ptr<Formatter> formatter) { m_formatter = formatter; }

    protected:
        LogLevel::Level m_level = LogLevel::Level::DEBUG;
        Formatter::ptr m_formatter;
    };

    class FileAppender : public Appender
    {
    public:
        typedef shared_ptr<FileAppender> ptr;
        FileAppender(string &filename) : m_filename(filename) {}
        void log(shared_ptr<Logger> logger, shared_ptr<LogEvent> event, LogLevel::Level level) override;
        bool reopen();

    private:
        string m_filename;
        ofstream m_filestream;
    };

    class ConsoleAppender : public Appender
    {
    public:
        typedef shared_ptr<ConsoleAppender> ptr;
        void log(shared_ptr<Logger> logger, shared_ptr<LogEvent> event, LogLevel::Level level) override;
    };

    class Logger : public enable_shared_from_this<Logger>
    {
    public:
        typedef shared_ptr<Logger> ptr;
        Logger(const string &name = "root") : m_loggerName(name), m_level(LogLevel::Level::DEBUG)
        {
            m_formatter.reset(new Formatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T[%p]%T%f:%l%T%m%n"));
        }
        void log(shared_ptr<LogEvent> event, LogLevel::Level level);

        void debug(shared_ptr<LogEvent> event);
        void info(shared_ptr<LogEvent> event);
        void warn(shared_ptr<LogEvent> event);
        void error(shared_ptr<LogEvent> event);
        void fatal(shared_ptr<LogEvent> event);

        void addAppender(shared_ptr<Appender> appender);
        void delAppender(shared_ptr<Appender> appender);
        void setFormatter(shared_ptr<Formatter> formatter) { m_formatter = formatter; }
        LogLevel::Level getLevel() const { return m_level; }
        const string getName() const { return m_loggerName; }
        void setLevel(LogLevel::Level level) { m_level = level; }

    private:
        string m_loggerName;
        LogLevel::Level m_level;
        list<Appender::ptr> m_appenders;
        Formatter::ptr m_formatter;
        LogEvent::ptr m_event;
    };

}