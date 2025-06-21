/**
 * Asthra Programming Language Compiler
 * Standalone Linker Tool - asthra-link
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * This tool provides advanced linking capabilities for Asthra object files
 * and can be used independently of the main compiler pipeline.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <getopt.h>
#include <unistd.h>

#include "../src/platform.h"
#include "../src/linker/asthra_linker.h"
#include "../src/linker/object_file_manager.h"
#include "../src/linker/symbol_resolution.h"
#include "../src/platform/executable_generator.h"

// =============================================================================
// COMMAND LINE OPTIONS
// =============================================================================

typedef struct {
    char **input_files;
    size_t input_count;
    char *output_file;
    bool verbose;
    bool static_linking;
    bool strip_symbols;
    bool debug_info;
    bool show_statistics;
    bool validate_output;
    bool force_overwrite;
    char **library_paths;
    size_t library_path_count;
    char **libraries;
    size_t library_count;
    Asthra_ExecutableFormatType output_format;
} AsthreLinkOptions;

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

static void print_usage(const char *program_name) {
    printf("Asthra Standalone Linker - asthra-link\n");
    printf("Usage: %s [options] input_files...\n\n", program_name);
    printf("Options:\n");
    printf("  -o, --output FILE       Output executable file\n");
    printf("  -l, --library LIB       Link with library\n");
    printf("  -L, --library-path DIR  Add library search path\n");
    printf("  -s, --strip             Strip symbol information\n");
    printf("  -g, --debug             Include debug information\n");
    printf("  -S, --static            Use static linking\n");
    printf("  -v, --verbose           Verbose output\n");
    printf("  -f, --force             Force overwrite output file\n");
    printf("  --stats                 Show linking statistics\n");
    printf("  --validate              Validate output executable\n");
    printf("  --format FORMAT         Output format (elf, macho, pe)\n");
    printf("  -h, --help              Show this help message\n");
    printf("  --version               Show version information\n\n");
    printf("Examples:\n");
    printf("  %s -o program file1.o file2.o\n", program_name);
    printf("  %s --static -o program file.o -lc\n", program_name);
    printf("  %s --verbose --stats -o program *.o\n", program_name);
}

static void print_version(void) {
    printf("asthra-link (Asthra Programming Language Linker)\n");
    printf("Version: 1.0.0\n");
    printf("Platform: %s\n", ASTHRA_PLATFORM_NAME);
    printf("Compiler: %s\n", ASTHRA_COMPILER_NAME);
}

static bool parse_format(const char *format_str, Asthra_ExecutableFormatType *format) {
    if (!format_str || !format) return false;
    
    if (strcmp(format_str, "elf") == 0) {
        *format = ASTHRA_EXEC_FORMAT_ELF;
        return true;
    } else if (strcmp(format_str, "macho") == 0) {
        *format = ASTHRA_EXEC_FORMAT_MACH_O;
        return true;
    } else if (strcmp(format_str, "pe") == 0) {
        *format = ASTHRA_EXEC_FORMAT_PE;
        return true;
    }
    
    return false;
}

static void free_options(AsthreLinkOptions *options) {
    if (!options) return;
    
    if (options->input_files) {
        for (size_t i = 0; i < options->input_count; i++) {
            if (options->input_files[i]) {
                free(options->input_files[i]);
            }
        }
        free(options->input_files);
    }
    
    if (options->output_file) {
        free(options->output_file);
    }
    
    if (options->library_paths) {
        for (size_t i = 0; i < options->library_path_count; i++) {
            if (options->library_paths[i]) {
                free(options->library_paths[i]);
            }
        }
        free(options->library_paths);
    }
    
    if (options->libraries) {
        for (size_t i = 0; i < options->library_count; i++) {
            if (options->libraries[i]) {
                free(options->libraries[i]);
            }
        }
        free(options->libraries);
    }
}

static bool add_string_to_array(char ***array, size_t *count, const char *str) {
    if (!array || !count || !str) return false;
    
    char **new_array = realloc(*array, (*count + 1) * sizeof(char*));
    if (!new_array) return false;
    
    new_array[*count] = strdup(str);
    if (!new_array[*count]) return false;
    
    *array = new_array;
    (*count)++;
    return true;
}

// =============================================================================
// COMMAND LINE PARSING
// =============================================================================

static bool parse_arguments(int argc, char *argv[], AsthreLinkOptions *options) {
    static struct option long_options[] = {
        {"output", required_argument, 0, 'o'},
        {"library", required_argument, 0, 'l'},
        {"library-path", required_argument, 0, 'L'},
        {"strip", no_argument, 0, 's'},
        {"debug", no_argument, 0, 'g'},
        {"static", no_argument, 0, 'S'},
        {"verbose", no_argument, 0, 'v'},
        {"force", no_argument, 0, 'f'},
        {"stats", no_argument, 0, 1000},
        {"validate", no_argument, 0, 1001},
        {"format", required_argument, 0, 1002},
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 1003},
        {0, 0, 0, 0}
    };
    
    memset(options, 0, sizeof(AsthreLinkOptions));
    options->output_format = asthra_executable_get_default_format();
    
    int option_index = 0;
    int c;
    
    while ((c = getopt_long(argc, argv, "o:l:L:sgSvfh", long_options, &option_index)) != -1) {
        switch (c) {
            case 'o':
                if (options->output_file) free(options->output_file);
                options->output_file = strdup(optarg);
                break;
                
            case 'l':
                if (!add_string_to_array(&options->libraries, &options->library_count, optarg)) {
                    fprintf(stderr, "Error: Failed to add library %s\n", optarg);
                    return false;
                }
                break;
                
            case 'L':
                if (!add_string_to_array(&options->library_paths, &options->library_path_count, optarg)) {
                    fprintf(stderr, "Error: Failed to add library path %s\n", optarg);
                    return false;
                }
                break;
                
            case 's':
                options->strip_symbols = true;
                break;
                
            case 'g':
                options->debug_info = true;
                break;
                
            case 'S':
                options->static_linking = true;
                break;
                
            case 'v':
                options->verbose = true;
                break;
                
            case 'f':
                options->force_overwrite = true;
                break;
                
            case 1000: // --stats
                options->show_statistics = true;
                break;
                
            case 1001: // --validate
                options->validate_output = true;
                break;
                
            case 1002: // --format
                if (!parse_format(optarg, &options->output_format)) {
                    fprintf(stderr, "Error: Unknown output format '%s'\n", optarg);
                    return false;
                }
                break;
                
            case 'h':
                print_usage(argv[0]);
                return false;
                
            case 1003: // --version
                print_version();
                return false;
                
            case '?':
                return false;
                
            default:
                fprintf(stderr, "Error: Unknown option\n");
                return false;
        }
    }
    
    // Collect input files
    for (int i = optind; i < argc; i++) {
        if (!add_string_to_array(&options->input_files, &options->input_count, argv[i])) {
            fprintf(stderr, "Error: Failed to add input file %s\n", argv[i]);
            return false;
        }
    }
    
    // Validate required options
    if (options->input_count == 0) {
        fprintf(stderr, "Error: No input files specified\n");
        return false;
    }
    
    if (!options->output_file) {
        options->output_file = strdup("a.out");
    }
    
    return true;
}

// =============================================================================
// LINKING IMPLEMENTATION
// =============================================================================

static bool perform_linking(const AsthreLinkOptions *options) {
    if (!options) return false;
    
    bool success = false;
    Asthra_ObjectFile **loaded_files = NULL;
    
    // Create object file manager
    Asthra_ObjectFileManager *manager = asthra_object_manager_create();
    if (!manager) {
        fprintf(stderr, "Error: Failed to create object file manager\n");
        return false;
    }
    
    // Add library search paths
    for (size_t i = 0; i < options->library_path_count; i++) {
        if (!asthra_object_manager_add_search_path(manager, options->library_paths[i])) {
            if (options->verbose) {
                fprintf(stderr, "Warning: Failed to add library path: %s\n", options->library_paths[i]);
            }
        }
    }
    
    // Load input files
    if (options->verbose) {
        printf("Loading %zu input files...\n", options->input_count);
    }
    
    for (size_t i = 0; i < options->input_count; i++) {
        if (options->verbose) {
            printf("  Loading: %s\n", options->input_files[i]);
        }
        
        Asthra_ObjectFile *object_file = asthra_object_manager_load_file(manager, options->input_files[i]);
        if (!object_file) {
            fprintf(stderr, "Error: Failed to load object file: %s\n", options->input_files[i]);
            goto cleanup;
        }
    }
    
    // Create symbol resolver
    Asthra_SymbolResolver *resolver = asthra_symbol_resolver_create(1024);
    if (!resolver) {
        fprintf(stderr, "Error: Failed to create symbol resolver\n");
        goto cleanup;
    }
    
    // Configure symbol resolver
    asthra_symbol_resolver_configure(resolver, 
                                   !options->static_linking, // allow undefined if dynamic
                                   true,  // prefer strong symbols
                                   true); // case sensitive
    
    // Add symbols from all loaded object files
    size_t total_symbols = 0;
    size_t loaded_count = 0;
    
    // Allocate space for loaded files
    size_t max_files = 1024;  // Maximum number of object files
    loaded_files = malloc(max_files * sizeof(Asthra_ObjectFile*));
    if (!loaded_files) {
        fprintf(stderr, "Error: Failed to allocate memory for object files\n");
        asthra_symbol_resolver_destroy(resolver);
        goto cleanup;
    }
    
    loaded_count = asthra_object_manager_get_all_files(manager, loaded_files, max_files);
    if (loaded_count == 0) {
        fprintf(stderr, "Error: No object files loaded\n");
        free(loaded_files);
        asthra_symbol_resolver_destroy(resolver);
        goto cleanup;
    }
    
    for (size_t i = 0; i < loaded_count; i++) {
        size_t added = asthra_symbol_resolver_add_object_file(resolver, loaded_files[i]);
        total_symbols += added;
        
        if (options->verbose) {
            printf("  Added %zu symbols from %s\n", added, loaded_files[i]->file_path);
        }
    }
    
    if (options->verbose) {
        printf("Total symbols loaded: %zu\n", total_symbols);
    }
    
    // Resolve symbols
    Asthra_ResolutionResult resolution_result;
    if (!asthra_symbol_resolver_resolve_all(resolver, &resolution_result)) {
        fprintf(stderr, "Error: Symbol resolution failed\n");
        
        if (resolution_result.undefined_count > 0) {
            fprintf(stderr, "Undefined symbols:\n");
            for (size_t i = 0; i < resolution_result.undefined_count; i++) {
                fprintf(stderr, "  %s\n", resolution_result.undefined_symbol_names[i]);
            }
        }
        
        asthra_resolution_result_cleanup(&resolution_result);
        asthra_symbol_resolver_destroy(resolver);
        goto cleanup;
    }
    
    if (options->verbose || options->show_statistics) {
        printf("Symbol resolution completed:\n");
        printf("  Total symbols: %zu\n", resolution_result.total_symbols);
        printf("  Resolved: %zu\n", resolution_result.resolved_symbols);
        printf("  Undefined: %zu\n", resolution_result.undefined_symbols);
        printf("  Weak: %zu\n", resolution_result.weak_symbols);
        printf("  Resolution time: %.2f ms\n", resolution_result.resolution_time_ms);
    }
    
    // Create linker
    Asthra_Linker *linker = asthra_linker_create();
    if (!linker) {
        fprintf(stderr, "Error: Failed to create linker\n");
        asthra_resolution_result_cleanup(&resolution_result);
        asthra_symbol_resolver_destroy(resolver);
        goto cleanup;
    }
    
    // Configure linker options
    Asthra_LinkingRequest request;
    memset(&request, 0, sizeof(request));
    request.output_executable_path = options->output_file;
    request.strip_symbols = options->strip_symbols;
    request.generate_debug_info = options->debug_info;
    request.static_linking = options->static_linking;
    
    // Convert object file paths
    const char **object_file_paths = malloc(loaded_count * sizeof(char*));
    if (!object_file_paths) {
        fprintf(stderr, "Error: Failed to allocate memory for object file paths\n");
        asthra_linker_destroy(linker);
        asthra_resolution_result_cleanup(&resolution_result);
        asthra_symbol_resolver_destroy(resolver);
        goto cleanup;
    }
    for (size_t i = 0; i < loaded_count; i++) {
        object_file_paths[i] = loaded_files[i]->file_path;
    }
    request.object_files = object_file_paths;
    request.object_file_count = loaded_count;
    
    // Configure and execute linking
    if (!asthra_linker_configure(linker, &request)) {
        fprintf(stderr, "Error: Failed to configure linker\n");
        free(object_file_paths);
        asthra_linker_destroy(linker);
        asthra_resolution_result_cleanup(&resolution_result);
        asthra_symbol_resolver_destroy(resolver);
        goto cleanup;
    }
    
    // Perform linking
    Asthra_LinkingResult linking_result;
    if (!asthra_linker_execute(linker, &linking_result)) {
        fprintf(stderr, "Error: Linking failed\n");
        if (linking_result.error_message) {
            fprintf(stderr, "  %s\n", linking_result.error_message);
        }
        
        asthra_linking_result_cleanup(&linking_result);
        asthra_linker_destroy(linker);
        asthra_resolution_result_cleanup(&resolution_result);
        asthra_symbol_resolver_destroy(resolver);
        goto cleanup;
    }
    
    if (options->verbose || options->show_statistics) {
        printf("Linking completed:\n");
        printf("  Output file: %s\n", linking_result.executable_path);
        printf("  Symbols processed: %zu\n", linking_result.total_symbols_processed);
        printf("  Symbols resolved: %zu\n", linking_result.symbols_resolved);
        printf("  Linking time: %.2f ms\n", linking_result.linking_time_ms);
    }
    
    // Validate output if requested
    if (options->validate_output) {
        if (options->verbose) {
            printf("Validating output executable...\n");
        }
        
        Asthra_ExecutableGenerator *generator = asthra_executable_generator_create(options->output_format);
        if (generator) {
            Asthra_ExecutableValidation validation;
            if (asthra_executable_generator_validate(generator, options->output_file, &validation)) {
                if (validation.is_valid) {
                    if (options->verbose) {
                        printf("Validation successful:\n");
                        printf("  File size: %zu bytes\n", validation.file_size);
                        printf("  Executable: %s\n", validation.is_executable_file ? "yes" : "no");
                        printf("  Format compliant: %s\n", validation.format_compliant ? "yes" : "no");
                    }
                } else {
                    fprintf(stderr, "Validation failed:\n");
                    for (size_t i = 0; i < validation.error_count; i++) {
                        fprintf(stderr, "  %s\n", validation.error_messages[i]);
                    }
                }
                asthra_executable_validation_cleanup(&validation);
            }
            asthra_executable_generator_destroy(generator);
        }
    }
    
    success = true;
    
    // Cleanup
    free(object_file_paths);
    asthra_linking_result_cleanup(&linking_result);
    asthra_linker_destroy(linker);
    asthra_resolution_result_cleanup(&resolution_result);
    asthra_symbol_resolver_destroy(resolver);
    
cleanup:
    if (loaded_files) {
        free(loaded_files);
    }
    asthra_object_manager_destroy(manager);
    
    return success;
}

// =============================================================================
// MAIN FUNCTION
// =============================================================================

int main(int argc, char *argv[]) {
    AsthreLinkOptions options;
    
    // Parse command line arguments
    if (!parse_arguments(argc, argv, &options)) {
        return 1;
    }
    
    // Check if output file exists and handle force flag
    if (!options.force_overwrite && access(options.output_file, F_OK) == 0) {
        fprintf(stderr, "Error: Output file '%s' already exists (use -f to force overwrite)\n", 
                options.output_file);
        free_options(&options);
        return 1;
    }
    
    // Check if output format is supported
    if (!asthra_executable_format_supported(options.output_format)) {
        fprintf(stderr, "Error: Output format not supported on this platform\n");
        free_options(&options);
        return 1;
    }
    
    if (options.verbose) {
        printf("Asthra Standalone Linker\n");
        printf("Input files: %zu\n", options.input_count);
        printf("Output file: %s\n", options.output_file);
        printf("Output format: %s\n", 
               options.output_format == ASTHRA_EXEC_FORMAT_ELF ? "ELF" :
               options.output_format == ASTHRA_EXEC_FORMAT_MACH_O ? "Mach-O" :
               options.output_format == ASTHRA_EXEC_FORMAT_PE ? "PE" : "Unknown");
        printf("Static linking: %s\n", options.static_linking ? "yes" : "no");
        printf("\n");
    }
    
    // Perform linking
    bool success = perform_linking(&options);
    
    if (success) {
        if (options.verbose) {
            printf("Linking completed successfully.\n");
        }
        free_options(&options);
        return 0;
    } else {
        fprintf(stderr, "Linking failed.\n");
        free_options(&options);
        return 1;
    }
} 
