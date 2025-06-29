/**
 * Asthra Programming Language LLVM Process Execution
 * Implementation of external process execution for LLVM tools
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "llvm_process_execution.h"
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Platform-specific includes
#ifdef _WIN32
#include <io.h> // For _unlink on Windows
#include <process.h>
#include <windows.h>
#define unlink _unlink
#else
#include <spawn.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h> // For unlink, access, etc.
extern char **environ;
#endif

// =============================================================================
// PROCESS EXECUTION IMPLEMENTATION
// =============================================================================

AsthraLLVMToolResult execute_command(const char **argv, bool capture_output) {
    AsthraLLVMToolResult result = {.success = false,
                                   .exit_code = -1,
                                   .stdout_output = NULL,
                                   .stderr_output = NULL,
                                   .execution_time_ms = 0.0};

    clock_t start_time = clock();

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

    clock_t end_time = clock();
    result.execution_time_ms = ((double)(end_time - start_time) / CLOCKS_PER_SEC) * 1000.0;

    return result;
}

// Find executable in PATH
char *find_executable(const char *name) {
    const char *path_env = getenv("PATH");
    if (!path_env)
        return NULL;

    // Try versioned names first (e.g., llc-18, clang-18)
    const char *version_suffixes[] = {"-18", "-17", "-16", "-15", "-14", ""};
    
    char *path_copy = strdup(path_env);
    
    for (int i = 0; version_suffixes[i] != NULL; i++) {
        char versioned_name[256];
        snprintf(versioned_name, sizeof(versioned_name), "%s%s", name, version_suffixes[i]);
        
        char *path_dir = strtok(path_copy, ":");
        while (path_dir) {
            char full_path[4096];
            snprintf(full_path, sizeof(full_path), "%s/%s", path_dir, versioned_name);

            if (access(full_path, X_OK) == 0) {
                free(path_copy);
                return strdup(full_path);
            }

            path_dir = strtok(NULL, ":");
        }
        
        // Reset for next version suffix
        free(path_copy);
        path_copy = strdup(path_env);
    }

    free(path_copy);
    return NULL;
}