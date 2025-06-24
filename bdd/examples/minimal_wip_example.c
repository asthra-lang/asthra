// Example: Minimal changes to support @wip tag filtering
// This shows how to update existing tests with minimal modifications

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "bdd_support.h"

// Include the WIP patch functionality
extern bool bdd_should_skip_scenario(const char* tags);

// Macro to conditionally run scenarios based on tags
#define RUN_SCENARIO_IF_NOT_WIP(func, name, is_wip) \
    do { \
        if (is_wip && getenv("BDD_SKIP_WIP") != "0") { \
            printf("\n  Scenario: %s [@wip]\n", name); \
            printf("    ⏭️  SKIPPED: Work in progress\n"); \
        } else { \
            func(); \
        } \
    } while(0)

// Test functions remain unchanged
void test_working_scenario(void) {
    bdd_scenario("Working scenario");
    bdd_given("a working implementation");
    bdd_when("I run the test");
    bdd_then("it should pass");
    bdd_assert(1, "This scenario works");
}

void test_wip_scenario(void) {
    bdd_scenario("WIP scenario");
    bdd_given("an incomplete implementation");
    bdd_when("I run the test");
    bdd_then("it might fail");
    bdd_assert(0, "This scenario is not ready yet");
}

int main(void) {
    bdd_init("Example Feature with WIP support");
    
    // Method 1: Simple manual approach
    test_working_scenario();
    
    // Skip @wip scenarios unless BDD_SKIP_WIP=0
    if (getenv("BDD_SKIP_WIP") == NULL || strcmp(getenv("BDD_SKIP_WIP"), "0") != 0) {
        printf("\n  Scenario: WIP scenario [@wip]\n");
        printf("    ⏭️  SKIPPED: Work in progress\n");
    } else {
        test_wip_scenario();
    }
    
    // Method 2: Using the macro
    RUN_SCENARIO_IF_NOT_WIP(test_working_scenario, "Working scenario", false);
    RUN_SCENARIO_IF_NOT_WIP(test_wip_scenario, "WIP scenario", true);
    
    return bdd_report();
}

/*
Usage examples:
1. Run with @wip scenarios skipped (default):
   ./test

2. Run all scenarios including @wip:
   BDD_SKIP_WIP=0 ./test

3. Run only @wip scenarios:
   BDD_TAG_FILTER="@wip" ./test

4. Skip scenarios with @slow tag:
   BDD_TAG_FILTER="not @slow" ./test
*/