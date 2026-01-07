#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

namespace MinimalCoupler {

enum class LogLevel {
    DEBUG = 0,
    INFO = 1,
    WARNING = 2,
    ERROR = 3
};

class Logger {
public:
    static Logger& getInstance();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void setLogLevel(LogLevel level);

    void setLogFile(const std::string& filename);

    template<typename... Args>
    void console(LogLevel level, Args&&... args) {
        if (level < minLevel_)
            return;

        std::ostringstream oss;
        oss << "[" << levelStr(level) << "] ";
        (oss << ... << args);

        if (level >= LogLevel::ERROR)
        {
            std::cerr << oss.str() << std::endl;
        }
        else
        {
            std::cout << oss.str() << std::endl;
        }
    }

    template<typename... Args>
    void file(LogLevel level, Args&&... args) {
        if (!logFile_.is_open() || level < minLevel_)
            return;

        std::ostringstream oss;
        oss << "[" << levelStr(level) << "] ";
        (oss << ... << args);

        logFile_ << oss.str() << std::endl;
        logFile_.flush();
    }

private:
    Logger();
    
    ~Logger();

    std::string levelStr(LogLevel level) const;

    LogLevel minLevel_;
    std::ofstream logFile_;
};

#define MINIMALCOUPLER_DEBUG(...) \
    MinimalCoupler::Logger::getInstance().console(MinimalCoupler::LogLevel::DEBUG, __VA_ARGS__)

#define MINIMALCOUPLER_INFO(...) \
    MinimalCoupler::Logger::getInstance().console(MinimalCoupler::LogLevel::INFO, __VA_ARGS__)

#define MINIMALCOUPLER_WARNING(...) \
    MinimalCoupler::Logger::getInstance().console(MinimalCoupler::LogLevel::WARNING, __VA_ARGS__)

#define MINIMALCOUPLER_ERROR(...) \
    MinimalCoupler::Logger::getInstance().console(MinimalCoupler::LogLevel::ERROR, __VA_ARGS__)

#define MINIMALCOUPLER_FILE_DEBUG(...) \
    MinimalCoupler::Logger::getInstance().file(MinimalCoupler::LogLevel::DEBUG, __VA_ARGS__)

#define MINIMALCOUPLER_FILE_INFO(...) \
    MinimalCoupler::Logger::getInstance().file(MinimalCoupler::LogLevel::INFO, __VA_ARGS__)

#define MINIMALCOUPLER_FILE_WARNING(...) \
    MinimalCoupler::Logger::getInstance().file(MinimalCoupler::LogLevel::WARNING, __VA_ARGS__)

#define MINIMALCOUPLER_FILE_ERROR(...) \
    MinimalCoupler::Logger::getInstance().file(MinimalCoupler::LogLevel::ERROR, __VA_ARGS__)

}