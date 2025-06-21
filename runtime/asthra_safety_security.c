/**
 * Asthra Safety System - Security Enforcement Module
 * Stack canaries and security validation
 */

#include "asthra_safety_common.h"
#include "asthra_runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "asthra_safety_security.h"

// =============================================================================
// SECURITY STATE
// =============================================================================

// Thread-local stack canary
static ASTHRA_THREAD_LOCAL AsthraStackCanary *g_thread_canary = NULL;

// =============================================================================
// STACK CANARY IMPLEMENTATION
// =============================================================================

int asthra_safety_install_stack_canary(void) {
    AsthraSafetyConfig* config = asthra_safety_get_config_ptr();
    
    if (!config->enable_stack_canaries) {
        return 0;
    }
    
    if (g_thread_canary) {
        return 0; // Already installed
    }
    
    g_thread_canary = malloc(sizeof(AsthraStackCanary));
    if (!g_thread_canary) {
        return -1;
    }
    
    // Generate a random canary value
    if (asthra_random_bytes((uint8_t*)&g_thread_canary->canary_value, 
                           sizeof(g_thread_canary->canary_value)) != 0) {
        free(g_thread_canary);
        g_thread_canary = NULL;
        return -1;
    }
    
    g_thread_canary->stack_base = __builtin_frame_address(0);
    g_thread_canary->stack_size = 0; // Could be determined from system limits
    g_thread_canary->thread_id = pthread_self();
    g_thread_canary->creation_timestamp = asthra_get_timestamp_ns();
    g_thread_canary->is_active = true;
    
    asthra_log(ASTHRA_LOG_DEBUG, ASTHRA_LOG_CATEGORY_GENERAL,
              "Stack canary installed for thread %lu with value 0x%lx", 
              (unsigned long)g_thread_canary->thread_id, g_thread_canary->canary_value);
    
    return 0;
}

int asthra_safety_check_stack_canary(void) {
    AsthraSafetyConfig* config = asthra_safety_get_config_ptr();
    
    if (!config->enable_stack_canaries || !g_thread_canary) {
        return 0;
    }
    
    // In a real implementation, this would check if the canary value
    // has been corrupted by comparing it with a stored value
    // For now, we just verify the canary structure is intact
    
    if (!g_thread_canary->is_active) {
        asthra_safety_report_violation(ASTHRA_VIOLATION_SECURITY, ASTHRA_SAFETY_LEVEL_ENHANCED,
                                     "Stack canary is not active", __FILE__, __LINE__, __func__, NULL, 0);
        return -1;
    }
    
    if (g_thread_canary->thread_id != pthread_self()) {
        asthra_safety_report_violation(ASTHRA_VIOLATION_SECURITY, ASTHRA_SAFETY_LEVEL_ENHANCED,
                                     "Stack canary thread mismatch", __FILE__, __LINE__, __func__, NULL, 0);
        return -1;
    }
    
    // Check if canary value has been corrupted (simplified check)
    if (g_thread_canary->canary_value == 0) {
        asthra_safety_report_violation(ASTHRA_VIOLATION_SECURITY, ASTHRA_SAFETY_LEVEL_ENHANCED,
                                     "Stack canary value corrupted (zero)", __FILE__, __LINE__, __func__, NULL, 0);
        return -1;
    }
    
    // Check for stack growth beyond expected bounds (simplified)
    void *current_frame = __builtin_frame_address(0);
    ptrdiff_t stack_growth = (char*)g_thread_canary->stack_base - (char*)current_frame;
    
    // Warn if stack has grown significantly (more than 1MB)
    if (stack_growth > 1024 * 1024) {
        char warning_msg[256];
        snprintf(warning_msg, sizeof(warning_msg),
                "Large stack growth detected: %ld bytes", (long)stack_growth);
        
        asthra_safety_report_violation(ASTHRA_VIOLATION_SECURITY, ASTHRA_SAFETY_LEVEL_STANDARD,
                                     warning_msg, __FILE__, __LINE__, __func__, 
                                     &stack_growth, sizeof(stack_growth));
    }
    
    return 0;
}

void asthra_safety_remove_stack_canary(void) {
    AsthraSafetyConfig* config = asthra_safety_get_config_ptr();
    
    if (!config->enable_stack_canaries) {
        return;
    }
    
    if (g_thread_canary) {
        // Mark as inactive before freeing
        g_thread_canary->is_active = false;
        
        asthra_log(ASTHRA_LOG_DEBUG, ASTHRA_LOG_CATEGORY_GENERAL,
                  "Stack canary removed for thread %lu", 
                  (unsigned long)g_thread_canary->thread_id);
        
        free(g_thread_canary);
        g_thread_canary = NULL;
    }
}

