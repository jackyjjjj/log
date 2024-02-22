#include "log.h"

#include <functional>
#include <map>

#define DO 0

namespace mylog
{
    string LogLevel::toString(LogLevel::Level level)
    {
        switch (level)
        {
#define XX(name)                \
    case LogLevel::Level::name: \
        return #name;
            break;
            XX(DEBUG)
            XX(INFO)
            XX(WARN)
            XX(ERROR)
            XX(FATAL)
#undef XX
        default:
            return "UNKNOWN";
            break;
        }
    }

    class MessageFormatItem : public Formatter::FormatItem
    {
    public:
        MessageFormatItem(const string &fmt) {}
        void format(ostream &os, shared_ptr<Logger> logger, shared_ptr<LogEvent> event, LogLevel::Level level) override
        {
#if DO
            cout << "message" << endl;
#endif
            os << event->getContent();
        }
    };
    class LevelFormatItem : public Formatter::FormatItem
    {
    public:
        LevelFormatItem(const string &fmt) {}
        void format(ostream &os, shared_ptr<Logger> logger, shared_ptr<LogEvent> event, LogLevel::Level level) override
        {
#if DO
            cout << "level" << endl;
#endif
            os << LogLevel::toString(level);
        }
    };
    class ElapseFormatItem : public Formatter::FormatItem
    {
    public:
        ElapseFormatItem(const string &fmt) {}
        void format(ostream &os, shared_ptr<Logger> logger, shared_ptr<LogEvent> event, LogLevel::Level level) override
        {
#if DO
            cout << "elapse" << endl;
#endif
            os << event->getElapsed();
        }
    };
    class NameFormatItem : public Formatter::FormatItem
    {
    public:
        NameFormatItem(const string &fmt) {}
        void format(ostream &os, shared_ptr<Logger> logger, shared_ptr<LogEvent> event, LogLevel::Level level) override
        {
#if DO
            cout << "name" << endl;
#endif
            os << logger->getName();
        }
    };
    class ThreadFormatItem : public Formatter::FormatItem
    {
    public:
        ThreadFormatItem(const string &fmt) {}
        void format(ostream &os, shared_ptr<Logger> logger, shared_ptr<LogEvent> event, LogLevel::Level level) override
        {
#if DO
            cout << "thread" << endl;
#endif
            os << event->getThreadId();
        }
    };
    class DataTimeFormatItem : public Formatter::FormatItem
    {
    public:
        DataTimeFormatItem(const std::string &format = "%Y-%m-%d %H:%M:%S")
            : m_format(format)
        {
            if (m_format.empty())
            {
                m_format = "%Y-%m-%d %H:%M:%S";
            }
        }

        void format(ostream &os, shared_ptr<Logger> logger, shared_ptr<LogEvent> event, LogLevel::Level level) override
        {
            struct tm tm;
            time_t time = event->getTimestamp();
            localtime_r(&time, &tm);
            char buf[64];
            strftime(buf, sizeof(buf), m_format.c_str(), &tm);
            os << buf;
#if DO
            cout << "time" << endl;
#endif
        }

    private:
        string m_format;
    };
    class LineFormatItem : public Formatter::FormatItem
    {
    public:
        LineFormatItem(const string &fmt) {}
        void format(ostream &os, shared_ptr<Logger> logger, shared_ptr<LogEvent> event, LogLevel::Level level) override
        {
#if DO
            cout << "line" << endl;
#endif
            os << event->getLine();
        }
    };
    class NewLineFormatItem : public Formatter::FormatItem
    {
    public:
        NewLineFormatItem(const string &fmt) {}
        void format(ostream &os, shared_ptr<Logger> logger, shared_ptr<LogEvent> event, LogLevel::Level level) override
        {
#if DO
            cout << "newline" << endl;
#endif
            os << endl;
        }
    };
    class FileFormatItem : public Formatter::FormatItem
    {
    public:
        FileFormatItem(const string &fmt) {}
        void format(ostream &os, shared_ptr<Logger> logger, shared_ptr<LogEvent> event, LogLevel::Level level) override
        {
#if DO
            cout << "file" << endl;
#endif
            os << event->getFilename();
        }
    };
    class StringFormatItem : public Formatter::FormatItem
    {
    public:
        StringFormatItem(const string &fmt) : m_string(fmt) {}
        void format(ostream &os, shared_ptr<Logger> logger, shared_ptr<LogEvent> event, LogLevel::Level level) override
        {
#if DO
            cout << "string" << endl;
#endif
            os << m_string;
        }

    private:
        string m_string;
    };
    class TabFormatItem : public Formatter::FormatItem
    {
    public:
        TabFormatItem(const string &fmt) {}
        void format(ostream &os, shared_ptr<Logger> logger, shared_ptr<LogEvent> event, LogLevel::Level level) override
        {
#if DO
            cout << "tab" << endl;
#endif
            os << "\t";
        }
    };

    LogEvent::LogEvent(const string &filename, int32_t line, uint32_t threadId, uint32_t elapsed, uint64_t timestamp)
        : m_filename(filename), m_line(line), m_threadId(threadId), m_elapsed(elapsed), m_timestamp(timestamp)
    {
#if DO
        cout << "log event" << endl;
#endif
    }
    LogEvent::~LogEvent()
    {
    }

