# Test Framework API

### 1. Test Suite Management (`test_suite.h/c`)

**Purpose**: Manages collections of tests with configuration and execution control.

```c
typedef struct AsthraTestSuite AsthraTestSuite;

typedef struct {
    const char *name;
    const char *description;
    bool lightweight_mode;
    bool custom_main;
    bool statistics_tracking;
    AsthraTestReportingLevel reporting_level;
    uint64_t default_timeout_ns;
    bool parallel_execution;
} AsthraTestSuiteConfig;
```

**Key Features**:
- Dynamic test registration
- Configurable execution modes
- Setup/teardown support
- Custom analysis callbacks
- Resource management

### 2. Test Context Management (`test_context.h/c`)

**Purpose**: Provides execution context and state management for individual tests.

```c
typedef struct {
    AsthraTestMetadata metadata;
    AsthraTestStatistics *stats;
    const char *error_message;
    AsthraTestResult result;
    uint64_t start_time_ns;
    uint64_t end_time_ns;
    void *user_data;
} AsthraTestContext;
```

**Key Features**:
- Test metadata tracking
- Timing measurement
- Error message management
- User data support for custom frameworks
- Resource lifecycle management

### 3. Assertion System (`test_assertions.h/c`)

**Purpose**: Provides comprehensive assertion macros with rich error reporting.

```c
// Core assertion types
#define ASTHRA_TEST_ASSERT_TRUE(context, condition, message)
#define ASTHRA_TEST_ASSERT_EQ(context, actual, expected, message)
#define ASTHRA_TEST_ASSERT_STR_EQ(context, actual, expected, message)
#define ASTHRA_TEST_ASSERT_NOT_NULL(context, pointer, message)

// Advanced assertions
#define ASTHRA_TEST_ASSERT_IN_RANGE(context, value, min, max, message)
#define ASTHRA_TEST_ASSERT_FLOAT_EQ(context, actual, expected, tolerance, message)
#define ASTHRA_TEST_ASSERT_MEMORY_EQ(context, ptr1, ptr2, size, message)
```

**Key Features**:
- Type-generic assertions using `_Generic`
- Rich error messages with context
- Performance optimized for common cases
- Extensible for custom assertion types

### 4. Statistics and Reporting (`test_statistics.h/c`)

**Purpose**: Collects, analyzes, and reports test execution metrics.

```c
typedef struct {
    size_t total_tests;
    size_t passed_tests;
    size_t failed_tests;
    size_t skipped_tests;
    size_t error_tests;
    uint64_t total_time_ns;
    double pass_rate;
    size_t total_assertions;
    size_t failed_assertions;
} AsthraTestStatistics;
```

**Key Features**:
- Comprehensive metrics collection
- Real-time statistics updates
- Custom analysis support
- Multiple output formats (text, JSON, XML) 