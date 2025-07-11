#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <functional>
#include <mutex>
#include <optional>
#include <unordered_map>
#include <set>

constexpr uint8_t DEFAULT_LEVEL = 2;
/**
 * @struct LogMessage
 * @brief Holds all the contextual information for a single log entry.
 */
struct LogMessage {
    uint8_t level;
    std::string levelName; // Store the user-friendly name
    std::chrono::system_clock::time_point timestamp;
    std::string file;
    std::string function;
    int line;
    std::string message;
};

// A function pointer type for custom log formatters.
using FormatterFunc = std::function<std::string(const LogMessage&)>;
using LevelMapping = std::unordered_map<uint8_t, std::string>;
/**
 * @class Logger
 * @brief A singleton logger class that handles logging messages to various outputs.
 *
 * This class is thread-safe and supports custom log level mappings.
 */
class Logger {
public:
    // Delete copy constructor and assignment operator to enforce singleton pattern.
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    /**
     * @brief Get the singleton instance of the Logger.
     * @return Reference to the Logger instance.
     */
    static Logger& getInstance()
    {
         static Logger instance;
         return instance;
    }

    void init(const LevelMapping& mappings);

    /**
     * @brief Sets the minimum log level using custom log level enum.
     * @tparam T The custom log level enum type.
     * @param level The minimum log level.
     */
    void setLevel(uint8_t level) 
    {
       currentLevel_ = level;
       exactLevel_ = std::nullopt; 
    }

    void setDefaultLevel()
    {
        currentLevel_ = DEFAULT_LEVEL;
    }

    void setDefault()
    {
        clearExclusions(); // Clear all exclusions first
        setDefaultLevel();
        clearExactLevel();
        setDefaultFormatter();
        
    }
    /**
     * @brief Sets the logger to only show messages of exactly this level using custom log level enum.
     * @tparam T The custom log level enum type.
     * @param level The exact log level to show.
     */
    void setExactLevel(uint8_t level) 
    {
       exactLevel_ = level;
    }

    /**
     * @brief Clears the exact level filter and returns to minimum level filtering.
     */
    void clearExactLevel()
    {
        exactLevel_ = std::nullopt;
    }

    /**
     * @brief Checks if exact level filtering is currently active.
     * @return true if exact level filtering is active, false otherwise.
     */
    bool isExactLevelSet() const
    {
        return exactLevel_.has_value();
    }

    /**
     * @brief Gets the current exact level if set.
     * @return The exact level if set, otherwise std::nullopt.
     */
    std::optional<uint8_t> getExactLevel() const
    {
        return exactLevel_;
    }

    /**
     * @brief Sets the output to a file.
     * @param filename The path to the log file.
     */
    void setOutputToFile(const std::string& filename);

    /**
     * @brief Sets the output to the standard console output (stdout).
     */
    void setOutputToStdout();

    /**
     * @brief Sets a custom function for formatting log messages.
     * @param formatter A function that takes a LogMessage and returns a formatted string.
     */
    void setFormatter(FormatterFunc formatter)
    {
        formatter_ = formatter;
    }

    void setDefaultFormatter()
    {
        formatter_ = [this](const LogMessage& msg) { return this->defaultFormatter(msg); };
    }
 /**
     * @brief Adds a log level to the exclusion list.
     * @param level The log level to exclude from output.
     */
    void notInclude(uint8_t level);

    /**
     * @brief Removes a log level from the exclusion list.
     * @param level The log level to stop excluding.
     */
    void includeBack(uint8_t level);

    /**
     * @brief Clears all excluded log levels.
     */
    void clearExclusions();

    /**
     * @brief Checks if a specific level is currently excluded.
     * @param level The log level to check.
     * @return true if the level is excluded, false otherwise.
     */
    bool isLevelExcluded(uint8_t level) const;

    /**
     * @brief Gets all currently excluded levels.
     * @return A set containing all excluded log levels.
     */
    std::set<uint8_t> getExcludedLevels() const;

    void log(uint8_t  level, const std::string& message, const char* file, const char* function, int line);
private:
    Logger();
    std::string defaultFormatter(const LogMessage& msg);

    mutable std::mutex mutex_;
    std::ostream* outputStream_;
    std::ofstream fileStream_;
    uint8_t currentLevel_;
    std::optional<uint8_t> exactLevel_; // For exact level filtering
    FormatterFunc formatter_;
    
    // Static members for level mapping
    static LevelMapping levelToString;
    static const LevelMapping defaultMappings;
    static bool isInitialized;
    std::set<uint8_t> excludedLevels_;
};

// --- Convenience Macro ---
// This generic macro uses the LogLevelTraits to map the user's level.
#define LOG(level, msg) Logger::getInstance().log(level, msg, __FILE__, __func__, __LINE__)
#define LOGINIT(mappings) Logger::getInstance().init(mappings)

#endif // LOGGER_H
