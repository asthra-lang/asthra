/**
 * Asthra Programming Language Compiler
 * Platform-Specific Executable Generator
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * This module provides platform-specific executable generation capabilities
 * that handle the final step of linking and producing runnable executables.
 */

#ifndef ASTHRA_EXECUTABLE_GENERATOR_H
#define ASTHRA_EXECUTABLE_GENERATOR_H

#include "../compiler.h"
#include "../linker/asthra_linker.h"
#include "../platform.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations
typedef struct Asthra_ExecutableGenerator Asthra_ExecutableGenerator;
typedef struct Asthra_ExecutableFormat Asthra_ExecutableFormat;
typedef struct Asthra_ExecutableSection Asthra_ExecutableSection;
typedef struct Asthra_ExecutableValidation Asthra_ExecutableValidation;

// =============================================================================
// EXECUTABLE FORMAT DEFINITIONS
// =============================================================================

/**
 * Supported executable formats
 */
typedef enum {
    ASTHRA_EXEC_FORMAT_ELF,    // Linux ELF
    ASTHRA_EXEC_FORMAT_MACH_O, // macOS Mach-O
    ASTHRA_EXEC_FORMAT_PE,     // Windows PE
    ASTHRA_EXEC_FORMAT_UNKNOWN
} Asthra_ExecutableFormatType;

/**
 * Executable section information
 */
struct Asthra_ExecutableSection {
    char *name;
    uint64_t virtual_address;
    uint64_t file_offset;
    size_t size;
    uint32_t permissions; // Read, Write, Execute flags

    // Section data
    uint8_t *data;
    size_t data_size;

    // Section properties
    bool is_loadable;
    bool is_executable;
    bool is_writable;
    bool is_readable;

    struct Asthra_ExecutableSection *next;
};

/**
 * Executable format configuration
 */
struct Asthra_ExecutableFormat {
    Asthra_ExecutableFormatType type;

    // Format-specific properties
    union {
        struct {
            uint16_t machine_type;
            uint32_t entry_point;
            bool position_independent;
        } elf;

        struct {
            uint32_t cpu_type;
            uint32_t cpu_subtype;
            uint64_t entry_point;
        } mach_o;

        struct {
            uint16_t machine_type;
            uint32_t entry_point;
            bool console_app;
        } pe;
    } format_data;

    // Common properties
    uint64_t base_address;
    size_t page_size;
    size_t alignment;

    // Runtime properties
    char **library_paths;
    size_t library_count;
    char **runtime_dependencies;
    size_t dependency_count;
};

/**
 * Executable validation result
 */
struct Asthra_ExecutableValidation {
    bool is_valid;
    bool has_entry_point;
    bool sections_valid;
    bool symbols_resolved;
    bool format_compliant;

    // Validation errors
    char **error_messages;
    size_t error_count;

    // Validation warnings
    char **warning_messages;
    size_t warning_count;

    // File properties
    size_t file_size;
    uint32_t file_permissions;
    bool is_executable_file;

    // Performance metrics
    double validation_time_ms;
};

/**
 * Main executable generator structure
 */
struct Asthra_ExecutableGenerator {
    // Target format configuration
    Asthra_ExecutableFormat format;
    Asthra_ExecutableFormatType target_format;

    // Sections to include in executable
    Asthra_ExecutableSection *sections;
    size_t section_count;

    // Generator state
    bool initialized;
    bool format_configured;
    char *output_path;

    // Linking information
    const Asthra_LinkingResult *linking_result;

    // Generation options
    struct {
        bool strip_symbols;
        bool optimize_size;
        bool add_debug_info;
        bool enable_pie; // Position Independent Executable
        bool static_linking;
    } options;

    // Statistics
    struct {
        size_t total_sections;
        size_t total_symbols;
        size_t executable_size;
        double generation_time_ms;
        size_t relocations_applied;
    } statistics;

    // Error handling
    char *last_error;
    bool error_occurred;

    // Platform-specific data
    void *platform_data;
    size_t platform_data_size;
};

// =============================================================================
// EXECUTABLE GENERATOR API
// =============================================================================

/**
 * Create a new executable generator for the target platform
 *
 * @param target_format Target executable format
 * @return New executable generator, or NULL on failure
 */
Asthra_ExecutableGenerator *
asthra_executable_generator_create(Asthra_ExecutableFormatType target_format);

/**
 * Destroy executable generator and free resources
 *
 * @param generator Generator to destroy
 */
void asthra_executable_generator_destroy(Asthra_ExecutableGenerator *generator);

/**
 * Configure the executable format and properties
 *
 * @param generator Executable generator
 * @param format Format configuration
 * @return true on success, false on failure
 */
bool asthra_executable_generator_configure_format(Asthra_ExecutableGenerator *generator,
                                                  const Asthra_ExecutableFormat *format);

/**
 * Set generation options
 *
 * @param generator Executable generator
 * @param strip_symbols Whether to strip symbol information
 * @param optimize_size Whether to optimize for size
 * @param add_debug_info Whether to include debug information
 * @param enable_pie Whether to create position-independent executable
 * @param static_linking Whether to use static linking
 * @return true on success, false on failure
 */
bool asthra_executable_generator_set_options(Asthra_ExecutableGenerator *generator,
                                             bool strip_symbols, bool optimize_size,
                                             bool add_debug_info, bool enable_pie,
                                             bool static_linking);

/**
 * Add section to the executable
 *
 * @param generator Executable generator
 * @param section Section to add
 * @return true on success, false on failure
 */
