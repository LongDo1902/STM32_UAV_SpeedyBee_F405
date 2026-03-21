# Get-ChildItem Core\Src, Core\Inc -Recurse -File | Where-Object {
#     $_.Extension -in '.c', '.h'
# } | ForEach-Object {
#     py .\long_style_force_3_blank_lines_btw_func.py -i $_.FullName
# }

#!/usr/bin/env python3
from __future__ import annotations

import argparse
import pathlib
import re
import sys
from dataclasses import dataclass

FORBIDDEN_START = re.compile(r"^(if|for|while|switch|catch|else)\b")
COMMENT_ONLY = re.compile(r"^\s*(//|/\*|\*|\*/)$")


def looks_like_function_signature(text: str) -> bool:
    s = " ".join(text.strip().split())
    if not s:
        return False
    if s.startswith("#"):
        return False
    if FORBIDDEN_START.match(s):
        return False
    if "(" not in s or ")" not in s:
        return False
    # Exclude common C constructs that are not function definitions.
    if s.endswith(";"):
        return False
    return True


def strip_comments_and_strings(line: str, in_block_comment: bool) -> tuple[str, bool]:
    out: list[str] = []
    i = 0
    in_string = False
    string_quote = ""
    while i < len(line):
        ch = line[i]
        nxt = line[i + 1] if i + 1 < len(line) else ""

        if in_block_comment:
            if ch == "*" and nxt == "/":
                in_block_comment = False
                i += 2
            else:
                i += 1
            continue

        if in_string:
            if ch == "\\":
                i += 2
                continue
            if ch == string_quote:
                in_string = False
            i += 1
            continue

        if ch == "/" and nxt == "*":
            in_block_comment = True
            i += 2
            continue
        if ch == "/" and nxt == "/":
            break
        if ch in ('"', "'"):
            in_string = True
            string_quote = ch
            i += 1
            continue

        out.append(ch)
        i += 1

    return "".join(out), in_block_comment


@dataclass
class FunctionBlock:
    start_line: int
    end_line: int | None = None


def find_functions(lines: list[str]) -> list[FunctionBlock]:
    brace_depth = 0
    in_block_comment = False
    top_buf: list[str] = []
    current_func: FunctionBlock | None = None
    functions: list[FunctionBlock] = []

    for idx, raw_line in enumerate(lines):
        clean, in_block_comment = strip_comments_and_strings(raw_line, in_block_comment)
        stripped = clean.strip()
        opens = clean.count("{")
        closes = clean.count("}")
        next_depth = brace_depth + opens - closes

        if brace_depth == 0:
            raw_stripped = raw_line.strip()
            if raw_stripped and not raw_stripped.startswith("#") and not COMMENT_ONLY.match(raw_stripped):
                top_buf.append(raw_line)

            # Detect top-level function start when entering the first brace level.
            if opens > 0 and next_depth > 0:
                candidate = "".join(top_buf)
                if looks_like_function_signature(candidate):
                    current_func = FunctionBlock(start_line=idx)
                    functions.append(current_func)
                top_buf = []
            elif stripped.endswith(";"):
                top_buf = []

        if brace_depth > 0 and next_depth == 0 and current_func is not None and "}" in clean:
            current_func.end_line = idx
            current_func = None
            top_buf = []

        brace_depth = next_depth

    return [f for f in functions if f.end_line is not None]


def rewrite_with_three_blank_lines(lines: list[str], functions: list[FunctionBlock]) -> str:
    if len(functions) < 2:
        return "".join(lines)

    end_to_next_start = {
        functions[i].end_line: functions[i + 1].start_line
        for i in range(len(functions) - 1)
    }

    out: list[str] = []
    i = 0
    while i < len(lines):
        out.append(lines[i])
        if i in end_to_next_start:
            j = i + 1
            # Skip all blank lines after the function end.
            while j < len(lines) and lines[j].strip() == "":
                j += 1
            # Insert exactly two blank lines before the next content.
            out.append("\n")
            out.append("\n")
            out.append("\n")
            i = j
            continue
        i += 1

    return "".join(out)


def process_file(path: pathlib.Path, in_place: bool) -> int:
    original = path.read_text(encoding="utf-8")
    lines = original.splitlines(keepends=True)
    functions = find_functions(lines)
    updated = rewrite_with_three_blank_lines(lines, functions)

    if in_place:
        path.write_text(updated, encoding="utf-8")
    else:
        sys.stdout.write(updated)
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Ensure exactly three blank lines between top-level C/C++ function definitions."
    )
    parser.add_argument("files", nargs="+", help="Source files to process")
    parser.add_argument("-i", "--in-place", action="store_true", help="Modify files in place")
    args = parser.parse_args()

    rc = 0
    for file_name in args.files:
        path = pathlib.Path(file_name)
        if not path.is_file():
            print(f"error: not a file: {path}", file=sys.stderr)
            rc = 1
            continue
        try:
            process_file(path, args.in_place)
        except Exception as exc:  # pragma: no cover
            print(f"error processing {path}: {exc}", file=sys.stderr)
            rc = 1
    return rc


if __name__ == "__main__":
    raise SystemExit(main())
