#ifndef PARAMETER_CHECK_H
#define PARAMETER_CHECK_H

//
// Parameter validation meant to be injected by tools/trace_injector:
//
//     void Strategy::OnTick(const TradeData& tick, int window)
//     {
//         check_all("Strategy::OnTick", {"tick", "window"}, tick, window);
//         ...
//     }
//
// Two rules follow from being injected rather than hand-written.
//
// The first is that an unknown type must not break the build. Injected across
// hundreds of functions, the first parameter of a type nobody registered would
// otherwise fail to compile at a line nobody wrote. An unregistered type is
// therefore not checked at all: the failure mode is a check that did not
// happen, which the reader can find, rather than a build that will not start.
//
// The second is that a check nobody believes is worse than no check. Only what
// is true of every value of a type is asserted here — a null pointer is never
// meaningful, a NaN is never a price. "Must be positive" is a rule about a
// particular quantity, not about int, and belongs in a registration.
//
// The return value is deliberately easy to ignore: check_all logs, and what to
// do about a bad parameter is the caller's business. Injected code cannot
// return early for you — the return type varies and there is no correct value
// to invent.
//

#include <cmath>
#include <cstddef>
#include <initializer_list>
#include <type_traits>
#include <utility>

#include "Logger.h"

//
// CustomerLogLevel::WARN, which is 7 in src/Types.h. A number rather than the
// enum so this header knows nothing of the project's own types; define it on
// the command line for a tree whose level map differs.
//
#ifndef PARAMETER_CHECK_LEVEL
#define PARAMETER_CHECK_LEVEL 7
#endif

//
// Where a type says what a valid value of it looks like. Specialise beside the
// type's own definition, not here — this header is meant to know no domain:
//
//     template<>
//     struct check_traits<TradeData> : std::true_type
//     {
//         static bool valid(const TradeData& v)
//         {
//             return v.price_ > 0.0 && v.timestamp_ms_ > 0;
//         }
//     };
//
// The unused second parameter is for partial specialisations that want to match
// a whole family of types by SFINAE rather than one type at a time.
//
template<typename T, typename = void>
struct check_traits : std::false_type
{
};

template<typename T>
inline constexpr bool has_check_v = check_traits<T>::value;

//
// What is asserted about a value with no registration of its own. A registered
// type is asked first, so a registration always wins over the generic rules.
//
template<typename T>
bool default_check(const T& value)
{
    if constexpr (has_check_v<T>)
    {
        return check_traits<T>::valid(value);
    }
    else if constexpr (std::is_pointer_v<T>)
    {
        return value != nullptr;
    }
    else if constexpr (std::is_floating_point_v<T>)
    {
        //
        // Finite, and nothing more. Zero and negative are ordinary values of a
        // price delta or a P&L.
        //
        return std::isfinite(value);
    }
    else
    {
        (void)value;
        return true;
    }
}

template<typename T>
bool check_one_param(
    const char* caller,
    const char* param_name,
    const T& value
)
{
    if (default_check(value))
    {
        return true;
    }

    //
    // LogStream rather than the LOG macro, with `caller` where the macro would
    // put __func__: the file and line are this header's either way, so the one
    // field that can name the function under check is spent on doing so.
    //
    LogStream(PARAMETER_CHECK_LEVEL, __FILE__, caller, __LINE__)
        << "Parameter \""
        << param_name
        << "\" is invalid";

    return false;
}

//
// One index per parameter, so a name is never read past the end of the list and
// nothing is mutated while the pack expands.
//
template<std::size_t... Index, typename... Args>
bool check_all_impl(
    const char* caller,
    const char* const* names,
    std::index_sequence<Index...>,
    const Args&... args
)
{
    //
    // A function taking nothing expands to no calls at all, which leaves both
    // of these unread.
    //
    (void)caller;
    (void)names;

    //
    // & and not &&, deliberately: every bad parameter has to be reported, and
    // && would stop at the first one. Do not "fix" this.
    //
    // The operands of & are evaluated in an unspecified order, which is fine
    // here only because each call is independent — hence the index rather than
    // a pointer being walked along.
    //
    return (true & ... & check_one_param(caller, names[Index], args));
}

template<typename... Args>
bool check_all(
    const char* caller,
    std::initializer_list<const char*> names,
    const Args&... args
)
{
    //
    // A mismatch is a caller that has drifted from its own signature, which
    // used to be a read off the end of the names array. Nothing is checked in
    // that case: names paired with the wrong values are worse than none.
    //
    if (names.size() != sizeof...(args))
    {
        LogStream(PARAMETER_CHECK_LEVEL, __FILE__, caller, __LINE__)
            << "check_all was given "
            << names.size()
            << " names for "
            << sizeof...(args)
            << " parameters";

        return false;
    }

    return check_all_impl(
        caller,
        names.begin(),
        std::index_sequence_for<Args...>{},
        args...
    );
}

#endif // PARAMETER_CHECK_H
