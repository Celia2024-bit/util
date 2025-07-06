#ifndef PARAMETER_CHECK_H
#define PARAMETER_CHECK_H

#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <type_traits>
#include "../Types.h"

// Helper to get current timestamp string
inline std::string current_timestamp() {
    std::time_t now = std::time(nullptr);
    char buf[64];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    return std::string(buf);
}

template<typename T>
bool default_check(const T& value) {
    if constexpr (std::is_integral_v<T>) {
        return value > 0;
    } else if constexpr (std::is_pointer_v<T>) {
        return value != nullptr;
    } else if constexpr (std::is_same_v<T, IntRange>) {
        return value.isValid();
    } else {
        static_assert(std::is_same_v<T, void>,
                      "default_check: Unsupported type, please add a type branch!");
        return true; // This line never executes, for syntax completeness
    }
}


template<typename T>
bool check_one_param(const char* caller, const char* param_name, const T& value) {
    bool result = default_check(value);
    if (!result) {
        std::string msg = "[" + current_timestamp() + "] "
                          "Error in function \"" + caller + "\" - "
                          "Parameter \"" + param_name + "\" is invalid. ";
        // Print to console
        std::cout << msg << std::endl;

        // Append to log file
        std::ofstream log_file("parameter_check.log", std::ios::app);
        if (log_file.is_open()) {
            log_file << msg << std::endl;
            log_file.close();
        } else {
            std::cerr << "Failed to open log file for writing." << std::endl;
        }
    }
    return result;
}

// --- Recursive Implementation Detail ---
inline bool check_all_impl(const char** names, size_t current_idx, const char* caller) {
    (void)names;
    (void)current_idx;
	(void)caller;
    return true; // All parameters processed
}

// Recursive variadic template to check parameters with names and caller info
template<typename First, typename... Rest>
bool check_all_impl(const char** names, size_t current_idx, const char* caller, const First& first, const Rest&... rest) {
    bool current_result = check_one_param(caller, names[current_idx], first);
    bool rest_result = check_all_impl(names, current_idx + 1, caller, rest...);
    return current_result && rest_result;
}

// User-facing check_all with names and caller
template<typename... Args>
bool check_all(const char* caller, const char** names, const Args&... args) {
    return check_all_impl(names, 0, caller, args...);
}

// User-facing check_all without names (uses dummy names), with caller
template<typename... Args>
bool check_all(const char* caller, const Args&... args) {
    constexpr size_t count = sizeof...(args);
    // Ensure the number of parameters doesn't exceed the dummy_names array size
    static_assert(count <= 10, "check_all supports up to 10 parameters only");

    // Dummy names array must be fixed-size based on the maximum supported count (10)
    static const char* dummy_names[10] = {
        "param1", "param2", "param3", "param4", "param5",
        "param6", "param7", "param8", "param9", "param10"
    };

    // Calls the user-facing check_all that accepts names
    return check_all(caller,dummy_names, args...);
}

#endif // PARAMETER_CHECK_H
