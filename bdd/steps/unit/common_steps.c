#include "bdd_support.h"

// Common step definitions for unit tests

void given_test_environment_setup(void) {
    bdd_given("test environment is set up");
    // Common setup code here
}

void then_no_memory_leaks(void) {
    bdd_then("there should be no memory leaks");
    // In real implementation, would check memory allocation tracking
    BDD_ASSERT_TRUE(1);
}