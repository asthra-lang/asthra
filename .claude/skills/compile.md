---
name: compile
description: Compile and run an Asthra source file
user_invocable: true
---

Compile and run an Asthra (.ast) source file. The argument is the path to the source file.

Steps:
1. Run `zig build` to ensure the compiler is up to date
2. Run `./zig-out/bin/asthra <file> -o /tmp/asthra_run` to compile the Asthra source
3. If compilation fails, show the error diagnostics and stop
4. Run `/tmp/asthra_run` and display the output
5. Clean up by removing `/tmp/asthra_run`

If no argument is provided, ask the user which file to compile.
