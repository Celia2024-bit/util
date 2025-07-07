#ifndef ERROR_LOGGER_H
#define ERROR_LOGGER_H

#include <fstream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <string>

class ErrorLogger {
public:
    static void LogError(const std::string& className,
                         const std::string& functionName,
                         const std::string& errorType,
                         const std::string& errorMessage) 
    {
        std::ofstream errorLog("error.log", std::ios::app);
        auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        errorLog << "[" << std::put_time(std::localtime(&now), "%F %T") << "] "
                 << "Error in " << className << "::" << functionName << "\n"
                 << "Type: " << errorType << "\n"
                 << "Message: " << errorMessage << "\n\n";
        errorLog.close();
    }
};
#endif //ERROR_LOGGER_H