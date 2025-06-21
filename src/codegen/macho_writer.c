/**
 * Asthra Programming Language Compiler
 * Mach-O Object File Writer - Include Aggregator
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * This file has been split into focused modules for better maintainability.
 * All implementation is now distributed across the following specialized files:
 * 
 * - macho_platform.h/c: Platform detection and CPU type constants
 * - macho_data_structures.h/c: Core data structure management and lifecycle
 * - macho_sections.h/c: Section creation, data manipulation, and standard sections
 * - macho_file_writer.h/c: Layout calculation and file writing operations
 * 
 * This include aggregator maintains backward compatibility while providing
 * a cleaner, more modular architecture that supports easier testing and
 * maintenance of the Mach-O object file generation subsystem.
 */

// Include all split modules to maintain API compatibility
#include "macho_platform.h"
#include "macho_data_structures.h"
#include "macho_sections.h"
#include "macho_file_writer.h"