// =============================================================================
// ENHANCED SECURITY VALIDATION
// =============================================================================

static int asthra_safety_validate_memory_protection(void *ptr, size_t size) {
    AsthraSafetyConfig* config = asthra_safety_get_config_ptr();
    
    if (!config->enable_secure_memory_validation) {
        return 0;
    }
    
    if (!ptr || size == 0) {
        asthra_safety_report_violation(ASTHRA_VIOLATION_SECURITY, ASTHRA_SAFETY_LEVEL_STANDARD,
                                     "Invalid memory protection validation parameters", 
                                     __FILE__, __LINE__, __func__, NULL, 0);
        return -1;
    }
    
    // Check if memory region is readable (simplified check)
    volatile char test_byte;
    char *test_ptr = (char*)ptr;
    
    // Try to read first and last bytes
    test_byte = test_ptr[0];
    test_byte = test_ptr[size - 1];
    (void)test_byte; // Suppress unused variable warning
    
    return 0;
}

static int asthra_safety_validate_pointer_alignment(void *ptr, size_t alignment) {
    AsthraSafetyConfig* config = asthra_safety_get_config_ptr();
    
    if (!config->enable_secure_memory_validation) {
        return 0;
    }
    
    if (!ptr) {
        return -1;
    }
    
    uintptr_t addr = (uintptr_t)ptr;
    if (addr % alignment != 0) {
        char violation_msg[256];
        snprintf(violation_msg, sizeof(violation_msg),
                "Pointer %p not aligned to %zu bytes", ptr, alignment);
        
        asthra_safety_report_violation(ASTHRA_VIOLATION_SECURITY, ASTHRA_SAFETY_LEVEL_STANDARD,
                                     violation_msg, __FILE__, __LINE__, __func__, 
                                     &addr, sizeof(addr));
        return -1;
    }
    
    return 0;
}

// =============================================================================
// SECURE MEMORY OPERATIONS
// =============================================================================

static int asthra_safety_secure_zero_memory(void *ptr, size_t size) {
    AsthraSafetyConfig* config = asthra_safety_get_config_ptr();
    
    if (!config->enable_secure_memory_validation) {
        return 0;
    }
    
    if (!ptr || size == 0) {
        return -1;
    }
    
    // Secure zeroing that prevents compiler optimization
    volatile char *volatile_ptr = (volatile char*)ptr;
    for (size_t i = 0; i < size; i++) {
        volatile_ptr[i] = 0;
    }
    
    // Verify zeroing was successful
    char *check_ptr = (char*)ptr;
    for (size_t i = 0; i < size; i++) {
        if (check_ptr[i] != 0) {
            char violation_msg[256];
            snprintf(violation_msg, sizeof(violation_msg),
                    "Secure zeroing failed at offset %zu", i);
            
            asthra_safety_report_violation(ASTHRA_VIOLATION_SECURITY, ASTHRA_SAFETY_LEVEL_ENHANCED,
                                         violation_msg, __FILE__, __LINE__, __func__, 
                                         &i, sizeof(i));
            return -1;
        }
    }
    
    return 0;
}

// =============================================================================
// SECURITY AUDIT FUNCTIONS
// =============================================================================

static void asthra_safety_audit_thread_security(void) {
    AsthraSafetyConfig* config = asthra_safety_get_config_ptr();
    
    if (!config->enable_security_enforcement) {
        return;
    }
    
    // Check stack canary
    if (asthra_safety_check_stack_canary() != 0) {
        asthra_log(ASTHRA_LOG_WARN, ASTHRA_LOG_CATEGORY_GENERAL,
                  "Security audit: Stack canary check failed");
    }
    
    // Additional security checks could be added here
    // - Check for ROP/JOP gadgets
    // - Validate control flow integrity
    // - Check for memory corruption patterns
    
    asthra_log(ASTHRA_LOG_DEBUG, ASTHRA_LOG_CATEGORY_GENERAL,
              "Security audit completed for thread %lu", 
              (unsigned long)pthread_self());
}

// =============================================================================
// MODULE CLEANUP
// =============================================================================

void asthra_safety_security_cleanup(void) {
    // Remove stack canary for current thread
    asthra_safety_remove_stack_canary();
} 
