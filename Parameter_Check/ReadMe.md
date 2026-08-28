# Header Type Validator & Parameter Check Tool

A static code analysis and runtime parameter validation utility for C++ applications.

This tool automatically verifies that custom C++ types (`struct`, `class`, `enum`) are properly equipped with validity checks before compilation, helping prevent invalid data propagation, bugs, and undefined behavior in C++ pipelines.

## 1. Quick Start / Usage

Run `validate_types.py` to statically analyze C++ header files and ensure all declared types conform to validation standards:

```bash
# Validate a compliant header file (Expected: Types validation passed)
python validate_types.py Types_examples/Types.h

# Validate a non-compliant header file (Expected: Validation Failed)
python validate_types.py Types_examples/TypesInvalid.h
```

## 2. Header Include Configuration (`ParameterCheck.h`)

`ParameterCheck.h` contains generic parameter validation logic and **does not manage file paths** for your type definition files.

- **Default Header Name**: `ParameterCheck.h` includes `"Types.h"` by default (`#include "Types.h"`).

- **Include Paths**: Ensure your build system (e.g., CMake `target_include_directories` or Makefile `-I` flags) includes the directory containing `Types.h`.

- **Custom Header Names**: If your project uses a different filename (e.g., `MyTypes.h`), simply update the `#include` directive in `ParameterCheck.h`:



```cpp
// ParameterCheck.h
#include "CheckTraits.h"
#include "Types.h"  // Change "Types.h" here if your project uses a different header name
```

## 3. Type Preparation Requirements

For a custom type to pass static analysis and work seamlessly with `ParameterCheck.h`, it must satisfy **at least one** of the following three rules:

1. **Member Method**: Implements a `bool isValid() const` method on the `struct`/`class`.

2. **Container-like Type**: Implements an `empty()` method (e.g., standard containers like `std::vector`).

3. **Traits Specialization**: Provides a `check_traits<T>` specialization placed alongside the type definition (required for `enum` types or unmodifiable structures).
