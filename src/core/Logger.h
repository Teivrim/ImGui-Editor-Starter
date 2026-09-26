#pragma once
#include "Types.h"
#include <string>
#include <mutex>
#include <vector>
#include <cstdio>
#include <ctime>

enum class LogLevel : u8 {
    Debug,
    Info,
    Warn,
    Error,
    Fatal
};

class Logger {
public:
    struct Entry {
        LogLevel level;
        std::string message;
        std::time_t timestamp;
    };

    static Logger& instance() {
        static Logger inst;
        return inst;
    }

    void log(LogLevel level, const std::string& msg);
    void debug(const std::string& msg) { log(LogLevel::Debug, msg); }
    void info(const std::string& msg) { log(LogLevel::Info, msg); }
    void warn(const std::string& msg) { log(LogLevel::Warn, msg); }
    void error(const std::string& msg) { log(LogLevel::Error, msg); }

    void setLevel(LogLevel level) { m_level = level; }
    const std::vector<Entry>& entries() const { return m_entries; }
    void clear() { std::lock_guard lock(m_mutex); m_entries.clear(); }

private:
    LogLevel m_level = LogLevel::Debug;
    std::vector<Entry> m_entries;
    std::mutex m_mutex;

    static const char* levelName(LogLevel level);
};
