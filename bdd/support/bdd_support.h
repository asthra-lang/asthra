#ifndef BDD_SUPPORT_H
#define BDD_SUPPORT_H

// BDD test support interface

void bdd_init(const char* feature_name);
void bdd_scenario(const char* scenario_name);
void bdd_given(const char* condition);
void bdd_when(const char* action);
void bdd_then(const char* expectation);
void bdd_assert(int condition, const char* message);
int bdd_report(void);

// Helper functions for @wip scenario handling
void bdd_skip_scenario(const char* scenario_name);
int bdd_should_skip_wip(void);

// Convenience macros
#define BDD_ASSERT_TRUE(expr) bdd_assert((expr), #expr " should be true")
#define BDD_ASSERT_FALSE(expr) bdd_assert(!(expr), #expr " should be false")
#define BDD_ASSERT_EQ(a, b) bdd_assert((a) == (b), #a " should equal " #b)
#define BDD_ASSERT_NE(a, b) bdd_assert((a) != (b), #a " should not equal " #b)
#define BDD_ASSERT_NULL(ptr) bdd_assert((ptr) == NULL, #ptr " should be NULL")
#define BDD_ASSERT_NOT_NULL(ptr) bdd_assert((ptr) != NULL, #ptr " should not be NULL")

// Macro to conditionally run scenarios based on @wip tag
#define BDD_RUN_SCENARIO(func, name, is_wip) \
    do { \
        if ((is_wip) && bdd_should_skip_wip()) { \
            bdd_skip_scenario(name " [@wip]"); \
        } else { \
            func(); \
        } \
    } while(0)

#endif // BDD_SUPPORT_H