#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <functional>
#include <mutex>
#include <optional>


/**
 * @struct LogMessage
 * @brief Holds all the contextual information for a single log entry.
 */
struct LogMessage {
    InternalLogLevel level;
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
    static Logger& getInstance();

    void init(const LevelMapping& mappings);
    /**
     * @brief Sets the minimum internal log level to be recorded.
     * @param level The minimum internal log level.
     */
    void setLevel(InternalLogLevel level);

    /**
     * @brief Sets the minimum log level using custom log level enum.
     * @tparam T The custom log level enum type.
     * @param level The minimum log level.
     */
    void setLevel(uint8_t level) {
        currentLevel_ = level;
        exactLevel_ = std::nullopt; // Clear exact level when setting minimum level
    }

    /**
     * @brief Sets the logger to only show messages of exactly this level.
     * @param level The exact internal log level to show.
     */
    void setExactLevel(InternalLogLevel level);

    /**
     * @brief Sets the logger to only show messages of exactly this level using custom log level enum.
     * @tparam T The custom log level enum type.
     * @param level The exact log level to show.
     */
    void setExactLevel(uint8_t level) {
        exactLevel_ = level;
    }

    /**
     * @brief Clears the exact level filter and returns to minimum level filtering.
     */
    void clearExactLevel();

    /**
     * @brief Checks if exact level filtering is currently active.
     * @return true if exact level filtering is active, false otherwise.
     */
    bool isExactLevelSet() const;

    /**
     * @brief Gets the current exact level if set.
     * @return The exact level if set, otherwise std::nullopt.
     */
    std::optional<InternalLogLevel> getExactLevel() const;

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
    void setFormatter(FormatterFunc formatter);


    void log(uint8_t  level, const std::string& message, const char* file, const char* function, int line);
private:
    Logger();
    std::string defaultFormatter(const LogMessage& msg);

    std::mutex mutex_;
    std::ostream* outputStream_;
    std::ofstream fileStream_;
    InternalLogLevel currentLevel_;
    std::optional<InternalLogLevel> exactLevel_; // For exact level filtering
    FormatterFunc formatter_;
    
// Internal mapping from level value to string
    LevelMapping levelToString;

    // Built-in default mappings
    const LevelMapping defaultMappings;

    // Flag indicating whether custom mappings have been initialized
    bool isInitialized;
};

// --- Convenience Macro ---
// This generic macro uses the LogLevelTraits to map the user's level.
#define LOG(level, msg) Logger::getInstance().log(level, msg, __FILE__, __func__, __LINE__)
#define LOGINIT(mappings) Logger::getInstance().init(mappings, __FILE__, __func__, __LINE__)

#endif // LOGGER_H
#endif // LOGGER_H