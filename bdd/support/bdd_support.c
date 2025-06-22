#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// BDD test support functions

typedef struct {
    const char* description;
    int passed;
    int failed;
} bdd_context_t;

static bdd_context_t g_bdd_context = {0};

void bdd_init(const char* feature_name) {
    g_bdd_context.description = feature_name;
    g_bdd_context.passed = 0;
    g_bdd_context.failed = 0;
    printf("\nFeature: %s\n", feature_name);
}

void bdd_scenario(const char* scenario_name) {
    printf("\n  Scenario: %s\n", scenario_name);
}

void bdd_given(const char* condition) {
    printf("    Given %s\n", condition);
}

void bdd_when(const char* action) {
    printf("    When %s\n", action);
}

void bdd_then(const char* expectation) {
    printf("    Then %s\n", expectation);
}

void bdd_assert(int condition, const char* message) {
    if (condition) {
        g_bdd_context.passed++;
        printf("      ✓ %s\n", message);
    } else {
        g_bdd_context.failed++;
        printf("      ✗ %s\n", message);
    }
}

int bdd_report(void) {
    printf("\n\nTest Summary for '%s':\n", g_bdd_context.description);
    printf("  Passed: %d\n", g_bdd_context.passed);
    printf("  Failed: %d\n", g_bdd_context.failed);
    printf("  Total:  %d\n", g_bdd_context.passed + g_bdd_context.failed);
    
    return g_bdd_context.failed == 0 ? 0 : 1;
}