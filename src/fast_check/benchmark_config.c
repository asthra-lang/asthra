/*
 * Asthra Fast Check Benchmark Configuration
 * Week 16: Performance Optimization & Testing
 *
 * Benchmark suites configuration and test code templates.
 */

#include "benchmark_internal.h"
#include <string.h>

// =============================================================================
// Benchmark Configuration
// =============================================================================

static const BenchmarkSuite BENCHMARK_SUITES[] = {
    {"single_file", "Single file performance", 100.0, 1, "simple"},
    {"medium_project", "Medium project (20 files)", 500.0, 20, "medium"},
    {"large_project", "Large project (50 files)", 2000.0, 50, "complex"},
    {"cache_performance", "Cache optimization test", 50.0, 10, "medium"}};

static const int BENCHMARK_SUITE_COUNT = sizeof(BENCHMARK_SUITES) / sizeof(BENCHMARK_SUITES[0]);

// =============================================================================
// Test Code Templates
// =============================================================================

static const char *SIMPLE_CODE_TEMPLATE = "package benchmark_%d;\n"
                                          "\n"
                                          "pub struct Point {\n"
                                          "    x: f64,\n"
                                          "    y: f64\n"
                                          "}\n"
                                          "\n"
                                          "pub fn distance(p1: Point, p2: Point) -> f64 {\n"
                                          "    let dx: f64 = p1.x - p2.x;\n"
                                          "    let dy: f64 = p1.y - p2.y;\n"
                                          "    return sqrt(dx * dx + dy * dy);\n"
                                          "}\n";

static const char *MEDIUM_CODE_TEMPLATE =
    "package benchmark_%d;\n"
    "\n"
    "pub struct User {\n"
    "    id: u64,\n"
    "    name: string,\n"
    "    email: string,\n"
    "    active: bool\n"
    "}\n"
    "\n"
    "pub enum Status {\n"
    "    Active,\n"
    "    Inactive,\n"
    "    Pending,\n"
    "    Suspended\n"
    "}\n"
    "\n"
    "pub fn validate_user(user: User) -> Result<bool, string> {\n"
    "    if user.name.len() == 0 {\n"
    "        return Result.Err(\"Name cannot be empty\");\n"
    "    }\n"
    "    \n"
    "    if !user.email.contains(\"@\") {\n"
    "        return Result.Err(\"Invalid email format\");\n"
    "    }\n"
    "    \n"
    "    return Result.Ok(true);\n"
    "}\n"
    "\n"
    "pub fn process_users(users: []User) -> []User {\n"
    "    let valid_users: []User = [];\n"
    "    \n"
    "    for user in users {\n"
    "        match validate_user(user) {\n"
    "            Result.Ok(valid) => {\n"
    "                if valid {\n"
    "                    valid_users.push(user);\n"
    "                }\n"
    "            },\n"
    "            Result.Err(error) => {\n"
    "                log(\"Validation error: \" + error);\n"
    "            }\n"
    "        }\n"
    "    }\n"
    "    \n"
    "    return valid_users;\n"
    "}\n";

static const char *COMPLEX_CODE_TEMPLATE =
    "package benchmark_%d;\n"
    "\n"
    "pub struct Database {\n"
    "    connections: u32,\n"
    "    max_connections: u32,\n"
    "    active_queries: []Query,\n"
    "    cache_size: u64\n"
    "}\n"
    "\n"
    "pub struct Query {\n"
    "    id: u64,\n"
    "    sql: string,\n"
    "    parameters: []string,\n"
    "    start_time: u64,\n"
    "    timeout_ms: u32\n"
    "}\n"
    "\n"
    "pub enum QueryResult {\n"
    "    Success([]Row),\n"
    "    Error(string),\n"
    "    Timeout\n"
    "}\n"
    "\n"
    "pub struct Row {\n"
    "    columns: []string,\n"
    "    values: []string\n"
    "}\n"
    "\n"
    "pub fn execute_query(db: Database, query: Query) -> Result<QueryResult, string> {\n"
    "    if db.connections >= db.max_connections {\n"
    "        return Result.Err(\"Database connection limit exceeded\");\n"
    "    }\n"
    "    \n"
    "    if query.sql.len() == 0 {\n"
    "        return Result.Err(\"Empty query not allowed\");\n"
    "    }\n"
    "    \n"
    "    // Simulate query processing\n"
    "    let rows: []Row = [];\n"
    "    \n"
    "    for i in range(0, 10) {\n"
    "        let row: Row = Row {\n"
    "            columns: [\"id\", \"name\", \"value\"],\n"
    "            values: [i.to_string(), \"test_\" + i.to_string(), (i * 2).to_string()]\n"
    "        };\n"
    "        rows.push(row);\n"
    "    }\n"
    "    \n"
    "    return Result.Ok(QueryResult.Success(rows));\n"
    "}\n"
    "\n"
    "pub fn batch_execute(db: Database, queries: []Query) -> []Result<QueryResult, string> {\n"
    "    let results: []Result<QueryResult, string> = [];\n"
    "    \n"
    "    for query in queries {\n"
    "        let result: Result<QueryResult, string> = execute_query(db, query);\n"
    "        results.push(result);\n"
    "    }\n"
    "    \n"
    "    return results;\n"
    "}\n";

// =============================================================================
// Configuration Access Functions
// =============================================================================

const BenchmarkSuite *get_benchmark_suites(void) {
    return BENCHMARK_SUITES;
}

int get_benchmark_suite_count(void) {
    return BENCHMARK_SUITE_COUNT;
}

const char *get_code_template(const char *complexity) {
    if (strcmp(complexity, "simple") == 0) {
        return SIMPLE_CODE_TEMPLATE;
    } else if (strcmp(complexity, "medium") == 0) {
        return MEDIUM_CODE_TEMPLATE;
    } else if (strcmp(complexity, "complex") == 0) {
        return COMPLEX_CODE_TEMPLATE;
    }
    return SIMPLE_CODE_TEMPLATE;
}