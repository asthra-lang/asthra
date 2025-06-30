/**
 * Asthra Programming Language Compiler
 * Command Line Interface - Implementation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "cli.h"
#include "../runtime/asthra_runtime.h"
#include "version.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Include getopt for command line parsing
#include <getopt.h>
#include <unistd.h>

// =============================================================================
// CLI FUNCTIONS IMPLEMENTATION
// =============================================================================

void cli_print_usage(const char *program_name) {
    // Get the appropriate executable extension for the platform
    const char *exe_ext = asthra_get_exe_extension();

    printf("Usage: %s%s [options] <input-file>\n", program_name, exe_ext);
    printf("\nOptions:\n");
    printf("  -o, --output <file>     Output file (default: a%s)\n", exe_ext);
    printf("  -O, --optimize <level>  Optimization level (0-3, default: 2)\n");
    printf("  -g, --debug             Include debug information\n");
    printf("  -v, --verbose           Verbose output\n");
    printf("  -t, --target <arch>     Target architecture (x86_64 [Linux only], arm64, wasm32, "
           "native)\n");
    printf("  -b, --backend <type>    Backend type (llvm only, default: llvm)\n");
    printf("  --emit <format>         Output format: llvm-ir, llvm-bc, asm, obj, exe\n");
    printf("  --no-stdlib             Don't link standard library\n");
    printf("  --pie                   Force generation of position-independent executables\n");
    printf("  --no-pie                Disable PIE generation\n");
    printf("  -I, --include <path>    Add include path\n");
    printf("  -L, --library-path <path> Add library search path\n");
    printf("  -l, --library <name>    Link library\n");
    printf("  --coverage              Enable coverage instrumentation\n");
    printf("  --test-mode             Run in test mode\n");
    printf("  --version               Show version information\n");
    printf("  -h, --help              Show this help message\n");
    printf("\nExamples:\n");
    printf("  %s%s hello.asthra                    # Compile to a%s (LLVM backend)\n", program_name,
           exe_ext, exe_ext);
    printf("  %s%s -o hello%s hello.asthra           # Compile to hello%s\n", program_name, exe_ext,
           exe_ext, exe_ext);
    printf("  %s%s -O3 -g hello.asthra             # Optimize and include debug info\n",
           program_name, exe_ext);
    printf("  %s%s --target wasm32 hello.asthra    # Compile for WebAssembly\n", program_name,
           exe_ext);
}

void cli_print_version(void) {
    printf("Asthra Programming Language Compiler\n");
    printf("Version: %s\n", ASTHRA_VERSION_STRING);
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdate-time"
#endif
    printf("Build Date: %s %s\n", ASTHRA_BUILD_DATE, ASTHRA_BUILD_TIME);
#ifdef __clang__
#pragma clang diagnostic pop
#endif
    printf("Platform: %s\n", asthra_get_platform_info());
    printf("Runtime Version: %s\n", asthra_runtime_version());

    printf("\nFeatures:\n");
#if ASTHRA_FEATURE_PATTERN_MATCHING
    printf("  - Pattern Matching\n");
#endif
#if ASTHRA_FEATURE_FFI_SAFETY
    printf("  - Safe FFI\n");
#endif
#if ASTHRA_FEATURE_GARBAGE_COLLECTION
    printf("  - Garbage Collection\n");
#endif
#if ASTHRA_FEATURE_CONCURRENCY
    printf("  - Lightweight Concurrency\n");
#endif
#if ASTHRA_FEATURE_CRYPTOGRAPHY
    printf("  - Cryptographic Primitives\n");
#endif
#if ASTHRA_FEATURE_SLICE_MANAGEMENT
    printf("  - Formalized Slice Management\n");
#endif
#if ASTHRA_FEATURE_STRING_INTERPOLATION
    printf("  - String Interpolation\n");
#endif
#if ASTHRA_FEATURE_RESULT_TYPES
    printf("  - Result Types\n");
#endif

    printf("\nPlatform Information:\n");
    printf("  - Operating System: %s\n", ASTHRA_PLATFORM_NAME);
    printf("  - Compiler: %s\n", ASTHRA_COMPILER_NAME);
    printf("  - Architecture: %s\n", asthra_get_target_triple(ASTHRA_TARGET_NATIVE));
    printf("  - Path Separator: '%c'\n", asthra_get_path_separator());
    printf("  - Executable Extension: '%s'\n", asthra_get_exe_extension());
}

AsthraTargetArch cli_parse_target_arch(const char *arch_str) {
    // C17 compound literal for architecture mapping
    static const struct {
        const char *name;
        AsthraTargetArch arch;
    } arch_map[] = {{"x86_64", ASTHRA_TARGET_X86_64},
                    {"arm64", ASTHRA_TARGET_ARM64},
                    {"aarch64", ASTHRA_TARGET_ARM64},
                    {"wasm32", ASTHRA_TARGET_WASM32},
                    {"native", ASTHRA_TARGET_NATIVE}};

    for (size_t i = 0; i < sizeof(arch_map) / sizeof(arch_map[0]); i++) {
        if (strcmp(arch_str, arch_map[i].name) == 0) {
            AsthraTargetArch arch = arch_map[i].arch;

            // Check for unsupported platform combinations
#ifdef __APPLE__
            if (arch == ASTHRA_TARGET_X86_64) {
                fprintf(stderr,
                        "Error: x86_64 is no longer supported on macOS. Use arm64 or native.\n");
                return ASTHRA_TARGET_NATIVE;
            }
#endif

            return arch;
        }
    }

    fprintf(stderr, "Error: Unknown target architecture '%s'\n", arch_str);
    return ASTHRA_TARGET_NATIVE;
}

AsthraOptimizationLevel cli_parse_optimization_level(const char *opt_str) {
    // C17 compound literal for optimization level mapping
    static const struct {
        const char *level;
        AsthraOptimizationLevel opt;
    } opt_map[] = {{"0", ASTHRA_OPT_NONE},
                   {"1", ASTHRA_OPT_BASIC},
                   {"2", ASTHRA_OPT_STANDARD},
                   {"3", ASTHRA_OPT_AGGRESSIVE}};

    for (size_t i = 0; i < sizeof(opt_map) / sizeof(opt_map[0]); i++) {
        if (strcmp(opt_str, opt_map[i].level) == 0) {
            return opt_map[i].opt;
        }
    }

    fprintf(stderr, "Error: Invalid optimization level '%s'\n", opt_str);
    return ASTHRA_OPT_STANDARD;
}

CliOptions cli_options_init(void) {
    CliOptions options = {0};
    options.compiler_options = asthra_compiler_default_options();
    options.test_mode = false;
    options.show_version = false;
    options.show_help = false;
    options.exit_code = 0;
    return options;
}

void cli_options_cleanup(CliOptions *options) {
    if (!options)
        return;

    // Cleanup is handled by the compiler context when it's destroyed
    // This function is provided for future extensibility
}

int cli_parse_arguments(int argc, char *argv[], CliOptions *options) {
    if (!options) {
        return -1;
    }

    // Initialize argument lists
    struct AsthraArgumentList *include_paths = asthra_argument_list_create(8);
    struct AsthraArgumentList *library_paths = asthra_argument_list_create(8);
    struct AsthraArgumentList *libraries = asthra_argument_list_create(8);

    if (!include_paths || !library_paths || !libraries) {
        fprintf(stderr, "Error: Failed to allocate argument lists\n");
        asthra_argument_list_destroy(include_paths);
        asthra_argument_list_destroy(library_paths);
        asthra_argument_list_destroy(libraries);
        return -1;
    }

#if ASTHRA_PLATFORM_UNIX
    // C17 designated initializer for long options (Unix only)
    static struct option long_options[] = {
        {.name = "output", .has_arg = required_argument, .flag = 0, .val = 'o'},
        {.name = "optimize", .has_arg = required_argument, .flag = 0, .val = 'O'},
        {.name = "debug", .has_arg = no_argument, .flag = 0, .val = 'g'},
        {.name = "verbose", .has_arg = no_argument, .flag = 0, .val = 'v'},
        {.name = "target", .has_arg = required_argument, .flag = 0, .val = 't'},
        {.name = "backend", .has_arg = required_argument, .flag = 0, .val = 'b'},
        {.name = "emit", .has_arg = required_argument, .flag = 0, .val = 1005},
        {.name = "no-stdlib", .has_arg = no_argument, .flag = 0, .val = 1002},
        {.name = "include", .has_arg = required_argument, .flag = 0, .val = 'I'},
        {.name = "library-path", .has_arg = required_argument, .flag = 0, .val = 'L'},
        {.name = "library", .has_arg = required_argument, .flag = 0, .val = 'l'},
        {.name = "test-mode", .has_arg = no_argument, .flag = 0, .val = 1003},
        {.name = "version", .has_arg = no_argument, .flag = 0, .val = 1004},
        {.name = "coverage", .has_arg = no_argument, .flag = 0, .val = 1006},
        {.name = "pie", .has_arg = no_argument, .flag = 0, .val = 1007},
        {.name = "no-pie", .has_arg = no_argument, .flag = 0, .val = 1008},
        {.name = "help", .has_arg = no_argument, .flag = 0, .val = 'h'},
        {0, 0, 0, 0}};
#endif

    int c;
    int option_index = 0;

    while ((c = getopt_long(argc, argv, "o:O:gvt:b:I:L:l:h",
#if ASTHRA_PLATFORM_UNIX
                            long_options,
#else
                            win_long_options,
#endif
                            &option_index)) != -1) {
        switch (c) {
        case 'o':
            options->compiler_options.output_file = optarg;
            break;
        case 'O':
            options->compiler_options.opt_level = cli_parse_optimization_level(optarg);
            break;
        case 'g':
            options->compiler_options.debug_info = true;
            break;
        case 'v':
            options->compiler_options.verbose = true;
            break;
        case 't':
            options->compiler_options.target_arch = cli_parse_target_arch(optarg);
            break;
        case 'b':
            // Backend option is deprecated - LLVM is the only backend
            fprintf(stderr,
                    "Warning: -b/--backend option is deprecated. LLVM is now the only backend.\n");
            break;
        case 'I':
            // Add include path using C17 flexible array
            if (!asthra_argument_list_add(&include_paths, optarg)) {
                fprintf(stderr, "Error: Failed to add include path\n");
                goto cleanup_and_exit;
            }
            break;
        case 'L':
            // Add library path using C17 flexible array
            if (!asthra_argument_list_add(&library_paths, optarg)) {
                fprintf(stderr, "Error: Failed to add library path\n");
                goto cleanup_and_exit;
            }
            break;
        case 'l':
            // Add library using C17 flexible array
            if (!asthra_argument_list_add(&libraries, optarg)) {
                fprintf(stderr, "Error: Failed to add library\n");
                goto cleanup_and_exit;
            }
            break;
        case 1002: // --no-stdlib
            options->compiler_options.no_stdlib = true;
            break;
        case 1005: // --emit
            if (strcmp(optarg, "llvm-ir") == 0) {
                options->compiler_options.output_format = ASTHRA_FORMAT_LLVM_IR;
            } else if (strcmp(optarg, "llvm-bc") == 0) {
                options->compiler_options.output_format = ASTHRA_FORMAT_LLVM_BC;
            } else if (strcmp(optarg, "asm") == 0) {
                options->compiler_options.output_format = ASTHRA_FORMAT_ASSEMBLY;
            } else if (strcmp(optarg, "obj") == 0) {
                options->compiler_options.output_format = ASTHRA_FORMAT_OBJECT;
            } else if (strcmp(optarg, "exe") == 0) {
                options->compiler_options.output_format = ASTHRA_FORMAT_EXECUTABLE;
            } else {
                fprintf(stderr, "Error: Invalid output format '%s'\n", optarg);
                fprintf(stderr, "Valid formats: llvm-ir, llvm-bc, asm, obj, exe\n");
                options->exit_code = 1;
                goto cleanup_and_exit;
            }
            break;
        case 1003: // --test-mode
            options->test_mode = true;
            break;
        case 1004: // --version
            options->show_version = true;
            options->exit_code = 0;
            goto cleanup_and_exit;
        case 1006: // --coverage
            options->compiler_options.coverage = true;
            break;
        case 1007: // --pie
            options->compiler_options.pie_mode = ASTHRA_PIE_FORCE_ENABLED;
            break;
        case 1008: // --no-pie
            options->compiler_options.pie_mode = ASTHRA_PIE_FORCE_DISABLED;
            break;
        case 'h':
            options->show_help = true;
            options->exit_code = 0;
            goto cleanup_and_exit;
        case '?':
            fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
            options->exit_code = 1;
            goto cleanup_and_exit;
        default:
            fprintf(stderr, "Error: Unknown option\n");
            options->exit_code = 1;
            goto cleanup_and_exit;
        }
    }

    // Check for input file
    if (optind >= argc) {
        fprintf(stderr, "Error: No input file specified\n");
        cli_print_usage(argv[0]);
        options->exit_code = 1;
        goto cleanup_and_exit;
    }

    // Set input file
    options->compiler_options.input_file = argv[optind];

    // Set argument lists in options
    options->compiler_options.include_paths = include_paths;
    options->compiler_options.library_paths = library_paths;
    options->compiler_options.libraries = libraries;

    // Backend type is always LLVM now

    return 0;

cleanup_and_exit:
    // Cleanup argument lists
    asthra_argument_list_destroy(include_paths);
    asthra_argument_list_destroy(library_paths);
    asthra_argument_list_destroy(libraries);

    return options->exit_code;
}
