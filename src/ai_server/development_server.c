/**
 * Asthra Programming Language Compiler
 * Development Server - Include Aggregator
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * This file has been split into focused modules for better maintainability.
 * All implementation is now distributed across the following specialized files:
 *
 * - server_lifecycle.h/c: Server creation, destruction, and control operations
 * - server_network.h/c: Socket operations, WebSocket handling, and network communication
 * - request_processing.h/c: Request handling, command processing, and response generation
 * - realtime_analysis.h/c: Real-time analysis features, streaming, and subscriptions
 *
 * This include aggregator maintains backward compatibility while providing
 * a cleaner, more modular architecture that supports easier testing and
 * maintenance of the AI development server subsystem.
 */

// Include all split modules to maintain API compatibility
#include "realtime_analysis.h"
#include "request_processing.h"
#include "server_lifecycle.h"
#include "server_network.h"