#include "Logger.h"

void Logger::log(LogLevel level, const std::string& msg) {
    if (level < m_level) return;
    std::lock_guard lock(m_mutex);
    Entry e;
    e.level = level;
    e.message = msg;
    e.timestamp = std::time(nullptr);
    m_entries.push_back(e);

    std::string prefix;
    switch (level) {
        case LogLevel::Debug: prefix = "[DEBUG]"; break;
        case LogLevel::Info:  prefix = "[INFO]";  break;
        case LogLevel::Warn:  prefix = "[WARN]";  break;
        case LogLevel::Error: prefix = "[ERROR]"; break;
        case LogLevel::Fatal: prefix = "[FATAL]"; break;
    }
    std::printf("%s %s\n", prefix.c_str(), msg.c_str());
}
