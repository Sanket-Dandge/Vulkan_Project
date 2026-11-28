#ifndef LOGGER_H
#define LOGGER_H

#include <cstdarg>
#include <cstdio>

typedef enum LogLevel {
    INFO,
    BVERBOSE,
    BWARNING,
    BERROR
} loglevel;

namespace capella {

class Logger {
public:
    void log(LogLevel level, const char* tagName, int line,
             const char* fmt, ...) {
        const char* label = "";

        switch (level) {
            case INFO:     label = "INFO";     break;
            case BVERBOSE: label = "BVERBOSE"; break;
            case BWARNING: label = "BWARNING"; break;
            case BERROR:   label = "BERROR";   break;
        }

        std::printf("[%s] [%s:%d]: ", label, tagName, line);

        va_list args;
        va_start(args, fmt);
        std::vprintf(fmt, args);
        va_end(args);

        std::printf("\n");
    }

    static Logger* getInstance() {
        static Logger instance;
        return &instance;
    }

    void log(const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        std::vprintf(fmt, args);
        va_end(args);

        std::printf("\n");
    }
};

}

// Logging macros
#define LOG_INFO(tag, ...) capella::Logger::getInstance()->log(LogLevel::INFO, tag, __LINE__, __VA_ARGS__)

#define LOG_VERBOSE(tag, ...) capella::Logger::getInstance()->log(LogLevel::BVERBOSE, tag, __LINE__, __VA_ARGS__)

#define LOG_WARNING(tag, ...) capella::Logger::getInstance()->log(LogLevel::BWARNING, tag, __LINE__, __VA_ARGS__)

#define LOG_ERROR(tag, ...) capella::Logger::getInstance()->log(LogLevel::BERROR, tag, __LINE__, __VA_ARGS__)

#define LOG_NODE(...) capella::Logger::getInstance()->log(__VA_ARGS__)

#endif
