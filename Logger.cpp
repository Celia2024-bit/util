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


Logger::Logger() : outputStream_(&std::cout), currentLevel_(InternalLogLevel::LEVEL2) { // Default to LEVEL2 (INFO)
    formatter_ = defaultFormatter;
}



void Logger::log(uint8_t  level, const std::string& message, const char* file, const char* function, int line) 
{
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
        logMsg.levelName = levelStr;
        logMsg.timestamp = std::chrono::system_clock::now();
        logMsg.file = file;
        logMsg.function = function;
        logMsg.line = line;
        logMsg.message = message;

        std::lock_guard<std::mutex> lock(mutex_);
        *outputStream_ << formatter_(logMsg) << std::endl;
    }
}
    
Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

void Logger::setLevel(InternalLogLevel level) {
    currentLevel_ = level;
    exactLevel_ = std::nullopt; // Clear exact level when setting minimum level
}

void Logger::setExactLevel(InternalLogLevel level) {
    exactLevel_ = level;
}

void Logger::clearExactLevel() {
    exactLevel_ = std::nullopt;
}

bool Logger::isExactLevelSet() const {
    return exactLevel_.has_value();
}

std::optional<InternalLogLevel> Logger::getExactLevel() const {
    return exactLevel_;
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

void Logger::setFormatter(FormatterFunc formatter) {
    formatter_ = formatter;
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