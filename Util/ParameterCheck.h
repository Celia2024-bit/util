#ifndef PARAMETER_CHECK_H
#define PARAMETER_CHECK_H

#include <iostream> 
#include "../Types.h" 


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
bool check_one_param(const char* name, const T& value) {
    bool result = default_check(value);
    if (!result) {
        std::cout << "Parameter \"" << name << "\" is invalid." << std::endl;
    }
    return result;
}

// --- Recursive Implementation Detail ---
bool check_all_impl(const char** names, size_t current_idx) {
    (void)names;
    (void)current_idx;
    return true; // All parameters processed
}

template<typename First, typename... Rest>
bool check_all_impl(const char** names, size_t current_idx, const First& first, const Rest&... rest) {
    bool current_result = check_one_param(names[current_idx], first);
    // Recursively call with the next index and the rest of the parameters
    bool rest_result = check_all_impl(names, current_idx + 1, rest...);
    return current_result && rest_result;
}

// --- User-Facing check_all Functions ---
template<typename... Args>
bool check_all(const char** names, const Args&... args) {
    // Calls the recursive implementation with initial index 0
    return check_all_impl(names, 0, args...);
}

// This is the function users call when they don't want to specify parameter names.
template<typename... Args>
bool check_all(const Args&... args) {
    constexpr size_t count = sizeof...(args);
    // Ensure the number of parameters doesn't exceed the dummy_names array size
    static_assert(count <= 10, "check_all supports up to 10 parameters only for dummy names.");

    // Dummy names array must be fixed-size based on the maximum supported count (10)
    static const char* dummy_names[10] = {
        "param1", "param2", "param3", "param4", "param5",
        "param6", "param7", "param8", "param9", "param10"
    };

    // Calls the user-facing check_all that accepts names
    return check_all(dummy_names, args...);
}

#endif // PARAMETER_CHECK_H
