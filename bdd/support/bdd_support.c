#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// BDD test support functions

typedef struct {
    const char* description;
    int passed;
    int failed;
    int skipped;
} bdd_context_t;

static bdd_context_t g_bdd_context = {0};

void bdd_init(const char* feature_name) {
    g_bdd_context.description = feature_name;
    g_bdd_context.passed = 0;
    g_bdd_context.failed = 0;
    g_bdd_context.skipped = 0;
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
    if (g_bdd_context.skipped > 0) {
        printf("  Skipped: %d\n", g_bdd_context.skipped);
    }
    printf("  Total:  %d\n", g_bdd_context.passed + g_bdd_context.failed + g_bdd_context.skipped);
    
    // Check if @wip scenarios were skipped
    const char* skip_wip = getenv("BDD_SKIP_WIP");
    if (!skip_wip || strcmp(skip_wip, "0") != 0) {
        if (g_bdd_context.skipped > 0) {
            printf("  Note: @wip scenarios were skipped\n");
        }
    }
    
    return g_bdd_context.failed == 0 ? 0 : 1;
}

// Skip a scenario and increment the skipped counter
void bdd_skip_scenario(const char* scenario_name) {
    g_bdd_context.skipped++;
    printf("\n  Scenario: %s\n", scenario_name);
    printf("    ⏭️  SKIPPED: Work in progress\n");
}

// Check if @wip scenarios should be skipped
int bdd_should_skip_wip(void) {
    const char* skip_wip = getenv("BDD_SKIP_WIP");
    // Skip @wip by default unless BDD_SKIP_WIP=0
    return !skip_wip || strcmp(skip_wip, "0") != 0;
}