//
// Self-checking test for util/ParameterCheck.h. Build and run:
//
//     make test_param_check && ./util/Test/test_param_check.exe
//
// Exit code 0 means every check passed. Nothing from src/ is used on purpose:
// the header under test knows no domain types, and neither does this file.
//
#include <cmath>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

#include "../ParameterCheck.h"

namespace
{

//
// A type with a rule of its own, registered the way a real one would be beside
// its own definition.
//
struct Bounded
{
    int value;
    int low;
    int high;
};

}  // namespace

template<>
struct check_traits<Bounded> : std::true_type
{
    static bool valid(const Bounded& b)
    {
        return b.value >= b.low && b.value <= b.high;
    }
};

namespace
{

//
// A type with no registration at all. The point of it is that it compiles.
//
struct Unregistered
{
    int whatever;
};

std::vector<std::string> captured;

int failures = 0;

void check(const char* what, bool ok)
{
    std::cout << (ok ? "PASS  " : "FAIL  ") << what << std::endl;

    if (!ok)
    {
        ++failures;
    }
}

//
// Runs `body` with the log captured rather than printed, and hands back what it
// wrote. The default formatter's timestamp is not under test, so it is replaced.
//
template<typename Body>
std::vector<std::string> record(Body body)
{
    captured.clear();

    Logger::getInstance().setFormatter(
        [](const LogMessage& msg)
        {
            captured.push_back(msg.function + std::string(": ") + msg.message);
            return std::string();
        }
    );

    std::ostringstream sink;
    std::streambuf* saved = std::cout.rdbuf(sink.rdbuf());

    body();

    std::cout.rdbuf(saved);

    Logger::getInstance().setDefaultFormatter();

    return captured;
}

bool mentions(const std::vector<std::string>& lines, const std::string& needle)
{
    for (const std::string& line : lines)
    {
        if (line.find(needle) != std::string::npos)
        {
            return true;
        }
    }

    return false;
}

void test_registered_type()
{
    const Bounded good{7, 3, 10};
    const Bounded bad{12, 3, 10};

    bool ok = true;
    bool wrong = true;

    const std::vector<std::string> lines = record(
        [&]()
        {
            ok = check_all("Fixture::Good", {"good"}, good);
            wrong = check_all("Fixture::Bad", {"bad"}, bad);
        }
    );

    check(
        "registered: a value inside its bounds passes",
        ok
    );

    check(
        "registered: a value outside them does not",
        !wrong
    );

    check(
        "registered: the log names the parameter and the function",
        mentions(lines, "Fixture::Bad: Parameter \"bad\" is invalid")
        && !mentions(lines, "Fixture::Good")
    );
}

void test_generic_rules()
{
    int value = 5;
    int* pointer = &value;
    int* null_pointer = nullptr;

    bool ok = true;
    bool wrong = true;
    bool nan_result = true;

    record(
        [&]()
        {
            ok = check_all("Fixture::Pointers", {"pointer"}, pointer);
            wrong = check_all("Fixture::Pointers", {"null"}, null_pointer);
            nan_result = check_all(
                "Fixture::Doubles",
                {"nan", "infinite", "ordinary"},
                std::nan(""),
                std::numeric_limits<double>::infinity(),
                -1.5
            );
        }
    );

    check(
        "generic: a pointer with something behind it passes",
        ok
    );

    check(
        "generic: a null one does not",
        !wrong
    );

    check(
        "generic: a NaN or an infinity does not, a negative double does",
        !nan_result
    );
}

void test_no_false_positives()
{
    bool ok = false;

    const std::vector<std::string> lines = record(
        [&]()
        {
            //
            // The check that used to be here, and was the reason to change
            // this file: `value > 0` for every integral type flagged a zero
            // count, a negative delta, a false flag, and a '\0'.
            //
            ok = check_all(
                "Fixture::Ordinary",
                {"zero", "negative", "flag", "letter"},
                0,
                -3,
                false,
                'x'
            );
        }
    );

    check(
        "no false positives: zero, negative, false and a char all pass",
        ok
    );

    check(
        "no false positives: and nothing was logged about them",
        lines.empty()
    );
}

void test_unregistered_type()
{
    const Unregistered thing{0};

    bool ok = false;

    record(
        [&]()
        {
            ok = check_all("Fixture::Unknown", {"thing"}, thing);
        }
    );

    //
    // The assertion that matters here is the one the compiler made: a type with
    // no registration used to be a static_assert, i.e. a broken build at a line
    // the injector wrote.
    //
    check(
        "unregistered: a type nobody registered is passed over, not rejected",
        ok
    );
}

void test_name_count_mismatch()
{
    int value = 5;

    bool ok = true;

    const std::vector<std::string> lines = record(
        [&]()
        {
            ok = check_all("Fixture::Drifted", {"one", "two"}, value);
        }
    );

    check(
        "mismatch: fewer parameters than names fails",
        !ok
    );

    check(
        "mismatch: and says so, naming the function",
        mentions(lines, "Fixture::Drifted: check_all was given 2 names for 1")
    );
}

void test_every_bad_parameter_reported()
{
    const Bounded bad{12, 3, 10};

    int* null_pointer = nullptr;

    const std::vector<std::string> lines = record(
        [&]()
        {
            check_all(
                "Fixture::Several",
                {"first", "second", "third"},
                bad,
                null_pointer,
                bad
            );
        }
    );

    //
    // Three lines, not one: the fold is written with & rather than && exactly
    // so that a second bad parameter is not hidden by the first.
    //
    check(
        "reporting: every bad parameter gets its own line",
        lines.size() == 3
        && mentions(lines, "\"first\"")
        && mentions(lines, "\"second\"")
        && mentions(lines, "\"third\"")
    );
}

void test_no_parameters()
{
    bool ok = false;

    record(
        [&]()
        {
            ok = check_all("Fixture::Nothing", {});
        }
    );

    check(
        "empty: a function taking nothing is vacuously fine",
        ok
    );
}

}  // namespace

int main()
{
    test_registered_type();
    test_generic_rules();
    test_no_false_positives();
    test_unregistered_type();
    test_name_count_mismatch();
    test_every_bad_parameter_reported();
    test_no_parameters();

    std::cout << std::endl
              << (failures == 0
                  ? std::string("all checks passed")
                  : "checks failed: " + std::to_string(failures))
              << std::endl;

    return failures == 0 ? 0 : 1;
}
