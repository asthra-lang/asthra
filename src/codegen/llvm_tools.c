/**
 * Asthra Programming Language LLVM Tools Integration
 * Implementation of LLVM ecosystem tool integration
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "llvm_tools.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

// Platform-specific includes
#ifdef _WIN32
    #include <windows.h>
    #include <process.h>
    #include <io.h>  // For _unlink on Windows
    #define unlink _unlink
#else
    #include <spawn.h>
    #include <unistd.h>  // For unlink, access, etc.
    #include <sys/types.h>
    #include <sys/wait.h>
    extern char **environ;
#endif

// =============================================================================
// INTERNAL HELPERS
// =============================================================================

// Execute external command and capture output
static AsthraLLVMToolResult execute_command(const char **argv, bool capture_output) {
    AsthraLLVMToolResult result = {
        .success = false,
        .exit_code = -1,
        .stdout_output = NULL,
        .stderr_output = NULL,
        .execution_time_ms = 0.0
    };
    
    clock_t start_time = clock();
    
#ifdef _WIN32
    // Windows implementation using CreateProcess
    // TODO: Implement Windows process execution
    result.stderr_output = strdup("Windows process execution not yet implemented");
    return result;
#else
    // Unix implementation using posix_spawn
    int stdout_pipe[2], stderr_pipe[2];
    pid_t pid;
    posix_spawn_file_actions_t actions;
    
    // Create pipes for capturing output if requested
    if (capture_output) {
        if (pipe(stdout_pipe) != 0 || pipe(stderr_pipe) != 0) {
            result.stderr_output = strdup("Failed to create pipes");
            return result;
        }
    }
    
    // Initialize spawn file actions
    posix_spawn_file_actions_init(&actions);
    
    if (capture_output) {
        // Redirect stdout and stderr to pipes
        posix_spawn_file_actions_addclose(&actions, stdout_pipe[0]);
        posix_spawn_file_actions_addclose(&actions, stderr_pipe[0]);
        posix_spawn_file_actions_adddup2(&actions, stdout_pipe[1], STDOUT_FILENO);
        posix_spawn_file_actions_adddup2(&actions, stderr_pipe[1], STDERR_FILENO);
        posix_spawn_file_actions_addclose(&actions, stdout_pipe[1]);
        posix_spawn_file_actions_addclose(&actions, stderr_pipe[1]);
    }
    
    // Spawn the process
    int spawn_result = posix_spawn(&pid, argv[0], &actions, NULL, (char **)argv, environ);
    posix_spawn_file_actions_destroy(&actions);
    
    if (spawn_result != 0) {
        if (capture_output) {
            close(stdout_pipe[0]);
            close(stdout_pipe[1]);
            close(stderr_pipe[0]);
            close(stderr_pipe[1]);
        }
        result.stderr_output = strdup(strerror(spawn_result));
        return result;
    }
    
    if (capture_output) {
        // Close write ends of pipes
        close(stdout_pipe[1]);
        close(stderr_pipe[1]);
        
        // Read output from pipes
        char buffer[4096];
        size_t stdout_size = 0, stderr_size = 0;
        size_t stdout_capacity = 4096, stderr_capacity = 4096;
        result.stdout_output = malloc(stdout_capacity);
        result.stderr_output = malloc(stderr_capacity);
        
        if (result.stdout_output && result.stderr_output) {
            result.stdout_output[0] = '\0';
            result.stderr_output[0] = '\0';
            
            // Read stdout
            ssize_t n;
            while ((n = read(stdout_pipe[0], buffer, sizeof(buffer))) > 0) {
                if (stdout_size + n >= stdout_capacity) {
                    stdout_capacity *= 2;
                    result.stdout_output = realloc(result.stdout_output, stdout_capacity);
                }
                memcpy(result.stdout_output + stdout_size, buffer, n);
                stdout_size += n;
                result.stdout_output[stdout_size] = '\0';
            }
            
            // Read stderr
            while ((n = read(stderr_pipe[0], buffer, sizeof(buffer))) > 0) {
                if (stderr_size + n >= stderr_capacity) {
                    stderr_capacity *= 2;
                    result.stderr_output = realloc(result.stderr_output, stderr_capacity);
                }
                memcpy(result.stderr_output + stderr_size, buffer, n);
                stderr_size += n;
                result.stderr_output[stderr_size] = '\0';
            }
        }
        
        close(stdout_pipe[0]);
        close(stderr_pipe[0]);
    }
    
    // Wait for process to complete
    int status;
    if (waitpid(pid, &status, 0) == -1) {
        result.stderr_output = strdup("Failed to wait for process");
        return result;
    }
    
    if (WIFEXITED(status)) {
        result.exit_code = WEXITSTATUS(status);
        result.success = (result.exit_code == 0);
    } else if (WIFSIGNALED(status)) {
        result.exit_code = -WTERMSIG(status);
        result.success = false;
    }
#endif
    
    clock_t end_time = clock();
    result.execution_time_ms = ((double)(end_time - start_time) / CLOCKS_PER_SEC) * 1000.0;
    
    return result;
}

// Find executable in PATH
static char* find_executable(const char *name) {
    const char *path_env = getenv("PATH");
    if (!path_env) return NULL;
    
    char *path_copy = strdup(path_env);
    char *path_dir = strtok(path_copy, ":");
    
    while (path_dir) {
        char full_path[4096];
        snprintf(full_path, sizeof(full_path), "%s/%s", path_dir, name);
        
        if (access(full_path, X_OK) == 0) {
            free(path_copy);
            return strdup(full_path);
        }
        
        path_dir = strtok(NULL, ":");
    }
    
    free(path_copy);
    return NULL;
}

// =============================================================================
// LLVM TOOL DETECTION AND VALIDATION
// =============================================================================

bool asthra_llvm_tools_available(void) {
    const char *required_tools[] = {"llc", "opt", "clang", NULL};
    
    for (int i = 0; required_tools[i]; i++) {
        char *path = find_executable(required_tools[i]);
        if (!path) {
            return false;
        }
        free(path);
    }
    
    return true;
}

const char* asthra_llvm_tool_path(const char *tool_name) {
    static char path_buffer[4096];
    char *path = find_executable(tool_name);
    
    if (path) {
        strncpy(path_buffer, path, sizeof(path_buffer) - 1);
        path_buffer[sizeof(path_buffer) - 1] = '\0';
        free(path);
        return path_buffer;
    }
    
    return NULL;
}

const char* asthra_llvm_version(void) {
    static char version_buffer[256];
    const char *llc_path = asthra_llvm_tool_path("llc");
    if (!llc_path) return NULL;
    
    const char *argv[] = {llc_path, "--version", NULL};
    AsthraLLVMToolResult result = execute_command(argv, true);
    
    if (result.success && result.stdout_output) {
        // Parse version from output
        char *version_line = strstr(result.stdout_output, "LLVM version");
        if (version_line) {
            sscanf(version_line, "LLVM version %255s", version_buffer);
            asthra_llvm_tool_result_free(&result);
            return version_buffer;
        }
    }
    
    asthra_llvm_tool_result_free(&result);
    return NULL;
}

// =============================================================================
// LLVM IR OPTIMIZATION
// =============================================================================

AsthraLLVMToolResult asthra_llvm_optimize(const char *input_file, 
                                          const AsthraLLVMToolOptions *options) {
    const char *opt_path = asthra_llvm_tool_path("opt");
    if (!opt_path) {
        AsthraLLVMToolResult result = {0};
        result.stderr_output = strdup("opt tool not found in PATH");
        return result;
    }
    
    // Build command line arguments
    const char *argv[32];
    int argc = 0;
    
    argv[argc++] = opt_path;
    argv[argc++] = input_file;
    argv[argc++] = "-o";
    argv[argc++] = options->output_file;
    
    // Add optimization level
    argv[argc++] = asthra_llvm_opt_level_flag(options->opt_level);
    
    // Add debug info preservation
    if (options->debug_info) {
        argv[argc++] = "-debugify";
    }
    
    // Add custom pass pipeline if specified
    if (options->pass_pipeline) {
        argv[argc++] = "-passes";
        argv[argc++] = options->pass_pipeline;
    }
    
    // Add S flag to emit text IR if requested
    if (options->emit_llvm_after_opt) {
        argv[argc++] = "-S";
    }
    
    argv[argc] = NULL;
    
    return execute_command(argv, options->verbose);
}

AsthraLLVMToolResult asthra_llvm_run_passes(const char *input_file,
                                            const char *passes,
                                            const char *output_file) {
    const char *opt_path = asthra_llvm_tool_path("opt");
    if (!opt_path) {
        AsthraLLVMToolResult result = {0};
        result.stderr_output = strdup("opt tool not found in PATH");
        return result;
    }
    
    const char *argv[] = {
        opt_path,
        input_file,
        "-o", output_file,
        "-passes", passes,
        "-S",  // Emit text IR
        NULL
    };
    
    return execute_command(argv, false);
}

// =============================================================================
// NATIVE CODE GENERATION
// =============================================================================

AsthraLLVMToolResult asthra_llvm_compile(const char *input_file,
                                         const AsthraLLVMToolOptions *options) {
    const char *llc_path = asthra_llvm_tool_path("llc");
    if (!llc_path) {
        AsthraLLVMToolResult result = {0};
        result.stderr_output = strdup("llc tool not found in PATH");
        return result;
    }
    
    // Build command line arguments
    const char *argv[32];
    int argc = 0;
    
    argv[argc++] = llc_path;
    argv[argc++] = input_file;
    argv[argc++] = "-o";
    argv[argc++] = options->output_file;
    
    // Add optimization level
    argv[argc++] = asthra_llvm_opt_level_flag(options->opt_level);
    
    // Add file type based on output format
    if (options->output_format == ASTHRA_FORMAT_ASSEMBLY) {
        argv[argc++] = "-filetype=asm";
    } else if (options->output_format == ASTHRA_FORMAT_OBJECT) {
        argv[argc++] = "-filetype=obj";
    }
    
    // Add target triple if specified
    if (options->target_triple) {
        argv[argc++] = "-mtriple";
        argv[argc++] = options->target_triple;
    }
    
    // Add CPU type if specified
    if (options->cpu_type) {
        argv[argc++] = "-mcpu";
        argv[argc++] = options->cpu_type;
    }
    
    // Add CPU features if specified
    if (options->features) {
        argv[argc++] = "-mattr";
        argv[argc++] = options->features;
    }
    
    argv[argc] = NULL;
    
    return execute_command(argv, options->verbose);
}

AsthraLLVMToolResult asthra_llvm_to_assembly(const char *input_file,
                                             const char *output_file,
                                             const char *target_triple) {
    AsthraLLVMToolOptions options = {
        .output_format = ASTHRA_FORMAT_ASSEMBLY,
        .output_file = output_file,
        .target_triple = target_triple,
        .opt_level = ASTHRA_OPT_NONE,
        .verbose = false
    };
    
    return asthra_llvm_compile(input_file, &options);
}

// =============================================================================
// LINKING AND EXECUTABLE GENERATION
// =============================================================================

AsthraLLVMToolResult asthra_llvm_link(const char **object_files,
                                      size_t num_objects,
                                      const AsthraLLVMToolOptions *options) {
    const char *clang_path = asthra_llvm_tool_path("clang");
    if (!clang_path) {
        AsthraLLVMToolResult result = {0};
        result.stderr_output = strdup("clang tool not found in PATH");
        return result;
    }
    
    // Build command line arguments
    const char **argv = malloc((num_objects + 16) * sizeof(char*));
    int argc = 0;
    
    argv[argc++] = clang_path;
    
    // Add object files
    for (size_t i = 0; i < num_objects; i++) {
        argv[argc++] = object_files[i];
    }
    
    argv[argc++] = "-o";
    argv[argc++] = options->output_file;
    
    // Add optimization level
    argv[argc++] = asthra_llvm_opt_level_flag(options->opt_level);
    
    // Add target triple if specified
    if (options->target_triple) {
        argv[argc++] = "-target";
        argv[argc++] = options->target_triple;
    }
    
    argv[argc] = NULL;
    
    AsthraLLVMToolResult result = execute_command(argv, options->verbose);
    free(argv);
    
    return result;
}

AsthraLLVMToolResult asthra_llvm_to_executable(const char *input_file,
                                               const char *output_file,
                                               const char **libraries,
                                               size_t num_libraries) {
    const char *clang_path = asthra_llvm_tool_path("clang");
    if (!clang_path) {
        AsthraLLVMToolResult result = {0};
        result.stderr_output = strdup("clang tool not found in PATH");
        return result;
    }
    
    // Build command line arguments
    const char **argv = malloc((num_libraries + 16) * sizeof(char*));
    int argc = 0;
    
    argv[argc++] = clang_path;
    argv[argc++] = input_file;
    argv[argc++] = "-o";
    argv[argc++] = output_file;
    
    // Add libraries
    for (size_t i = 0; i < num_libraries; i++) {
        argv[argc++] = "-l";
        argv[argc++] = libraries[i];
    }
    
    argv[argc] = NULL;
    
    AsthraLLVMToolResult result = execute_command(argv, false);
    free(argv);
    
    return result;
}

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

char* asthra_llvm_get_output_filename(const char *input_file,
                                     AsthraOutputFormat format) {
    if (!input_file) return NULL;
    
    // Find the last dot to get extension
    const char *last_dot = strrchr(input_file, '.');
    size_t base_len = last_dot ? (size_t)(last_dot - input_file) : strlen(input_file);
    
    const char *extension;
    switch (format) {
        case ASTHRA_FORMAT_LLVM_IR:
            extension = ".ll";
            break;
        case ASTHRA_FORMAT_LLVM_BC:
            extension = ".bc";
            break;
        case ASTHRA_FORMAT_ASSEMBLY:
            extension = ".s";
            break;
        case ASTHRA_FORMAT_OBJECT:
            extension = ".o";
            break;
        case ASTHRA_FORMAT_EXECUTABLE:
            extension = "";  // No extension for executables
            break;
        default:
            extension = ".out";
    }
    
    size_t output_len = base_len + strlen(extension) + 1;
    char *output_file = malloc(output_len);
    if (output_file) {
        strncpy(output_file, input_file, base_len);
        output_file[base_len] = '\0';
        strcat(output_file, extension);
    }
    
    return output_file;
}

const char* asthra_llvm_opt_level_flag(AsthraOptimizationLevel level) {
    switch (level) {
        case ASTHRA_OPT_NONE:
            return "-O0";
        case ASTHRA_OPT_BASIC:
            return "-O1";
        case ASTHRA_OPT_STANDARD:
            return "-O2";
        case ASTHRA_OPT_AGGRESSIVE:
            return "-O3";
        default:
            return "-O2";
    }
}

const char* asthra_llvm_target_triple(AsthraTargetArch arch) {
    switch (arch) {
        case ASTHRA_TARGET_X86_64:
            #ifdef __APPLE__
                return "x86_64-apple-darwin";
            #elif defined(__linux__)
                return "x86_64-pc-linux-gnu";
            #elif defined(_WIN32)
                return "x86_64-pc-windows-msvc";
            #else
                return "x86_64-unknown-unknown";
            #endif
            
        case ASTHRA_TARGET_ARM64:
            #ifdef __APPLE__
                return "arm64-apple-darwin";
            #elif defined(__linux__)
                return "aarch64-unknown-linux-gnu";
            #elif defined(_WIN32)
                return "aarch64-pc-windows-msvc";
            #else
                return "aarch64-unknown-unknown";
            #endif
            
        case ASTHRA_TARGET_WASM32:
            return "wasm32-unknown-unknown";
            
        case ASTHRA_TARGET_NATIVE:
        default:
            return NULL;  // Let LLVM detect the host triple
    }
}

void asthra_llvm_tool_result_free(AsthraLLVMToolResult *result) {
    if (!result) return;
    
    if (result->stdout_output) {
        free(result->stdout_output);
        result->stdout_output = NULL;
    }
    
    if (result->stderr_output) {
        free(result->stderr_output);
        result->stderr_output = NULL;
    }
}

// =============================================================================
// PIPELINE INTEGRATION
// =============================================================================

AsthraLLVMToolResult asthra_llvm_compile_pipeline(const char *ir_file,
                                                  const char *output_file,
                                                  AsthraOutputFormat format,
                                                  const AsthraCompilerOptions *options) {
    AsthraLLVMToolResult result = {0};
    
    // If output is LLVM IR and no optimization, just copy the file
    if (format == ASTHRA_FORMAT_LLVM_IR && options->opt_level == ASTHRA_OPT_NONE) {
        // Simple file copy
        FILE *src = fopen(ir_file, "r");
        FILE *dst = fopen(output_file, "w");
        if (src && dst) {
            char buffer[4096];
            size_t n;
            while ((n = fread(buffer, 1, sizeof(buffer), src)) > 0) {
                fwrite(buffer, 1, n, dst);
            }
            fclose(src);
            fclose(dst);
            result.success = true;
            return result;
        }
        result.stderr_output = strdup("Failed to copy IR file");
        return result;
    }
    
    // Temporary file for optimization output
    char *optimized_file = NULL;
    const char *compile_input = ir_file;
    
    // Run optimization if requested
    if (options->opt_level > ASTHRA_OPT_NONE) {
        optimized_file = asthra_llvm_get_output_filename(ir_file, ASTHRA_FORMAT_LLVM_BC);
        AsthraLLVMToolOptions opt_options = {
            .output_file = optimized_file,
            .opt_level = options->opt_level,
            .debug_info = options->debug_info,
            .verbose = options->verbose,
            .emit_llvm_after_opt = (format == ASTHRA_FORMAT_LLVM_IR)
        };
        
        result = asthra_llvm_optimize(ir_file, &opt_options);
        if (!result.success) {
            free(optimized_file);
            return result;
        }
        
        compile_input = optimized_file;
    }
    
    // If final output is LLVM IR, we're done
    if (format == ASTHRA_FORMAT_LLVM_IR) {
        if (optimized_file) {
            // Move optimized file to output
            if (rename(optimized_file, output_file) != 0) {
                result.success = false;
                result.stderr_output = strdup("Failed to move optimized IR to output");
            }
            free(optimized_file);
        }
        return result;
    }
    
    // Generate assembly or object code
    if (format == ASTHRA_FORMAT_ASSEMBLY || format == ASTHRA_FORMAT_OBJECT) {
        AsthraLLVMToolOptions compile_options = {
            .output_format = format,
            .output_file = output_file,
            .opt_level = ASTHRA_OPT_NONE,  // Already optimized
            .target_arch = options->target_arch,
            .target_triple = asthra_llvm_target_triple(options->target_arch),
            .debug_info = options->debug_info,
            .verbose = options->verbose
        };
        
        result = asthra_llvm_compile(compile_input, &compile_options);
    }
    
    // Generate executable
    else if (format == ASTHRA_FORMAT_EXECUTABLE) {
        // First compile to object
        char *object_file = asthra_llvm_get_output_filename(ir_file, ASTHRA_FORMAT_OBJECT);
        AsthraLLVMToolOptions compile_options = {
            .output_format = ASTHRA_FORMAT_OBJECT,
            .output_file = object_file,
            .opt_level = ASTHRA_OPT_NONE,  // Already optimized
            .target_arch = options->target_arch,
            .target_triple = asthra_llvm_target_triple(options->target_arch),
            .debug_info = options->debug_info,
            .verbose = options->verbose
        };
        
        result = asthra_llvm_compile(compile_input, &compile_options);
        if (result.success) {
            // Link to executable
            const char *objects[] = { object_file };
            AsthraLLVMToolOptions link_options = {
                .output_file = output_file,
                .opt_level = ASTHRA_OPT_NONE,
                .target_triple = compile_options.target_triple,
                .verbose = options->verbose
            };
            
            asthra_llvm_tool_result_free(&result);
            result = asthra_llvm_link(objects, 1, &link_options);
        }
        
        // Clean up temporary object file
        unlink(object_file);
        free(object_file);
    }
    
    // Clean up temporary optimized file
    if (optimized_file) {
        unlink(optimized_file);
        free(optimized_file);
    }
    
    return result;
}