bool asthra_executable_generator_add_section(Asthra_ExecutableGenerator *generator,
                                             const Asthra_ExecutableSection *section);

/**
 * Generate executable from linking result
 *
 * @param generator Executable generator
 * @param linking_result Result from linking phase
 * @param output_path Path for output executable
 * @return true on success, false on failure
 */
bool asthra_executable_generator_generate(Asthra_ExecutableGenerator *generator,
                                          const Asthra_LinkingResult *linking_result,
                                          const char *output_path);

/**
 * Validate generated executable
 *
 * @param generator Executable generator
 * @param executable_path Path to executable to validate
 * @param validation Result of validation
 * @return true if validation successful, false on error
 */
bool asthra_executable_generator_validate(Asthra_ExecutableGenerator *generator,
                                          const char *executable_path,
                                          Asthra_ExecutableValidation *validation);

/**
 * Set executable permissions and metadata
 *
 * @param generator Executable generator
 * @param executable_path Path to executable
 * @return true on success, false on failure
 */
bool asthra_executable_generator_set_permissions(Asthra_ExecutableGenerator *generator,
                                                 const char *executable_path);

// =============================================================================
// PLATFORM-SPECIFIC FUNCTIONS
// =============================================================================

/**
 * Get the default executable format for current platform
 *
 * @return Default executable format type
 */
Asthra_ExecutableFormatType asthra_executable_get_default_format(void);

/**
 * Get file extension for executable format
 *
 * @param format Executable format type
 * @return File extension string (e.g., ".exe", "")
 */
const char *asthra_executable_get_extension(Asthra_ExecutableFormatType format);

/**
 * Check if format is supported on current platform
 *
 * @param format Format to check
 * @return true if supported, false otherwise
 */
bool asthra_executable_format_supported(Asthra_ExecutableFormatType format);

/**
 * Create default format configuration for platform
 *
 * @param format_type Type of format to configure
 * @param format Output format configuration
 * @return true on success, false on failure
 */
bool asthra_executable_create_default_format(Asthra_ExecutableFormatType format_type,
                                             Asthra_ExecutableFormat *format);

// =============================================================================
// ELF-SPECIFIC FUNCTIONS
// =============================================================================

/**
 * Generate ELF executable (Linux)
 *
 * @param generator Executable generator
 * @param linking_result Linking result with object files
 * @param output_path Output executable path
 * @return true on success, false on failure
 */
bool asthra_executable_generate_elf(Asthra_ExecutableGenerator *generator,
                                    const Asthra_LinkingResult *linking_result,
                                    const char *output_path);

/**
 * Validate ELF executable format
 *
 * @param executable_path Path to ELF executable
 * @param validation Validation result
 * @return true if validation successful, false on error
 */
bool asthra_executable_validate_elf(const char *executable_path,
                                    Asthra_ExecutableValidation *validation);

// =============================================================================
// MACH-O SPECIFIC FUNCTIONS (macOS)
// =============================================================================

/**
 * Generate Mach-O executable (macOS)
 *
 * @param generator Executable generator
 * @param linking_result Linking result with object files
 * @param output_path Output executable path
 * @return true on success, false on failure
 */
bool asthra_executable_generate_mach_o(Asthra_ExecutableGenerator *generator,
                                       const Asthra_LinkingResult *linking_result,
                                       const char *output_path);

/**
 * Validate Mach-O executable format
 *
 * @param executable_path Path to Mach-O executable
 * @param validation Validation result
 * @return true if validation successful, false on error
 */
bool asthra_executable_validate_mach_o(const char *executable_path,
                                       Asthra_ExecutableValidation *validation);

// =============================================================================
// PE SPECIFIC FUNCTIONS (Windows)
// =============================================================================

/**
 * Generate PE executable (Windows)
 *
 * @param generator Executable generator
 * @param linking_result Linking result with object files
 * @param output_path Output executable path
 * @return true on success, false on failure
 */
bool asthra_executable_generate_pe(Asthra_ExecutableGenerator *generator,
                                   const Asthra_LinkingResult *linking_result,
                                   const char *output_path);

/**
 * Validate PE executable format
 *
 * @param executable_path Path to PE executable
 * @param validation Validation result
 * @return true if validation successful, false on error
 */
bool asthra_executable_validate_pe(const char *executable_path,
                                   Asthra_ExecutableValidation *validation);

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

/**
 * Get executable generator statistics
 *
 * @param generator Executable generator
 * @param total_sections Number of sections generated
 * @param executable_size Size of generated executable
 * @param generation_time_ms Time taken for generation
 * @param relocations_applied Number of relocations applied
 */
void asthra_executable_generator_get_statistics(Asthra_ExecutableGenerator *generator,
                                                size_t *total_sections, size_t *executable_size,
                                                double *generation_time_ms,
                                                size_t *relocations_applied);

/**
 * Free executable section
 *
 * @param section Section to free
 */
void asthra_executable_section_free(Asthra_ExecutableSection *section);

/**
 * Free executable format
 *
 * @param format Format to free
 */
void asthra_executable_format_free(Asthra_ExecutableFormat *format);

/**
 * Cleanup executable validation result
 *
 * @param validation Validation result to cleanup
 */
void asthra_executable_validation_cleanup(Asthra_ExecutableValidation *validation);

/**
 * Get last error message from generator
 *
 * @param generator Executable generator
 * @return Error message string, or NULL if no error
 */
const char *asthra_executable_generator_get_last_error(Asthra_ExecutableGenerator *generator);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_EXECUTABLE_GENERATOR_H
