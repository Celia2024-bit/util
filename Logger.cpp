#include "Logger.h"
#include <sstream>
#include <iomanip>

// --- Logger Implementation ---

LevelMapping Logger::levelToString;
const LevelMapping Logger::defaultMappings = {
    {1, "DEBUG"},
    {2, "INFO"},
    {3, "WARN"},
    {4, "ERROR"}
};
bool Logger::isInitialized = false;

void Logger::init(const LevelMapping& mappings) {
    levelToString = mappings;
    isInitialized = true;
}

Logger::Logger() : outputStream_(&std::cout), currentLevel_(DEFAULT_LEVEL) { // Use literal value
    formatter_ = [this](const LogMessage& msg) { return this->defaultFormatter(msg); };
}

void Logger::log(uint8_t  level, const std::string& message, const char* file, const char* function, int line) 
{
    if (isLevelExcluded(level)) 
    {
        return; // Skip logging for excluded levels
    }
    // Check if message should be logged based on current filtering mode
    bool shouldLog = false;
    if (exactLevel_.has_value()) {
        // Exact level filtering: only log if level matches exactly
        shouldLog = (level == exactLevel_.value());
    } else {
        // Minimum level filtering: log if level is >= minimum level
        shouldLog = (level >= currentLevel_);
    }

    if (shouldLog) 
    {
        const std::string* levelStr = nullptr;
        if (isInitialized) 
        {
            auto it = levelToString.find(level);
            if (it != levelToString.end()) 
            {
                levelStr = &it->second;
            }
        } else 
        {
            auto it = defaultMappings.find(level);
            if (it != defaultMappings.end()) 
            {
                levelStr = &it->second;
            }
        }
        LogMessage logMsg;
        logMsg.level = level;
        logMsg.levelName = levelStr ? *levelStr : "UNKNOWN";;
        logMsg.timestamp = std::chrono::system_clock::now();
        logMsg.file = file;
        logMsg.function = function;
        logMsg.line = line;
        logMsg.message = message;

        std::lock_guard<std::mutex> lock(mutex_);
        *outputStream_ << formatter_(logMsg) << std::endl;
    }
}

void Logger::setOutputToFile(const std::string& filename) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (fileStream_.is_open()) {
        fileStream_.close();
    }
    fileStream_.open(filename, std::ios::out | std::ios::app);
    outputStream_ = &fileStream_;
}

void Logger::setOutputToStdout() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (fileStream_.is_open()) {
        fileStream_.close();
    }
    outputStream_ = &std::cout;
}

void Logger::notInclude(uint8_t level) {
    std::lock_guard<std::mutex> lock(mutex_);
    excludedLevels_.insert(level);
}

void Logger::includeBack(uint8_t level) {
    std::lock_guard<std::mutex> lock(mutex_);
    excludedLevels_.erase(level);
}

void Logger::clearExclusions() {
    std::lock_guard<std::mutex> lock(mutex_);
    excludedLevels_.clear();
}

bool Logger::isLevelExcluded(uint8_t level) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return excludedLevels_.find(level) != excludedLevels_.end();
}

std::set<uint8_t> Logger::getExcludedLevels() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return excludedLevels_;
}

std::string Logger::defaultFormatter(const LogMessage& msg) {
    std::stringstream ss;
    auto now_c = std::chrono::system_clock::to_time_t(msg.timestamp);
    ss << "[" << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S") << "] ";
    ss << "[" << msg.levelName << "] "; // Use the user-friendly name
    ss << "[" << msg.file << ":" << msg.function << ":" << msg.line << "] ";
    ss << msg.message;
    return ss.str();
}