//
// Self-checking test for util/ScopeTrace.h. Build and run:
//
//     make test_scope_trace && ./util/Test/test_scope_trace.exe
//
// Exit code 0 means every check passed. Nothing is asserted about the
// timestamp or the level name — the formatter is replaced for the duration of
// the run so that only what ScopeTrace itself decides is under test.
//
#include <exception>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "../ScopeTrace/ScopeTrace.h"

namespace
{

std::vector<LogMessage> captured;

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
// The traced functions. Each takes its own name as a literal, the way the
// injector writes it.
//
void Inner()
{
    ScopeTrace trace(__FILE__, __LINE__, "Nest::Inner");
}

void Outer()
{
    ScopeTrace trace(__FILE__, __LINE__, "Nest::Outer");

    Inner();
}

//
// Everything logged while `body` runs, in order.
//
std::vector<LogMessage> record(void (*body)())
{
    captured.clear();

    Logger::getInstance().setFormatter(
        [](const LogMessage& msg)
        {
            captured.push_back(msg);
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

//
// From here to the end of the checks, only the enabled build has anything to
// say — every assertion below is about a line that is deliberately not written
// when tracing is switched off.
//
#if SCOPE_TRACE_ENABLED

bool contains(const std::string& haystack, const std::string& needle)
{
    return haystack.find(needle) != std::string::npos;
}

int EarlyReturn(bool leave)
{
    ScopeTrace trace(__FILE__, __LINE__, "Nest::EarlyReturn");

    if (leave)
    {
        return 1;
    }

    return 2;
}

void Throws()
{
    ScopeTrace trace(__FILE__, __LINE__, "Nest::Throws");

    throw std::runtime_error("deliberate");
}

void test_nesting()
{
    const std::vector<LogMessage> lines = record(&Outer);

    check(
        "nesting: four lines, in and out of both",
        lines.size() == 4
    );

    if (lines.size() != 4)
    {
        return;
    }

    check(
        "nesting: outer enters first, at no indent",
        lines[0].message == "-> Nest::Outer"
    );

    check(
        "nesting: inner is indented by its depth",
        lines[1].message == "  -> Nest::Inner"
    );

    check(
        "nesting: inner leaves before outer, still indented",
        contains(lines[2].message, "  <- Nest::Inner")
    );

    check(
        "nesting: outer leaves last, back at no indent",
        lines[3].message.rfind("<- Nest::Outer", 0) == 0
    );
}

void test_elapsed()
{
    const std::vector<LogMessage> lines = record(&Inner);

    check(
        "elapsed: the exit line carries a duration",
        lines.size() == 2 && contains(lines[1].message, " us)")
    );

    check(
        "elapsed: the entry line does not",
        lines.size() == 2 && !contains(lines[0].message, " us)")
    );
}

void test_location()
{
    const std::vector<LogMessage> lines = record(&Inner);

    check(
        "location: the traced file is reported, not ScopeTrace.h",
        lines.size() == 2 && !contains(lines[0].file, "ScopeTrace.h")
    );

    check(
        "location: the function is the name that was passed in",
        lines.size() == 2 && lines[0].function == std::string("Nest::Inner")
    );

    check(
        "location: the line is the guard's own, not the header's",
        lines.size() == 2 && lines[0].line > 0
    );
}

void test_early_return()
{
    const std::vector<LogMessage> lines = record(
        []()
        {
            EarlyReturn(true);
        }
    );

    check(
        "early return: the exit is logged anyway",
        lines.size() == 2 && contains(lines[1].message, "<- Nest::EarlyReturn")
    );

    check(
        "early return: and it is not marked as an exception",
        lines.size() == 2 && !contains(lines[1].message, "[exception]")
    );
}

void test_exception()
{
    const std::vector<LogMessage> lines = record(
        []()
        {
            try
            {
                Throws();
            }
            catch (const std::exception&)
            {
            }
        }
    );

    check(
        "exception: the unwound scope says so",
        lines.size() == 2 && contains(lines[1].message, "[exception]")
    );
}

void test_depth_restored()
{
    //
    // Run after the exception test on purpose: an exit that skipped its
    // decrement would show up here as an indented top-level call.
    //
    const std::vector<LogMessage> lines = record(&Inner);

    check(
        "depth: back to zero after everything above",
        lines.size() == 2 && lines[0].message == "-> Nest::Inner"
    );
}

#endif

}  // namespace

int main()
{
#if SCOPE_TRACE_ENABLED

    test_nesting();
    test_elapsed();
    test_location();
    test_early_return();
    test_exception();
    test_depth_restored();

#else

    //
    // Built with -DSCOPE_TRACE_ENABLED=0. That the file compiles at all is
    // half the check; the other half is that the guards went quiet.
    //
    check(
        "disabled: a guard logs nothing",
        record(&Outer).empty()
    );

#endif

    std::cout << std::endl
              << (failures == 0 ? "all checks passed" : "checks failed: ")
              << (failures == 0 ? "" : std::to_string(failures))
              << std::endl;

    return failures == 0 ? 0 : 1;
}
