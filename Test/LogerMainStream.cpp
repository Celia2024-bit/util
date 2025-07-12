#include "../Logger.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <iomanip>

// Custom log levels enum
enum CustomerLogLevel 
{  
    Main = 1, 
    MarketData,
    Strategy, 
    Execution,
    DEBUG,
    INFO,
    WARN,
    ERROR
};

// Custom mappings
LevelMapping customMappings = {
    {Main,        "Main:"},
    {MarketData,  "Market Data"},
    {Strategy,    "Strategy:"},
    {Execution,   "Execution:"},
    {DEBUG,       "DEBUG"},
    {INFO,        "INFO"},
    {WARN,        "WARN"},
    {ERROR,       "ERROR"}
};

void testBasicLogging() {
    std::cout << "\n=== Testing Basic Logging ===" << std::endl;
    
    // Test simple string logging
    LOG(Main) << "Application started";
    LOG(MarketData) << "Market data connection established";
    LOG(Strategy) << "Strategy engine initialized";
    LOG(Execution) << "Trade executor ready";
    
    // Test different log levels
    LOG(DEBUG) << "Debug message";
    LOG(INFO) << "Info message";
    LOG(WARN) << "Warning message";
    LOG(ERROR) << "Error message";
}

void testVariableLogging() {
    std::cout << "\n=== Testing Variable Logging ===" << std::endl;
    
    // Test various data types
    int count = 42;
    double price = 65432.789;
    std::string symbol = "BTC";
    bool isActive = true;
    
    LOG(MarketData) << "Symbol: " << symbol << ", Price: $" << std::fixed << std::setprecision(2) << price;
    LOG(Strategy) << "Processing " << count << " signals";
    LOG(Execution) << "Account status: " << (isActive ? "Active" : "Inactive");
    
    // Test complex formatting
    LOG(Main) << "Trading pair: " << symbol << "/USD"
              << ", Current price: $" << std::fixed << std::setprecision(2) << price
              << ", Volume: " << count << " units"
              << ", Status: " << std::boolalpha << isActive;
}

void testStreamManipulators() {
    std::cout << "\n=== Testing Stream Manipulators ===" << std::endl;
    
    double values[] = {123.456789, 0.001234, 9876.54321};
    
    for (int i = 0; i < 3; ++i) {
        LOG(MarketData) << "Value " << i << ": " 
                       << std::fixed << std::setprecision(2) << values[i];
        
        LOG(Strategy) << "Scientific notation: " 
                     << std::scientific << std::setprecision(3) << values[i];
        
        LOG(Execution) << "Hexadecimal: " 
                      << std::hex << static_cast<int>(values[i]) << std::dec;
    }
}

void testLongMessages() {
    std::cout << "\n=== Testing Long Messages ===" << std::endl;
    
    LOG(Main) << "This is a very long message that contains multiple pieces of information: "
              << "timestamp=" << std::chrono::duration_cast<std::chrono::seconds>(
                     std::chrono::system_clock::now().time_since_epoch()).count()
              << ", user_id=12345, session_token=abc123xyz, action=login, "
              << "ip_address=192.168.1.100, user_agent=Mozilla/5.0";
}

