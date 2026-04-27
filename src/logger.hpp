#pragma once

#include <iostream>
#include <sstream>
#include <string>

namespace MinimalCoupler {

enum class LogLevel { DEBUG = 0, INFO = 1, WARNING = 2, ERROR = 3 };

class Logger {
public:
    static Logger& getInstance();

    Logger(const Logger&)            = delete;
    Logger& operator=(const Logger&) = delete;

    void setLogLevel(LogLevel level);

    template <typename... Args> void console(LogLevel level, Args&&... args) const
    {
        if (level < minLevel_)
            return;

        std::ostringstream oss;
        oss << "[" << levelStr(level) << "] ";
        (oss << ... << args);

        if (level >= LogLevel::ERROR) {
            std::cerr << oss.str() << std::endl;
        } else {
            std::cout << oss.str() << std::endl;
        }
    }

private:
    Logger();

    ~Logger();

    std::string levelStr(LogLevel level) const;

    LogLevel minLevel_;
};

#define MINIMALCOUPLER_DEBUG(...)                                                                                      \
    MinimalCoupler::Logger::getInstance().console(MinimalCoupler::LogLevel::DEBUG, __VA_ARGS__)

#define MINIMALCOUPLER_INFO(...)                                                                                       \
    MinimalCoupler::Logger::getInstance().console(MinimalCoupler::LogLevel::INFO, __VA_ARGS__)

#define MINIMALCOUPLER_WARNING(...)                                                                                    \
    MinimalCoupler::Logger::getInstance().console(MinimalCoupler::LogLevel::WARNING, __VA_ARGS__)

#define MINIMALCOUPLER_ERROR(...)                                                                                      \
    MinimalCoupler::Logger::getInstance().console(MinimalCoupler::LogLevel::ERROR, __VA_ARGS__)


} // namespace MinimalCoupler