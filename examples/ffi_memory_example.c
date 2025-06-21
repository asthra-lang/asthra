/**
 * Asthra Safe C Memory Interface Example
 * Demonstrates PQC key management and FFI operations
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#include "../runtime/asthra_ffi_memory.h"
#include "../runtime/asthra_runtime.h"

// Simulated PQC algorithm parameters
typedef struct {
    const char *name;
    size_t public_key_size;
    size_t private_key_size;
    size_t signature_size;
} PQCAlgorithm;

// Example PQC algorithms with different key sizes
static const PQCAlgorithm pqc_algorithms[] = {
    {"Dilithium2", 1312, 2528, 2420},
    {"Dilithium3", 1952, 4000, 3293},
    {"Dilithium5", 2592, 4864, 4595},
    {"Kyber512", 800, 1632, 0},      // KEM, no signatures
    {"Kyber768", 1184, 2400, 0},
    {"Kyber1024", 1568, 3168, 0}
};

#define NUM_ALGORITHMS (sizeof(pqc_algorithms) / sizeof(pqc_algorithms[0]))

// Simulated key generation function
void simulate_key_generation(uint8_t *public_key, size_t pub_size,
                           uint8_t *private_key, size_t priv_size) {
    // Fill with pseudo-random data for demonstration
    srand((unsigned int)time(NULL));
    
    for (size_t i = 0; i < pub_size; i++) {
        public_key[i] = (uint8_t)(rand() % 256);
    }
    
    for (size_t i = 0; i < priv_size; i++) {
        private_key[i] = (uint8_t)(rand() % 256);
    }
}

// Demonstrate PQC key management with slices
void demonstrate_pqc_key_management(void) {
    printf("\n=== PQC Key Management Demo ===\n");
    
    for (size_t i = 0; i < NUM_ALGORITHMS; i++) {
        const PQCAlgorithm *alg = &pqc_algorithms[i];
        
        printf("\nAlgorithm: %s\n", alg->name);
        printf("  Public key size: %zu bytes\n", alg->public_key_size);
        printf("  Private key size: %zu bytes\n", alg->private_key_size);
        
        // Create secure slices for keys
        AsthraFFISliceHeader pub_key_slice = Asthra_slice_new(
            sizeof(uint8_t), alg->public_key_size, alg->public_key_size,
            ASTHRA_ZONE_HINT_SECURE);
        
        AsthraFFISliceHeader priv_key_slice = Asthra_slice_new(
            sizeof(uint8_t), alg->private_key_size, alg->private_key_size,
            ASTHRA_ZONE_HINT_SECURE);
        
        if (!Asthra_slice_is_valid(pub_key_slice) || !Asthra_slice_is_valid(priv_key_slice)) {
            printf("  ERROR: Failed to allocate key slices\n");
            continue;
        }
        
        // Generate keys
        uint8_t *pub_key_data = (uint8_t*)Asthra_slice_get_ptr(pub_key_slice);
        uint8_t *priv_key_data = (uint8_t*)Asthra_slice_get_ptr(priv_key_slice);
        
        simulate_key_generation(pub_key_data, alg->public_key_size,
                              priv_key_data, alg->private_key_size);
        
        printf("  ✓ Keys generated successfully\n");
        
        // Demonstrate slice operations
        uint8_t first_byte;
        AsthraFFIResult get_result = Asthra_slice_get_element(pub_key_slice, 0, &first_byte);
        if (Asthra_result_is_ok(get_result)) {
            printf("  First byte of public key: 0x%02X\n", first_byte);
        }
        
        // Create subslice for key fingerprint (first 8 bytes)
        size_t fingerprint_size = (alg->public_key_size >= 8) ? 8 : alg->public_key_size;
        AsthraFFIResult subslice_result = Asthra_slice_subslice(pub_key_slice, 0, fingerprint_size);
        
        if (Asthra_result_is_ok(subslice_result)) {
            AsthraFFISliceHeader *fingerprint = (AsthraFFISliceHeader*)Asthra_result_unwrap_ok(subslice_result);
            printf("  Key fingerprint: ");
            
            for (size_t j = 0; j < fingerprint_size; j++) {
                uint8_t byte;
                if (Asthra_result_is_ok(Asthra_slice_get_element(*fingerprint, j, &byte))) {
                    printf("%02X", byte);
                }
            }
            printf("\n");
            
            free(fingerprint);
        }
        
        // Secure cleanup
        Asthra_secure_zero_slice(pub_key_slice);
        Asthra_secure_zero_slice(priv_key_slice);
        Asthra_slice_free(pub_key_slice);
        Asthra_slice_free(priv_key_slice);
        
        printf("  ✓ Keys securely cleaned up\n");
    }
}

// Demonstrate string interpolation for logging
void demonstrate_string_operations(void) {
    printf("\n=== String Operations Demo ===\n");
    
    // Create variant array for interpolation
    AsthraVariantArray args = Asthra_variant_array_new(4);
    
    // Add various types of data
    AsthraVariant timestamp_var = {.type = ASTHRA_VARIANT_U64, .value.u64_val = (uint64_t)time(NULL)};
    AsthraVariant algorithm_var = {.type = ASTHRA_VARIANT_STRING, 
                                  .value.string_val = Asthra_string_from_cstr("Dilithium3", ASTHRA_OWNERSHIP_TRANSFER_FULL)};
    AsthraVariant key_size_var = {.type = ASTHRA_VARIANT_U32, .value.u32_val = 1952};
    AsthraVariant success_var = {.type = ASTHRA_VARIANT_BOOL, .value.bool_val = true};
    
    Asthra_variant_array_push(&args, timestamp_var);
    Asthra_variant_array_push(&args, algorithm_var);
    Asthra_variant_array_push(&args, key_size_var);
    Asthra_variant_array_push(&args, success_var);
    
    // Interpolate log message
    const char *template = "[{}] Algorithm: {}, Key size: {} bytes, Success: {}";
    AsthraFFIResult interp_result = Asthra_string_interpolate(template, args);
    
    if (Asthra_result_is_ok(interp_result)) {
        AsthraFFIString *log_msg = (AsthraFFIString*)Asthra_result_unwrap_ok(interp_result);
        printf("Log message: %s\n", log_msg->data);
        
        // Convert to C string for external logging
        char *c_log_msg = Asthra_string_to_cstr(*log_msg, false);
        printf("C string: %s\n", c_log_msg);
        
        Asthra_string_free(*log_msg);
        free(log_msg);
    } else {
        printf("ERROR: String interpolation failed: %s\n", 
               Asthra_result_get_error_message(interp_result));
    }
    
    // String concatenation example
    AsthraFFIString prefix = Asthra_string_from_cstr("PQC-", ASTHRA_OWNERSHIP_TRANSFER_FULL);
    AsthraFFIString suffix = Asthra_string_from_cstr("-SECURE", ASTHRA_OWNERSHIP_TRANSFER_FULL);
    
    AsthraFFIResult concat_result = Asthra_string_concat(prefix, suffix);
    if (Asthra_result_is_ok(concat_result)) {
        AsthraFFIString *combined = (AsthraFFIString*)Asthra_result_unwrap_ok(concat_result);
        printf("Combined string: %s\n", combined->data);
        
        Asthra_string_free(*combined);
        free(combined);
    }
    
    // Cleanup
    Asthra_string_free(prefix);
    Asthra_string_free(suffix);
    Asthra_variant_array_free(args);
}

// Demonstrate error handling patterns
void demonstrate_error_handling(void) {
    printf("\n=== Error Handling Demo ===\n");
    
    // Create an invalid slice to trigger errors
    AsthraFFISliceHeader invalid_slice = {0};
    
    // Try to access invalid slice
    AsthraFFIResult bounds_result = Asthra_slice_bounds_check(invalid_slice, 0);
    
    if (Asthra_result_is_err(bounds_result)) {
        printf("Expected error caught: %s\n", 
               Asthra_result_get_error_message(bounds_result));
        printf("Error code: %d\n", 
               Asthra_result_get_error_code(bounds_result));
    }
    
    // Demonstrate error propagation chain
    printf("\nError propagation chain:\n");
    
    // Step 1: Create valid slice
    AsthraFFISliceHeader test_slice = Asthra_slice_new(sizeof(int), 5, 10, ASTHRA_ZONE_HINT_MANUAL);
    
    // Step 2: Try valid operation
    AsthraFFIResult step1 = Asthra_slice_bounds_check(test_slice, 2);
    if (Asthra_result_is_ok(step1)) {
        printf("  Step 1: Bounds check passed\n");
        
        // Step 3: Try invalid operation
        AsthraFFIResult step2 = Asthra_slice_bounds_check(test_slice, 100);
        if (Asthra_result_is_err(step2)) {
            printf("  Step 2: Bounds check failed as expected: %s\n",
                   Asthra_result_get_error_message(step2));
        }
    }
    
    Asthra_slice_free(test_slice);
}

// Demonstrate ownership tracking
void demonstrate_ownership_tracking(void) {
    printf("\n=== Ownership Tracking Demo ===\n");
    
    // Allocate external memory
    void *external_buffer = malloc(1024);
    printf("Allocated external buffer: %p\n", external_buffer);
    
    // Register for ownership tracking
    AsthraFFIResult reg_result = Asthra_ownership_register(
        external_buffer, 1024, ASTHRA_OWNERSHIP_TRANSFER_FULL, free);
    
    if (Asthra_result_is_ok(reg_result)) {
        printf("✓ Buffer registered for ownership tracking\n");
        
        // Query ownership
        AsthraFFIResult query_result = Asthra_ownership_query(external_buffer);
        if (Asthra_result_is_ok(query_result)) {
            printf("✓ Ownership query successful\n");
            free(Asthra_result_unwrap_ok(query_result));
        }
        
        // Transfer ownership
        AsthraFFIResult transfer_result = Asthra_ownership_transfer(
            external_buffer, ASTHRA_OWNERSHIP_TRANSFER_NONE);
        
        if (Asthra_result_is_ok(transfer_result)) {
            printf("✓ Ownership transferred to borrowed\n");
            free(Asthra_result_unwrap_ok(transfer_result));
        }
        
        // Unregister
        AsthraFFIResult unreg_result = Asthra_ownership_unregister(external_buffer);
        if (Asthra_result_is_ok(unreg_result)) {
            printf("✓ Buffer unregistered\n");
        }
        
        // Manual cleanup since we unregistered
        free(external_buffer);
    } else {
        printf("ERROR: Failed to register buffer: %s\n",
               Asthra_result_get_error_message(reg_result));
        free(external_buffer);
    }
}

// Display memory statistics
void display_memory_statistics(void) {
    printf("\n=== Memory Statistics ===\n");
    
    AsthraFFIMemoryStats stats = Asthra_ffi_get_memory_stats();
    
    printf("Total allocations: %zu\n", stats.total_allocations);
    printf("Total frees: %zu\n", stats.total_frees);
    printf("Current allocations: %zu\n", stats.current_allocations);
    printf("Peak allocations: %zu\n", stats.peak_allocations);
    printf("Bytes allocated: %zu\n", stats.bytes_allocated);
    printf("Bytes freed: %zu\n", stats.bytes_freed);
    printf("Current bytes: %zu\n", stats.current_bytes);
    printf("Peak bytes: %zu\n", stats.peak_bytes);
    printf("Active slices: %zu\n", stats.slice_count);
    printf("Active strings: %zu\n", stats.string_count);
    printf("Active results: %zu\n", stats.result_count);
    
    // Validate all pointers
    AsthraFFIResult validation_result = Asthra_ffi_validate_all_pointers();
    if (Asthra_result_is_ok(validation_result)) {
        int *count = (int*)Asthra_result_unwrap_ok(validation_result);
        printf("Validated pointers: %d\n", *count);
        free(count);
    } else {
        printf("Pointer validation failed: %s\n",
               Asthra_result_get_error_message(validation_result));
    }
}

int main(void) {
    printf("Asthra Safe C Memory Interface Example\n");
    printf("======================================\n");
    
    // Initialize Asthra runtime
    AsthraGCConfig gc_config = {
        .initial_heap_size = 2 * 1024 * 1024,    // 2MB
        .max_heap_size = 32 * 1024 * 1024,       // 32MB
        .gc_threshold = 0.8,
        .conservative_mode = true,
        .concurrent_gc = false
    };
    
    if (asthra_runtime_init(&gc_config) != 0) {
        printf("Failed to initialize Asthra runtime\n");
        return 1;
    }
    
    printf("✓ Asthra runtime initialized\n");
    
    // Reset statistics for clean demo
    Asthra_ffi_reset_memory_stats();
    
    // Run demonstrations
    demonstrate_pqc_key_management();
    demonstrate_string_operations();
    demonstrate_error_handling();
    demonstrate_ownership_tracking();
    
    // Show final statistics
    display_memory_statistics();
    
    // Dump memory state for debugging
    printf("\n=== Final Memory State ===\n");
    Asthra_ffi_dump_memory_state(stdout);
    
    // Cleanup runtime
    asthra_runtime_cleanup();
    printf("\n✓ Asthra runtime cleaned up\n");
    
    printf("\nExample completed successfully!\n");
    return 0;
} 
