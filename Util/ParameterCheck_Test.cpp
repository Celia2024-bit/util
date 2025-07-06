#include <iostream>
#include "ParameterCheck.h"  // Make sure this header contains your parameter checking implementation

// Simple unit test helper to print test results
void run_test(const char* test_name, bool result) {
    std::cout << test_name << ": " << (result ? "PASS" : "FAIL") << std::endl;
}

int main() {
    // Valid and invalid test values
    int a = 5;
    int b = -3;              // Invalid integral (<= 0)
    int* p = &a;
    int* null_ptr = nullptr; // Invalid pointer

    IntRange validRange(7, 3, 10);     // x=7 in [3,10], valid
    IntRange invalidRange(12, 3, 10);  // x=12 out of range, invalid

    // Test 1: all valid parameters
    std::cout <<"Test 1 Starts" << std::endl;
    bool test1 = check_all("test1", a, p, validRange);
    run_test("Test 1 (all valid)", test1);
    std::cout << std::endl;

    // Test 2: invalid integral parameter
    std::cout <<"Test 2 Starts" << std::endl;
    bool test2 = check_all("test2", b, p, validRange);
    run_test("Test 2 (invalid integral)", !test2);
    std::cout << std::endl;

    // Test 3: invalid pointer parameter
    std::cout <<"Test 3 Starts" << std::endl;
    bool test3 = check_all("test3", a, null_ptr, validRange);
    run_test("Test 3 (invalid pointer)", !test3);
    std::cout << std::endl;

    // Test 4: invalid IntRange parameter
    std::cout <<"Test 4 Starts" << std::endl;
    bool test4 = check_all("test4", a, p, invalidRange);
    run_test("Test 4 (invalid IntRange)", !test4);
    std::cout << std::endl;

    // Test 5: multiple invalid parameters
    std::cout <<"Test 5 Starts" << std::endl;
    bool test5 = check_all("test4", b, null_ptr, invalidRange);
    run_test("Test 5 (multiple invalid)", !test5);
    std::cout << std::endl;

    return 0;
}
