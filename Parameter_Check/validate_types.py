#!/usr/bin/env python3
"""
Static, regex-based validation for a Types.h header.

This module owns ONLY the question "is this Types.h prepared?" - it does not
render or write anything. generate_parameter_check.py imports
check_types_header() from here instead of duplicating the logic, so the two
concerns (checking vs. generating) can change independently.
"""
import argparse
import re
import sys
from pathlib import Path

#
# Code, not prose. Every pattern below is looking for a declaration, and a
# comment is neither -- but it is made of the same words, so a sentence like
#
#     // An enum cannot carry isValid(), so it is prepared from the outside.
#
# used to be read as declaring an enum named "cannot" and reported as an
# unprepared type. Documenting your types should not fail validation.
#
COMMENT_RE = re.compile(
    r'//[^\n]*|/\*.*?\*/',
    re.DOTALL
)


def strip_comments(content: str) -> str:
    """
    Blank out comments, keeping the line structure. A block comment's newlines
    are preserved so whatever sat on either side of it stays separated.
    """

    return COMMENT_RE.sub(
        lambda match: "\n" * match.group(0).count("\n"),
        content
    )

#
# A specialization, not a use. "struct check_traits<ActionType>" prepares the
# type; "check_traits<ActionType>::check(type_)" only calls whatever is there,
# and matching the bare name counted the call as if it were the definition --
# which let the deliberately-invalid test fixture pass validation.
#
CHECK_TRAITS_SPECIALIZATION_RE = re.compile(
    r'(?:struct|class)\s+check_traits\s*<\s*([A-Za-z0-9_]+)'
)

#
# A declaration, not a call. "bool isValid() const" counts; "p.isValid()" and
# "ptr->empty()" do not -- those are the type using somebody else's check, and
# used to be read as the type having one of its own.
#
MEMBER_ISVALID_RE = re.compile(r'(?<![\w.>])isValid\s*\(\s*\)')
MEMBER_EMPTY_RE = re.compile(r'(?<![\w.>])empty\s*\(\s*\)')


def check_types_header(types_path: Path) -> bool:
    """
    Statically analyzes Types.h to verify that all custom types meet
    one of the three validity check requirements:
      1) Defines 'bool isValid() const'
      2) Defines 'empty()' (container-like types)
      3) Provides a 'check_traits<T>' specialization
    """
    if not types_path.exists():
        print(f"[Error] Target Types header not found at: {types_path}")
        return False

    content = strip_comments(
        types_path.read_text(encoding='utf-8')
    )

    struct_matches = re.findall(r'(?:struct|class)\s+([A-Za-z0-9_]+)', content)
    enum_matches = re.findall(r'enum\s+(?:class\s+)?([A-Za-z0-9_]+)', content)
    check_traits_matches = set(CHECK_TRAITS_SPECIALIZATION_RE.findall(content))

    passed = True

    # 1. Validate Structs/Classes against isValid, empty, or check_traits
    for struct_name in struct_matches:
        if struct_name in ['check_traits', 'std']:
            continue

        # "enum class ActionType" also matches the struct/class pattern above.
        # Left in, it reports one unprepared enum twice under two different
        # explanations, and the struct-side one is the misleading of the two:
        # it suggests adding isValid() to an enum. The enum loop below covers
        # these correctly.
        if struct_name in enum_matches:
            continue

        struct_pattern = rf'(?:struct|class)\s+{struct_name}\s*\{{(.*?)\}};'
        match = re.search(struct_pattern, content, re.DOTALL)

        body = match.group(1) if match else ""

        has_is_valid = bool(MEMBER_ISVALID_RE.search(body))
        has_empty = bool(MEMBER_EMPTY_RE.search(body))
        has_traits = struct_name in check_traits_matches

        if not (has_is_valid or has_empty or has_traits):
            print(f"[Validation Failed] Type '{struct_name}' in {types_path.name} "
                  f"does NOT implement 'isValid()', 'empty()', nor 'check_traits<{struct_name}>'.")
            passed = False

    # 2. Validate Enums against check_traits specialization
    for enum_name in enum_matches:
        if enum_name not in check_traits_matches:
            print(f"[Validation Failed] Enum '{enum_name}' in {types_path.name} "
                  f"lacks 'check_traits<{enum_name}>' specialization.")
            passed = False

    return passed


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Validate a Types.h header against the isValid()/empty()/"
                     "check_traits<T> preparation rules, without generating "
                     "or writing anything."
    )
    parser.add_argument(
        "types_header", nargs="?", default="src/Types.h",
        help="Path to the Types.h to validate (default: src/Types.h)"
    )
    args = parser.parse_args()

    types_path = Path(args.types_header)

    print(f"--> Validating {types_path}...")
    ok = check_types_header(types_path)

    if ok:
        print("--> Types validation passed.")
    else:
        print("\n[Validation Failed] See messages above.")

    sys.exit(0 if ok else 1)
