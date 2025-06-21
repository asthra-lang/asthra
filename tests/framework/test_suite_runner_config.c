/**
 * Test Suite Runner - Configuration Management
 * 
 * Configuration parsing and management for test suite runner.
 * 
 * Copyright (c) 2025 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "test_suite_runner_common.h"

// =============================================================================
// CONFIGURATION MANAGEMENT
// =============================================================================

AsthraTestRunnerConfig default_config(void) {
    AsthraTestRunnerConfig config = {0};
    config.parallel_execution = false;
    config.max_parallel_tests = 4;
    config.fail_fast = false;
    config.randomize_order = false;
    config.repeat_tests = false;
    config.repeat_count = 1;
    config.output_format = ASTHRA_FORMAT_CONSOLE;
    config.verbose = false;
    config.quiet = false;
    config.show_skipped = true;
    config.show_duration = true;
    config.colored_output = true;
    config.output_file = NULL;
    config.debug_mode = false;
    config.profile_memory = false;
    config.dry_run = false;
    config.default_timeout_ns = 30000000000ULL; // 30 seconds
    config.suite_timeout_ns = 600000000000ULL;  // 10 minutes
    config.auto_discover = false;
    return config;
}

void print_usage(const char* program_name) {
    printf("Usage: %s [OPTIONS] [TEST_PATTERNS...]\n\n", program_name);
    printf("Test Selection Options:\n");
    printf("  -t, --test PATTERN      Run tests matching PATTERN\n");
    printf("  -s, --suite PATTERN     Run test suites matching PATTERN\n");
    printf("  -e, --exclude PATTERN   Exclude tests matching PATTERN\n");
    printf("  -d, --discover DIR      Auto-discover tests in directory\n");
    printf("\nExecution Options:\n");
    printf("  -j, --parallel N        Run up to N tests in parallel\n");
    printf("  -f, --fail-fast         Stop on first test failure\n");
    printf("  -r, --randomize         Randomize test execution order\n");
    printf("  --repeat N              Repeat tests N times\n");
    printf("  --timeout SECONDS       Set default test timeout\n");
    printf("\nOutput Options:\n");
    printf("  -v, --verbose           Verbose output\n");
    printf("  -q, --quiet             Minimal output\n");
    printf("  --format FORMAT         Output format: console, json, tap, junit\n");
    printf("  -o, --output FILE       Write output to file\n");
    printf("  --no-color              Disable colored output\n");
    printf("  --no-duration           Hide test durations\n");
    printf("\nDebugging Options:\n");
    printf("  --debug                 Enable debug output\n");
    printf("  --profile-memory        Profile memory usage\n");
    printf("  --dry-run               Show tests that would run without executing\n");
    printf("\nOther Options:\n");
    printf("  -h, --help              Show this help message\n");
    printf("  --version               Show version information\n");
}

AsthraTestOutputFormat parse_output_format(const char* format_str) {
    if (strcmp(format_str, "console") == 0) return ASTHRA_FORMAT_CONSOLE;
    if (strcmp(format_str, "plain") == 0) return ASTHRA_FORMAT_PLAIN;
    if (strcmp(format_str, "json") == 0) return ASTHRA_FORMAT_JSON;
    if (strcmp(format_str, "xml") == 0) return ASTHRA_FORMAT_XML;
    if (strcmp(format_str, "tap") == 0) return ASTHRA_FORMAT_TAP;
    if (strcmp(format_str, "junit") == 0) return ASTHRA_FORMAT_JUNIT;
    if (strcmp(format_str, "markdown") == 0) return ASTHRA_FORMAT_MARKDOWN;
    
    printf("Warning: Unknown output format '%s', using console\n", format_str);
    return ASTHRA_FORMAT_CONSOLE;
}

int parse_command_line(int argc, char** argv, AsthraTestRunnerConfig* config) {
    static struct option long_options[] = {
        {"test", required_argument, 0, 't'},
        {"suite", required_argument, 0, 's'},
        {"exclude", required_argument, 0, 'e'},
        {"discover", required_argument, 0, 'd'},
        {"parallel", required_argument, 0, 'j'},
        {"fail-fast", no_argument, 0, 'f'},
        {"randomize", no_argument, 0, 'r'},
        {"repeat", required_argument, 0, 1001},
        {"timeout", required_argument, 0, 1002},
        {"verbose", no_argument, 0, 'v'},
        {"quiet", no_argument, 0, 'q'},
        {"format", required_argument, 0, 1003},
        {"output", required_argument, 0, 'o'},
        {"no-color", no_argument, 0, 1004},
        {"no-duration", no_argument, 0, 1005},
        {"debug", no_argument, 0, 1006},
        {"profile-memory", no_argument, 0, 1007},
        {"dry-run", no_argument, 0, 1008},
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 1009},
        {0, 0, 0, 0}
    };
    
    int c;
    while ((c = getopt_long(argc, argv, "t:s:e:d:j:frvqo:h", long_options, NULL)) != -1) {
        switch (c) {
            case 't':
                // Add test pattern
                config->test_patterns = realloc(config->test_patterns, 
                    (config->pattern_count + 1) * sizeof(char*));
                config->test_patterns[config->pattern_count++] = strdup(optarg);
                break;
            case 's':
                // Add suite pattern
                config->suite_patterns = realloc(config->suite_patterns,
                    (config->suite_pattern_count + 1) * sizeof(char*));
                config->suite_patterns[config->suite_pattern_count++] = strdup(optarg);
                break;
            case 'e':
                // Add exclude pattern
                config->exclude_patterns = realloc(config->exclude_patterns,
                    (config->exclude_pattern_count + 1) * sizeof(char*));
                config->exclude_patterns[config->exclude_pattern_count++] = strdup(optarg);
                break;
            case 'd':
                // Add test directory
                config->test_directories = realloc(config->test_directories,
                    (config->directory_count + 1) * sizeof(char*));
                config->test_directories[config->directory_count++] = strdup(optarg);
                config->auto_discover = true;
                break;
            case 'j':
                config->parallel_execution = true;
                config->max_parallel_tests = (size_t)atoi(optarg);
                break;
            case 'f':
                config->fail_fast = true;
                break;
            case 'r':
                config->randomize_order = true;
                break;
            case 'v':
                config->verbose = true;
                break;
            case 'q':
                config->quiet = true;
                break;
            case 'o':
                config->output_file = strdup(optarg);
                break;
            case 1001: // --repeat
                config->repeat_tests = true;
                config->repeat_count = (size_t)atoi(optarg);
                break;
            case 1002: // --timeout
                config->default_timeout_ns = (uint64_t)atoi(optarg) * 1000000000ULL;
                break;
            case 1003: // --format
                config->output_format = parse_output_format(optarg);
                break;
            case 1004: // --no-color
                config->colored_output = false;
                break;
            case 1005: // --no-duration
                config->show_duration = false;
                break;
            case 1006: // --debug
                config->debug_mode = true;
                break;
            case 1007: // --profile-memory
                config->profile_memory = true;
                break;
            case 1008: // --dry-run
                config->dry_run = true;
                break;
            case 'h':
                print_usage(argv[0]);
                return 1;
            case 1009: // --version
                printf("Asthra Test Runner v1.0.0\n");
                return 1;
            case '?':
                printf("Use --help for usage information\n");
                return -1;
            default:
                break;
        }
    }
    
    // Add remaining arguments as test patterns
    for (int i = optind; i < argc; i++) {
        config->test_patterns = realloc(config->test_patterns,
            (config->pattern_count + 1) * sizeof(char*));
        config->test_patterns[config->pattern_count++] = strdup(argv[i]);
    }
    
    return 0;
}