void testThreadSafety() {
    std::cout << "\n=== Testing Thread Safety ===" << std::endl;
    
    auto workerThread = [](int threadId) {
        for (int i = 0; i < 5; ++i) {
            LOG(Main) << "Thread " << threadId << " - Message " << i;
            LOG(MarketData) << "Thread " << threadId << " - Market update " << i;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    };
    
    std::thread t1(workerThread, 1);
    std::thread t2(workerThread, 2);
    std::thread t3(workerThread, 3);
    
    t1.join();
    t2.join();
    t3.join();
}

void testLogLevels() {
    std::cout << "\n=== Testing Log Level Filtering ===" << std::endl;
    
    // Test minimum level filtering
    std::cout << "Setting minimum level to Strategy (3):" << std::endl;
    Logger::getInstance().setLevel(Strategy);
    
    LOG(Main) << "This should NOT appear (level 1)";
    LOG(MarketData) << "This should NOT appear (level 2)";
    LOG(Strategy) << "This SHOULD appear (level 3)";
    LOG(Execution) << "This SHOULD appear (level 4)";
    
    // Test exact level filtering
    std::cout << "Setting exact level to MarketData (2):" << std::endl;
    Logger::getInstance().setExactLevel(MarketData);
    
    LOG(Main) << "This should NOT appear (level 1)";
    LOG(MarketData) << "This SHOULD appear (level 2)";
    LOG(Strategy) << "This should NOT appear (level 3)";
    LOG(Execution) << "This should NOT appear (level 4)";
    
    // Reset to default
    Logger::getInstance().setDefault();
}

void testExclusions() {
    std::cout << "\n=== Testing Level Exclusions ===" << std::endl;
    
    // Exclude DEBUG and INFO levels
    Logger::getInstance().notInclude(DEBUG);
    Logger::getInstance().notInclude(INFO);
    
    LOG(DEBUG) << "This should NOT appear (excluded)";
    LOG(INFO) << "This should NOT appear (excluded)";
    LOG(WARN) << "This SHOULD appear (not excluded)";
    LOG(ERROR) << "This SHOULD appear (not excluded)";
    
    // Include back INFO level
    Logger::getInstance().includeBack(INFO);
    LOG(INFO) << "This SHOULD appear (included back)";
    
    // Clear all exclusions
    Logger::getInstance().clearExclusions();
    LOG(DEBUG) << "This SHOULD appear (exclusions cleared)";
}

void testFileOutput() {
    std::cout << "\n=== Testing File Output ===" << std::endl;
    
    // Redirect to file
    Logger::getInstance().setOutputToFile("test_log.txt");
    
    LOG(Main) << "This message goes to file";
    LOG(MarketData) << "File logging test with numbers: " << 123 << " and " << 45.67;
    
    // Back to console
    Logger::getInstance().setOutputToStdout();
    LOG(Main) << "Back to console output";
    
    std::cout << "Check 'test_log.txt' file for file output results." << std::endl;
}

void testEdgeCases() {
    std::cout << "\n=== Testing Edge Cases ===" << std::endl;
    
    // Empty message
    LOG(Main) << "";
    
    // Only manipulators
    LOG(MarketData) << std::fixed << std::setprecision(2);
    
    // Special characters
    LOG(Strategy) << "Special chars: !@#$%^&*()_+-=[]{}|;':\",./<>?";
    
    // Unicode (if supported)
    LOG(Execution) << "Unicode test: ñáéíóú αβγδε 中文测试";
    
    // Very long number
    LOG(DEBUG) << "Large number: " << 1234567890123456789LL;
    
    // Null pointer (careful!)
    const char* nullPtr = nullptr;
    LOG(WARN) << "Null pointer test: " << (nullPtr ? nullPtr : "NULL");
}

void performanceTest() {
    std::cout << "\n=== Performance Test ===" << std::endl;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // Log 1000 messages
    for (int i = 0; i < 1000; ++i) {
        LOG(Main) << "Performance test message " << i << " with value " << i * 3.14159;
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Logged 1000 messages in " << duration.count() << " ms" << std::endl;
}

int main() {
    std::cout << "Starting Logger Tests..." << std::endl;
    
    // Initialize logger with custom mappings
    LOGINIT(customMappings);
    
    // Run all tests
    testBasicLogging();
    testVariableLogging();
    testStreamManipulators();
    testLongMessages();
    testThreadSafety();
    testLogLevels();
    testExclusions();
    testFileOutput();
    testEdgeCases();
    performanceTest();
    
    std::cout << "\n=== All Tests Completed ===" << std::endl;
    
    return 0;
}