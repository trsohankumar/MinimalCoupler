#include "logger.hpp"

namespace MinimalCoupler {

Logger& Logger::getInstance()
{
    static Logger instance;
    return instance;
}

void Logger::setLogLevel(LogLevel level)
{
    minLevel_ = level;
}

Logger::Logger()
    : minLevel_(LogLevel::INFO)
{
}

Logger::~Logger() = default;

std::string Logger::levelStr(LogLevel level) const
{
    switch (level) {
    case LogLevel::DEBUG:
        return "DEBUG";
    case LogLevel::INFO:
        return "INFO";
    case LogLevel::WARNING:
        return "WARN";
    case LogLevel::ERROR:
        return "ERROR";
    default:
        return "UNKNOWN";
    }
}

} // namespace MinimalCoupler