---
name: test-compiler
description: Compile and run all example Asthra programs to test the compiler
user_invocable: true
---

Test the Asthra compiler by compiling and running every `.ast` file in the `examples/` directory.

Steps:
1. Run `zig build` to ensure the compiler is up to date. If this fails, report the build error and stop.
2. Find all `.ast` files in `examples/`
3. For each file:
   - Compile with `./zig-out/bin/asthra <file> -o /tmp/asthra_test_<basename>`
   - If compilation fails, record as FAIL with the error message
   - If compilation succeeds, run `/tmp/asthra_test_<basename>` and record as PASS with the output
   - Clean up the temp executable
4. Print a summary table showing each file and its PASS/FAIL status
5. Print final line: `N/M tests passed`
