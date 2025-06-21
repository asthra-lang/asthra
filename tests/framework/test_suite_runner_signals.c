/**
 * Test Suite Runner - Signal Handling
 * 
 * Signal handling functionality for test suite runner.
 * 
 * Copyright (c) 2025 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "test_suite_runner_common.h"

// Global runner instance for signal handling
AsthraTestRunner* g_runner = NULL;

// =============================================================================
// SIGNAL HANDLING
// =============================================================================

void signal_handler(int sig) {
    (void)sig;
    if (g_runner) {
        g_runner->interrupted = true;
        printf("\n\nTest execution interrupted by user\n");
    }
}

void setup_signal_handlers(void) {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
}