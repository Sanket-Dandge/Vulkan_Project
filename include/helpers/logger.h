#ifndef LOGGER_H
#define LOGGER_H

#include <cstdarg>
#include <cstdio>
#include <string>

typedef enum LogLevel {
    INFO,
    BVERBOSE,
    BWARNING,
    BERROR
} loglevel;


namespace capella {

class Logger {
    public:
        void log(LogLevel level, std::string tagName, int line, const char* fmt, ...) {
            const char* label = "";
            switch (level) {
                case INFO: label = "INFO"; break;
                case BVERBOSE: label = "BVERBOSE"; break;
                case BWARNING: label = "BWARNING"; break;
                case BERROR: label = "BERROR"; break;
            }

            std::printf("[%s] [%s:%d]: ", label, tagName.c_str(), line);

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


#define LOG_INFO(tag, msg) capella::Logger::getInstance()->log(LogLevel::INFO, tag, __LINE__, msg)
#define LOG_VERBOSE(tag, msg) capella::Logger::getInstance()->log(LogLevel::BVERBOSE, tag, __LINE__, msg)
#define LOG_WARNING(tag, msg) capella::Logger::getInstance()->log(LogLevel::BWARNING, tag, __LINE__, msg)
#define LOG_ERROR(tag, msg) capella::Logger::getInstance()->log(LogLevel::BERROR, tag, __LINE__, msg)

#define LOG_NODE(fmt, ...) \
    capella::Logger::getInstance()->log(fmt, ##__VA_ARGS__)

#endif
