/**
 * Asthra Programming Language Compiler
 * Command Line Interface - Implementation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "cli.h"
#include "version.h"
#include "../runtime/asthra_runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Only include getopt on Unix platforms
#if ASTHRA_PLATFORM_UNIX
    #include <getopt.h>
    #include <unistd.h>
#endif

// =============================================================================
// WINDOWS COMMAND LINE PARSING IMPLEMENTATION
// =============================================================================

#if ASTHRA_PLATFORM_WINDOWS

int win_optind = 1;
char* win_optarg = NULL;

static const win_option_t win_long_options[] = {
    {"output", true, 'o'},
    {"optimize", true, 'O'},
    {"debug", false, 'g'},
    {"verbose", false, 'v'},
    {"target", true, 't'},
    {"backend", true, 'b'},
    {"emit-llvm", false, 1000},
    {"emit-asm", false, 1001},
    {"no-stdlib", false, 1002},
    {"include", true, 'I'},
    {"library-path", true, 'L'},
    {"library", true, 'l'},
    {"test-mode", false, 1003},
    {"version", false, 1004},
    {"help", false, 'h'},
    {NULL, false, 0}
};

int win_getopt_long(int argc, char* argv[], const char* optstring, const win_option_t* longopts) {
    if (win_optind >= argc) {
        return -1;
    }

    char* arg = argv[win_optind];
    
    // Handle long options (--option)
    if (arg[0] == '-' && arg[1] == '-') {
        const char* opt_name = arg + 2;
        char* eq_pos = strchr(opt_name, '=');
        size_t name_len = eq_pos ? (size_t)(eq_pos - opt_name) : strlen(opt_name);
        
        for (const win_option_t* opt = longopts; opt->name; opt++) {
            if (strncmp(opt->name, opt_name, name_len) == 0 && strlen(opt->name) == name_len) {
                win_optind++;
                if (opt->has_arg) {
                    if (eq_pos) {
                        win_optarg = eq_pos + 1;
                    } else if (win_optind < argc) {
                        win_optarg = argv[win_optind++];
                    } else {
                        fprintf(stderr, "Option --%s requires an argument\n", opt->name);
                        return '?';
                    }
                }
                return opt->val;
            }
        }
        fprintf(stderr, "Unknown option: %s\n", arg);
        return '?';
    }
    
    // Handle short options (-o)
    if (arg[0] == '-' && arg[1] != '\0') {
        char opt_char = arg[1];
        win_optind++;
        
        // Check if option requires argument
        char* opt_pos = strchr(optstring, opt_char);
        if (opt_pos && opt_pos[1] == ':') {
            if (arg[2] != '\0') {
                win_optarg = arg + 2;
            } else if (win_optind < argc) {
                win_optarg = argv[win_optind++];
            } else {
                fprintf(stderr, "Option -%c requires an argument\n", opt_char);
                return '?';
            }
        }
        return opt_char;
    }
    
    // Not an option
    return -1;
}

#endif

// =============================================================================
// CLI FUNCTIONS IMPLEMENTATION
// =============================================================================

void cli_print_usage(const char *program_name) {
    // Get the appropriate executable extension for the platform
    const char* exe_ext = asthra_get_exe_extension();
    
    printf("Usage: %s%s [options] <input-file>\n", program_name, exe_ext);
    printf("\nOptions:\n");
    printf("  -o, --output <file>     Output file (default: a%s)\n", exe_ext);
    printf("  -O, --optimize <level>  Optimization level (0-3, default: 2)\n");
    printf("  -g, --debug             Include debug information\n");
    printf("  -v, --verbose           Verbose output\n");
    printf("  -t, --target <arch>     Target architecture (x86_64, arm64, wasm32, native)\n");
    printf("  -b, --backend <type>    Backend type (llvm only, default: llvm)\n");
    printf("  --emit-llvm             Deprecated - LLVM IR is now the default\n");
    printf("  --emit-asm              Deprecated - assembly backend removed\n");
    printf("  --no-stdlib             Don't link standard library\n");
    printf("  -I, --include <path>    Add include path\n");
    printf("  -L, --library-path <path> Add library search path\n");
    printf("  -l, --library <name>    Link library\n");
    printf("  --test-mode             Run in test mode\n");
    printf("  --version               Show version information\n");
    printf("  -h, --help              Show this help message\n");
    printf("\nExamples:\n");
    printf("  %s%s hello.asthra                    # Compile to a%s (LLVM backend)\n", program_name, exe_ext, exe_ext);
    printf("  %s%s -o hello%s hello.asthra           # Compile to hello%s\n", program_name, exe_ext, exe_ext, exe_ext);
    printf("  %s%s -O3 -g hello.asthra             # Optimize and include debug info\n", program_name, exe_ext);
    printf("  %s%s --target wasm32 hello.asthra    # Compile for WebAssembly\n", program_name, exe_ext);
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
    } arch_map[] = {
        {"x86_64", ASTHRA_TARGET_X86_64},
        {"arm64", ASTHRA_TARGET_ARM64},
        {"aarch64", ASTHRA_TARGET_ARM64},
        {"wasm32", ASTHRA_TARGET_WASM32},
        {"native", ASTHRA_TARGET_NATIVE}
    };

    for (size_t i = 0; i < sizeof(arch_map) / sizeof(arch_map[0]); i++) {
        if (strcmp(arch_str, arch_map[i].name) == 0) {
            return arch_map[i].arch;
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
    } opt_map[] = {
        {"0", ASTHRA_OPT_NONE},
        {"1", ASTHRA_OPT_BASIC},
        {"2", ASTHRA_OPT_STANDARD},
        {"3", ASTHRA_OPT_AGGRESSIVE}
    };

    for (size_t i = 0; i < sizeof(opt_map) / sizeof(opt_map[0]); i++) {
        if (strcmp(opt_str, opt_map[i].level) == 0) {
            return opt_map[i].opt;
        }
    }

    fprintf(stderr, "Error: Invalid optimization level '%s'\n", opt_str);
    return ASTHRA_OPT_STANDARD;
}

AsthraBackendType cli_parse_backend_type(const char *backend_str) {
    // C17 compound literal for backend type mapping
    static const struct {
        const char *name;
        AsthraBackendType type;
    } backend_map[] = {
        {"llvm", ASTHRA_BACKEND_LLVM_IR},
        {"LLVM", ASTHRA_BACKEND_LLVM_IR},
        {"llvm-ir", ASTHRA_BACKEND_LLVM_IR},
        {"ir", ASTHRA_BACKEND_LLVM_IR}
    };

    // Check if it's a deprecated backend
    if (strcmp(backend_str, "c") == 0 || strcmp(backend_str, "C") == 0) {
        fprintf(stderr, "Warning: C backend has been removed. Using LLVM backend instead.\n");
        return ASTHRA_BACKEND_LLVM_IR;
    }
    if (strcmp(backend_str, "asm") == 0 || strcmp(backend_str, "ASM") == 0 ||
        strcmp(backend_str, "assembly") == 0 || strcmp(backend_str, "s") == 0) {
        fprintf(stderr, "Warning: Assembly backend has been removed. Using LLVM backend instead.\n");
        return ASTHRA_BACKEND_LLVM_IR;
    }

    for (size_t i = 0; i < sizeof(backend_map) / sizeof(backend_map[0]); i++) {
        if (strcmp(backend_str, backend_map[i].name) == 0) {
            return backend_map[i].type;
        }
    }

    fprintf(stderr, "Error: Unknown backend type '%s'\n", backend_str);
    fprintf(stderr, "Valid backend: llvm\n");
    return ASTHRA_BACKEND_LLVM_IR;
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
    if (!options) return;
    
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
        {.name = "output",       .has_arg = required_argument, .flag = 0, .val = 'o'},
        {.name = "optimize",     .has_arg = required_argument, .flag = 0, .val = 'O'},
        {.name = "debug",        .has_arg = no_argument,       .flag = 0, .val = 'g'},
        {.name = "verbose",      .has_arg = no_argument,       .flag = 0, .val = 'v'},
        {.name = "target",       .has_arg = required_argument, .flag = 0, .val = 't'},
        {.name = "backend",      .has_arg = required_argument, .flag = 0, .val = 'b'},
        {.name = "emit-llvm",    .has_arg = no_argument,       .flag = 0, .val = 1000},
        {.name = "emit-asm",     .has_arg = no_argument,       .flag = 0, .val = 1001},
        {.name = "no-stdlib",    .has_arg = no_argument,       .flag = 0, .val = 1002},
        {.name = "include",      .has_arg = required_argument, .flag = 0, .val = 'I'},
        {.name = "library-path", .has_arg = required_argument, .flag = 0, .val = 'L'},
        {.name = "library",      .has_arg = required_argument, .flag = 0, .val = 'l'},
        {.name = "test-mode",    .has_arg = no_argument,       .flag = 0, .val = 1003},
        {.name = "version",      .has_arg = no_argument,       .flag = 0, .val = 1004},
        {.name = "help",         .has_arg = no_argument,       .flag = 0, .val = 'h'},
        {0, 0, 0, 0}
    };
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
                options->compiler_options.backend_type = cli_parse_backend_type(optarg);
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
            case 1000: // --emit-llvm
                options->compiler_options.emit_llvm = true;
                fprintf(stderr, "Warning: --emit-llvm is deprecated. LLVM IR is now the default backend.\n");
                break;
            case 1001: // --emit-asm
                options->compiler_options.emit_asm = true;
                fprintf(stderr, "Warning: --emit-asm is deprecated. Assembly backend has been removed.\n");
                break;
            case 1002: // --no-stdlib
                options->compiler_options.no_stdlib = true;
                break;
            case 1003: // --test-mode
                options->test_mode = true;
                break;
            case 1004: // --version
                options->show_version = true;
                options->exit_code = 0;
                goto cleanup_and_exit;
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
    // Warn if legacy flags are used
    if (options->compiler_options.emit_llvm) {
        fprintf(stderr, "Warning: --emit-llvm flag is deprecated. LLVM IR is now the default backend.\n");
    }
    if (options->compiler_options.emit_asm) {
        fprintf(stderr, "Warning: --emit-asm flag is deprecated. Assembly backend has been removed.\n");
    }
    
    // Always use LLVM backend
    if (options->compiler_options.backend_type == 0) {
        options->compiler_options.backend_type = ASTHRA_BACKEND_LLVM_IR;
    }

    return 0;

cleanup_and_exit:
    // Cleanup argument lists
    asthra_argument_list_destroy(include_paths);
    asthra_argument_list_destroy(library_paths);
    asthra_argument_list_destroy(libraries);
    
    return options->exit_code;
} 
