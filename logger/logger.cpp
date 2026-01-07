#include "logger.hpp"

namespace MinimalCoupler
{

    Logger& Logger::getInstance()
    {
        static Logger instance;
        return instance;
    }


    void Logger::setLogLevel(LogLevel level)
    {
        minLevel_ = level;
    }

    void Logger::setLogFile(const std::string &filename)
    {
        if (logFile_.is_open())
        {
            logFile_.close();
        }
        logFile_.open(filename, std::ios::app);
    }

    template <typename... Args>
    void Logger::console(LogLevel level, Args &&...args)
    {
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

    template <typename... Args>
    void Logger::file(LogLevel level, Args &&...args)
    {
        if (!logFile_.is_open() || level < minLevel_)
            return;

        std::ostringstream oss;
        oss << "[" << levelStr(level) << "] ";
        (oss << ... << args);

        logFile_ << oss.str() << std::endl;
        logFile_.flush();
    }


    Logger::Logger() : minLevel_(LogLevel::INFO) {}
    
    Logger::~Logger() {
        if (logFile_.is_open()) {
            logFile_.close();
        }
    }


    std::string Logger::levelStr(LogLevel level) const {
        switch (level) {
            case LogLevel::DEBUG: return "DEBUG";
            case LogLevel::INFO: return "INFO";
            case LogLevel::WARNING: return "WARN";
            case LogLevel::ERROR: return "ERROR";
            default: return "UNKNOWN";
        }
    }

}