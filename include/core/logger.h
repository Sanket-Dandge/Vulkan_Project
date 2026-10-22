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


namespace Uriel {

class Logger {
    public:
        void log(LogLevel level, const char* file, int line, const char* fmt, ...) {
            const char* label = "";
            switch (level) {
                case INFO: label = "INFO"; break;
                case BVERBOSE: label = "BVERBOSE"; break;
                case BWARNING: label = "BWARNING"; break;
                case BERROR: label = "BERROR"; break;
            }


            std::printf("[%s] [%s:%d]: ", label, file, line);

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
};

}


#define LOG_INFO(msg) Uriel::Logger::getInstance()->log(LogLevel::INFO, __FILE__, __LINE__, msg)
#define LOG_VERBOSE(msg) Uriel::Logger::getInstance()->log(LogLevel::BVERBOSE, __FILE__, __LINE__, msg)
#define LOG_WARNING(msg) Uriel::Logger::getInstance()->log(LogLevel::BWARNING, __FILE__, __LINE__, msg)
#define LOG_ERROR(msg) Uriel::Logger::getInstance()->log(LogLevel::BERROR, __FILE__, __LINE__, msg)

#endif
