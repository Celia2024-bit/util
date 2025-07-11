#include "Logger.h"
#include <iostream>
#include <sstream>

// --- User-Defined Log Level and Mapping ---


// --- Main function to demonstrate the Logger ---
int main() {
    enum CustomerLogLevel 
    {
        DEBUG = 1,
        INFO,
        WARN,
        ERROR
    };
    
    LevelMapping customMappings = {
        {DEBUG, "CUSTOM_DEBUG"},
        {INFO, "CUSTOM_INFO"},
        {WARN, "CUSTOM_WARN"},
        {ERROR, "CUSTOM_ERROR"}
    };

    LOGINIT(customMappings);

    // --- 1. Default Logging (INFO level to stdout) ---
    std::cout << "--- 1. Default Logging Example ---" << std::endl;
    // The default level is LEVEL2, which maps to CustomerLogLevel::INFO.
    LOG(CustomerLogLevel::INFO, "This is an info message.");
    LOG(CustomerLogLevel::WARN, "This is a warning message.");
    // This DEBUG message will not be printed.
    LOG(CustomerLogLevel::DEBUG, "This is a debug message and should NOT appear.");
    std::cout << std::endl;


    // --- 2. Change Log Level ---
    std::cout << "--- 2. Setting Log Level to DEBUG using custom enum ---" << std::endl;
    Logger::getInstance().setLevel(CustomerLogLevel::DEBUG);
    LOG(CustomerLogLevel::DEBUG, "This debug message should now appear.");
    LOG(CustomerLogLevel::INFO, "This info message should also appear.");
    LOG(CustomerLogLevel::WARN, "This warning message should also appear.");
    std::cout << std::endl;


    // --- 3. NEW: Exact Level Filtering ---
    std::cout << "--- 3. NEW: Exact Level Filtering - Only WARN messages ---" << std::endl;
    Logger::getInstance().setExactLevel(CustomerLogLevel::WARN); // WARN only
    LOG(CustomerLogLevel::DEBUG, "This debug message should NOT appear.");
    LOG(CustomerLogLevel::INFO, "This info message should NOT appear.");
    LOG(CustomerLogLevel::WARN, "This warning message SHOULD appear.");
    LOG(CustomerLogLevel::ERROR, "This error message should NOT appear.");
    std::cout << std::endl;


    // --- 4. Switch to another exact level ---
    std::cout << "--- 4. Exact Level Filtering - Only ERROR messages ---" << std::endl;
    Logger::getInstance().setExactLevel(CustomerLogLevel::ERROR); // ERROR only
    LOG(CustomerLogLevel::DEBUG, "This debug message should NOT appear.");
    LOG(CustomerLogLevel::INFO, "This info message should NOT appear.");
    LOG(CustomerLogLevel::WARN, "This warning message should NOT appear.");
    LOG(CustomerLogLevel::ERROR, "This error message SHOULD appear.");
    std::cout << std::endl;


    // --- 5. Clear exact level and return to minimum level filtering ---
    std::cout << "--- 5. Clearing exact level - back to minimum level filtering ---" << std::endl;
    Logger::getInstance().clearExactLevel();
    Logger::getInstance().setLevel(CustomerLogLevel::INFO); // INFO and above
    LOG(CustomerLogLevel::DEBUG, "This debug message should NOT appear.");
    LOG(CustomerLogLevel::INFO, "This info message should appear.");
    LOG(CustomerLogLevel::WARN, "This warning message should appear.");
    LOG(CustomerLogLevel::ERROR, "This error message should appear.");
    std::cout << std::endl;


    // --- 6. Log to a File with exact level ---
    std::cout << "--- 6. Logging to a file with exact level filtering ---" << std::endl;
    Logger::getInstance().setOutputToFile("my_app.log");
    Logger::getInstance().setExactLevel(CustomerLogLevel::INFO); // INFO only
    LOG(CustomerLogLevel::INFO, "This INFO message is being written to the log file.");
    LOG(CustomerLogLevel::ERROR, "This ERROR message should NOT be written to the file.");
    std::cout << "Check the 'my_app.log' file - should only contain INFO messages." << std::endl;
    std::cout << std::endl;

    // --- 7. Use a Custom Formatter with exact level ---
    std::cout << "--- 7. Using a Custom Formatter with exact level filtering ---" << std::endl;
    Logger::getInstance().setFormatter([](const LogMessage& msg) {
        std::stringstream ss;
        ss << msg.levelName << " :: " << msg.message;
        return ss.str();
    });
    LOG(CustomerLogLevel::INFO, "This info message uses the custom format and exact level.");
    LOG(CustomerLogLevel::WARN, "This warning should NOT appear due to exact level filtering.");
    std::cout << "Check the log file again to see the new format with exact level filtering." << std::endl;
    std::cout << std::endl;
    
    // --- 8. Switch back to stdout ---
    std::cout << "--- 8. Switching back to stdout with exact level filtering ---" << std::endl;
    Logger::getInstance().setOutputToStdout();
    LOG(CustomerLogLevel::INFO, "This message is now on the console with exact level filtering.");
    LOG(CustomerLogLevel::DEBUG, "This debug message should NOT appear.");
    std::cout << std::endl;


    
     Logger::getInstance().setLevel(CustomerLogLevel::DEBUG);
     Logger::getInstance().clearExactLevel();
     Logger::getInstance().setDefaultFormatter();
     // --- 9. NEW: Test notInclude() - Exclude INFO level ---
    std::cout << "--- 9. NEW: Excluding INFO level (should show DEBUG, WARN, ERROR) ---" << std::endl;
    Logger::getInstance().notInclude(CustomerLogLevel::INFO);
    LOG(CustomerLogLevel::DEBUG, "DEBUG message - should appear");
    LOG(CustomerLogLevel::INFO, "INFO message - should NOT appear (excluded)");
    LOG(CustomerLogLevel::WARN, "WARN message - should appear");
    LOG(CustomerLogLevel::ERROR, "ERROR message - should appear");
    std::cout << std::endl;

    // --- 10. NEW: Test multiple exclusions ---
    std::cout << "--- 10. NEW: Excluding both INFO and WARN levels ---" << std::endl;
    Logger::getInstance().notInclude(CustomerLogLevel::WARN); // Now excluding both INFO and WARN
    LOG(CustomerLogLevel::DEBUG, "DEBUG message - should appear");
    LOG(CustomerLogLevel::INFO, "INFO message - should NOT appear (excluded)");
    LOG(CustomerLogLevel::WARN, "WARN message - should NOT appear (excluded)");
    LOG(CustomerLogLevel::ERROR, "ERROR message - should appear");
    std::cout << std::endl;

    // --- 11. NEW: Test includeBack() - Bring back INFO level ---
    std::cout << "---11. NEW: Including INFO level back (WARN still excluded) ---" << std::endl;
    Logger::getInstance().includeBack(CustomerLogLevel::INFO);
    LOG(CustomerLogLevel::DEBUG, "DEBUG message - should appear");
    LOG(CustomerLogLevel::INFO, "INFO message - should appear (included back)");
    LOG(CustomerLogLevel::WARN, "WARN message - should NOT appear (still excluded)");
    LOG(CustomerLogLevel::ERROR, "ERROR message - should appear");
    std::cout << std::endl;

    // ---12. NEW: Test exclusions with minimum level filtering ---
    std::cout << "--- 12. NEW: Minimum level INFO + excluding WARN ---" << std::endl;
    Logger::getInstance().setLevel(CustomerLogLevel::INFO); // Only INFO and above
    // WARN is still excluded from previous test
    LOG(CustomerLogLevel::DEBUG, "DEBUG message - should NOT appear (below min level)");
    LOG(CustomerLogLevel::INFO, "INFO message - should appear");
    LOG(CustomerLogLevel::WARN, "WARN message - should NOT appear (excluded)");
    LOG(CustomerLogLevel::ERROR, "ERROR message - should appear");
    std::cout << std::endl;

    // --- 13. NEW: Test exclusions with exact level filtering ---
    std::cout << "--- 13. Reset to default ---" << std::endl;
    Logger::getInstance().setDefault();
    LOG(CustomerLogLevel::INFO, "This is an info message.");
    LOG(CustomerLogLevel::WARN, "This is a warning message.");
    // This DEBUG message will not be printed.
    LOG(CustomerLogLevel::DEBUG, "This is a debug message and should NOT appear.");
    std::cout << std::endl;
}