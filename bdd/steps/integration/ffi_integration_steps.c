#include <stdio.h>
#include <stdlib.h>
#include "bdd_support.h"

// Sample BDD integration test for FFI functionality

typedef struct {
    void* handle;
    int (*c_function)(int);
} ffi_context_t;

static ffi_context_t ffi_ctx = {0};
static int call_result = 0;

void given_c_library_loaded(void) {
    bdd_given("a C library is loaded");
    // Simulate library loading
    ffi_ctx.handle = (void*)0x1234; // Mock handle
    ffi_ctx.c_function = NULL;
}

void when_asthra_calls_c_function(void) {
    bdd_when("Asthra code calls a C function");
    // Simulate FFI call
    if (ffi_ctx.handle) {
        call_result = 42; // Mock result
    }
}

void then_ffi_call_succeeds(void) {
    bdd_then("the FFI call succeeds");
    BDD_ASSERT_NOT_NULL(ffi_ctx.handle);
    BDD_ASSERT_EQ(call_result, 42);
}

int main(void) {
    bdd_init("FFI Integration");
    
    bdd_scenario("Call C function from Asthra");
    given_c_library_loaded();
    when_asthra_calls_c_function();
    then_ffi_call_succeeds();
    
    return bdd_report();
}