    string Formatter::format(Logger::ptr logger, LogEvent::ptr event, LogLevel::Level level)
    {
#if DO
        cout << "Formatter::format" << endl;
#endif
        stringstream ss;
        for (auto &i : m_items)
        {
            i->format(ss, logger, event, level);
        }
#if DO
        cout << "Formatter::format" << endl;
#endif
        return ss.str();
    }
    void Formatter::parsePattern()
    {
        vector<tuple<string, string, int>> vec;
        string nstr;
        for (size_t i = 0; i < m_pattern.size(); ++i)
        {
            if (m_pattern[i] != '%')
            {
                nstr.append(1, m_pattern[i]);
                continue;
            }
            size_t n = i + 1;
            int fmt_status = 0;
            size_t fmt_begin = 0;
            string str;
            string fmt;
            while (n < m_pattern.size())
            {
                if (!fmt_status && (!isalpha(m_pattern[n]) && m_pattern[n] != '{' && m_pattern[n] != '}'))
                {
                    str = m_pattern.substr(i + 1, n - i - 1);
                    break;
                }

                if (fmt_status == 0)
                {
                    if (m_pattern[n] == '{')
                    {
                        str = m_pattern.substr(i + 1, n - i - 1);
                        fmt_status = 1; // 解析格式
                        fmt_begin = n;
                        ++n;
                        continue;
                    }
                }
                if (fmt_status == 1)
                {
                    if (m_pattern[n] == '}')
                    {
                        fmt = m_pattern.substr(fmt_begin + 1, n - fmt_begin - 1);
                        fmt_status = 0;
                        ++n;
                        break;
                    }
                }
                ++n;
                if (n == m_pattern.size())
                {
                    if (str.empty())
                    {
                        str = m_pattern.substr(i + 1);
                    }
                }
            }
            if (fmt_status == 0)
            {
                if (!nstr.empty())
                {
                    vec.push_back(make_tuple(nstr, string(), 0));
                    nstr.clear();
                }
                vec.push_back(make_tuple(str, fmt, 1));
                i = n - 1;
            }
            else if (fmt_status == 1)
            {
                std::cout << "pattern parse error: " << m_pattern << " - " << m_pattern.substr(i) << std::endl;
                m_error = true;
                vec.push_back(std::make_tuple("<<pattern_error>>", fmt, 0));
            }
        }

        if (!nstr.empty())
        {
            vec.push_back(make_tuple(nstr, string(), 0));
        }

        /*
            %m 消息体
            %p level
            %r 启动后的时间
            %c 日志名称
            %n 回车换行
            %d 时间
            %f 文件名
            %l 行号
        */
        static map<string, function<Formatter::FormatItem::ptr(const string &str)>> s_format_items = {
#define XX(str, C)                                                                    \
    {                                                                                 \
        str, [](const string &fmt) { return Formatter::FormatItem::ptr(new C(fmt)); } \
    }
            XX("m", MessageFormatItem),
            XX("p", LevelFormatItem),
            XX("r", ElapseFormatItem),
            XX("c", NameFormatItem),
            XX("t", ThreadFormatItem),
            XX("n", NewLineFormatItem),
            XX("d", DataTimeFormatItem),
            XX("f", FileFormatItem),
            XX("l", LineFormatItem),
            XX("T", TabFormatItem)
#undef XX
        };
        for (auto &i : vec)
        {
            if (get<2>(i) == 0)
            {
                m_items.push_back(FormatItem::ptr(new StringFormatItem(get<0>(i))));
            }
            else
            {
                auto it = s_format_items.find(get<0>(i));
                if (it == s_format_items.end())
                {
                    m_items.push_back(FormatItem::ptr(new StringFormatItem("<<error_format %" + get<0>(i) + ">>")));
                    m_error = true;
                }
                else
                {
                    m_items.push_back(it->second(get<1>(i)));
                }
            }
            std::cout << "(" << std::get<0>(i) << ") - (" << std::get<1>(i) << ") - (" << std::get<2>(i) << ")" << std::endl;
        }
    }

    void FileAppender::log(Logger::ptr logger, LogEvent::ptr event, LogLevel::Level level)
    {
        if (level >= m_level)
        {
            m_filestream << m_formatter->format(logger, event, level) << std::endl;
            m_filestream.flush();
        }
    }
    bool FileAppender::reopen()
    {
        if (m_filestream.is_open())
        {
            m_filestream.close();
        }
        m_filestream.open(m_filename);
        return !m_filestream.fail();
    }

    void ConsoleAppender::log(Logger::ptr logger, LogEvent::ptr event, LogLevel::Level level)
    {
#if DO
        cout << "ConsoleAppender::log" << endl;
#endif
        if (level >= m_level)
        {
            cout << m_formatter->format(logger, event, level) << endl;
        }
    }

    void Logger::addAppender(Appender::ptr appender)
    {
#if DO
        cout << "add appender" << endl;
        cout << "line:" << __LINE__ << endl;
#endif
        if (!appender->getFormtter())
        {
            appender->setFormatter(m_formatter);
        }
        m_appenders.push_back(appender);
    }
    void Logger::delAppender(Appender::ptr appender)
    {
        for (auto it = m_appenders.begin(); it != m_appenders.end(); ++it)
        {
            if (*it == appender)
            {
                m_appenders.erase(it);
                break;
            }
        }
    }

    void Logger::log(LogEvent::ptr event, LogLevel::Level level)
    {
#if DEBUG
        cout << "Logger::log" << endl;
#endif
        auto self = shared_from_this();
        for (auto &i : m_appenders)
        {
            i->log(self, event, level);
        }
    }
    void Logger::debug(LogEvent::ptr event) { log(event, LogLevel::DEBUG); }
    void Logger::info(LogEvent::ptr event) { log(event, LogLevel::INFO); }
    void Logger::warn(LogEvent::ptr event) { log(event, LogLevel::WARN); }
    void Logger::error(LogEvent::ptr event) { log(event, LogLevel::ERROR); }
    void Logger::fatal(LogEvent::ptr event) { log(event, LogLevel::FATAL); }
}