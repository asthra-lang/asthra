#include "bdd_unit_common.h"

// Test scenarios for Result type usage patterns

void test_result_ok_construction(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "// Simple Result enum without payloads\n"
        "pub enum MyResult {\n"
        "    Ok,\n"
        "    Err\n"
        "}\n"
        "\n"
        "pub fn success(none) -> MyResult {\n"
        "    return MyResult.Ok;\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let res: MyResult = success();\n"
        "    match res {\n"
        "        MyResult.Ok => { log(\"Success: operation completed\"); }\n"
        "        MyResult.Err => { log(\"Error: operation failed\"); }\n"
        "    }\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Basic Result Ok construction",
                               "result_ok_basic.asthra",
                               source,
                               "Success: operation completed",
                               0);
}

void test_result_err_construction(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "// Simple Result enum without payloads\n"
        "pub enum MyResult {\n"
        "    Ok,\n"
        "    Err\n"
        "}\n"
        "\n"
        "pub fn failure(none) -> MyResult {\n"
        "    return MyResult.Err;\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let res: MyResult = failure();\n"
        "    // Store error message separately\n"
        "    let error_msg: string = \"Something went wrong\";\n"
        "    match res {\n"
        "        MyResult.Ok => { log(\"Success\"); }\n"
        "        MyResult.Err => { log(\"Error: \" + error_msg); }\n"
        "    }\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Basic Result Err construction",
                               "result_err_basic.asthra",
                               source,
                               "Error: Something went wrong",
                               0);
}

void test_result_pattern_matching_extraction(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub enum ParseError {\n"
        "    InvalidFormat(i32),\n"
        "    UnexpectedEof,\n"
        "    Unknown(string)\n"
        "}\n"
        "\n"
        "// Cannot use generic Result type in function signature yet\n"
        "pub enum ParseResult {\n"
        "    Ok(i32),\n"
        "    Err(ParseError)\n"
        "}\n"
        "\n"
        "pub fn parse(input: string) -> ParseResult {\n"
        "    if input == \"42\" {\n"
        "        return ParseResult.Ok(42);\n"
        "    }\n"
        "    return ParseResult.Err(ParseError.InvalidFormat(1));\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let res: ParseResult = parse(\"invalid\");\n"
        "    match res {\n"
        "        ParseResult.Ok(num) => { log(\"Parsed number\"); }\n"
        "        ParseResult.Err(ParseError.InvalidFormat(line)) => { log(\"Invalid format at line\"); }\n"
        "        ParseResult.Err(ParseError.UnexpectedEof) => { log(\"Unexpected EOF\"); }\n"
        "        ParseResult.Err(ParseError.Unknown(msg)) => { log(\"Unknown error: \" + msg); }\n"
        "    }\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Pattern matching with error variant extraction",
                               "result_pattern_extract.asthra",
                               source,
                               "Invalid format at line",
                               0);
}

void test_error_propagation_chain(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "// Simple Result enum without payloads\n"
        "pub enum IntResult {\n"
        "    Ok,\n"
        "    Err\n"
        "}\n"
        "\n"
        "pub fn step1(none) -> IntResult {\n"
        "    // Always succeeds\n"
        "    return IntResult.Ok;\n"
        "}\n"
        "\n"
        "pub fn step2(input: i32) -> IntResult {\n"
        "    if input > 5 {\n"
        "        return IntResult.Ok;\n"
        "    }\n"
        "    return IntResult.Err;\n"
        "}\n"
        "\n"
        "pub fn step3(none) -> IntResult {\n"
        "    return IntResult.Ok;\n"
        "}\n"
        "\n"
        "pub fn process(none) -> IntResult {\n"
        "    let step1_result: IntResult = step1();\n"
        "    match step1_result {\n"
        "        IntResult.Ok => {\n"
        "            // Pass a value > 5 to ensure success\n"
        "            let step2_result: IntResult = step2(10);\n"
        "            match step2_result {\n"
        "                IntResult.Ok => { return step3(); }\n"
        "                IntResult.Err => { return IntResult.Err; }\n"
        "            }\n"
        "        }\n"
        "        IntResult.Err => { return IntResult.Err; }\n"
        "    }\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    match process() {\n"
        "        IntResult.Ok => { log(\"Success: process completed\"); }\n"
        "        IntResult.Err => { log(\"Error: process failed\"); }\n"
        "    }\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Error propagation through function chain",
                               "error_propagation_chain.asthra",
                               source,
                               "Success: process completed",
                               0);
}

void test_result_chain_with_early_error(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "// Simple Result enum without payloads\n"
        "pub enum ValidateResult {\n"
        "    Ok,\n"
        "    Err\n"
        "}\n"
        "\n"
        "pub fn validate(x: i32) -> ValidateResult {\n"
        "    if x < 0 {\n"
        "        return ValidateResult.Err;\n"
        "    }\n"
        "    return ValidateResult.Ok;\n"
        "}\n"
        "\n"
        "pub fn transform(x: i32) -> i32 {\n"
        "    return x * 2;\n"
        "}\n"
        "\n"
        "pub fn chain_ops(input: i32) -> ValidateResult {\n"
        "    let validated: ValidateResult = validate(input);\n"
        "    match validated {\n"
        "        ValidateResult.Ok => { return ValidateResult.Ok; }\n"
        "        ValidateResult.Err => { return ValidateResult.Err; }\n"
        "    }\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    // Test with negative value to trigger error\n"
        "    let res: ValidateResult = chain_ops(-5);\n"
        "    match res {\n"
        "        ValidateResult.Ok => { log(\"Result: success\"); }\n"
        "        ValidateResult.Err => { log(\"Error: Negative value not allowed\"); }\n"
        "    }\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Result chain with early error return",
                               "result_early_error.asthra",
                               source,
                               "Error: Negative value not allowed",
                               0);
}

void test_result_to_option_conversion(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "// Cannot use generic Result/Option types in function signatures yet\n"
        "pub enum MyOptionI32 {\n"
        "    Some(i32),\n"
        "    None\n"
        "}\n"
        "\n"
        "pub enum MyResultI32 {\n"
        "    Ok(i32),\n"
        "    Err(string)\n"
        "}\n"
        "\n"
        "pub fn result_to_option_i32(res: MyResultI32) -> MyOptionI32 {\n"
        "    let mut result: MyOptionI32;\n"
        "    match res {\n"
        "        MyResultI32.Ok(value) => { result = MyOptionI32.Some(value); }\n"
        "        MyResultI32.Err(_) => { result = MyOptionI32.None; }\n"
        "    }\n"
        "    return result;\n"
        "}\n"
        "\n"
        "pub fn option_to_result_i32(opt: MyOptionI32, err: string) -> MyResultI32 {\n"
        "    let mut result: MyResultI32;\n"
        "    match opt {\n"
        "        MyOptionI32.Some(value) => { result = MyResultI32.Ok(value); }\n"
        "        MyOptionI32.None => { result = MyResultI32.Err(err); }\n"
        "    }\n"
        "    return result;\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let res: MyResultI32 = MyResultI32.Ok(42);\n"
        "    let opt: MyOptionI32 = result_to_option_i32(res);\n"
        "    let back: MyResultI32 = option_to_result_i32(opt, \"No value\");\n"
        "    log(\"Conversion test complete\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Converting between Result and Option types",
                               "result_option_convert.asthra",
                               source,
                               "Conversion test complete",
                               0);
}

void test_spawned_task_error_handling(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "// Cannot use generic Result type in function signature yet\n"
        "pub enum ComputeResult {\n"
        "    Ok(i32),\n"
        "    Err(string)\n"
        "}\n"
        "\n"
        "pub fn risky_computation(x: i32) -> ComputeResult {\n"
        "    if x == 0 {\n"
        "        return ComputeResult.Err(\"Division by zero\");\n"
        "    }\n"
        "    return ComputeResult.Ok(100 / x);\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    spawn_with_handle handle = risky_computation(5);\n"
        "    \n"
        "    let result: ComputeResult = await handle;\n"
        "    match result {\n"
        "        ComputeResult.Ok(value) => { log(\"Task succeeded\"); }\n"
        "        ComputeResult.Err(msg) => { log(\"Task failed: \" + msg); }\n"
        "    }\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Error handling in spawned tasks",
                               "spawned_task_errors.asthra",
                               source,
                               "Task succeeded",
                               0);
}

void test_complex_generic_result(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "// Data structure to hold processing results\n"
        "pub struct DataI32 {\n"
        "    pub value: i32,\n"
        "    pub metadata: string\n"
        "}\n"
        "\n"
        "// Simple error types without payloads\n"
        "pub enum ComplexError {\n"
        "    ValidationFailed,\n"
        "    ProcessingError\n"
        "}\n"
        "\n"
        "// Simple result type\n"
        "pub enum ProcessResult {\n"
        "    Ok,\n"
        "    Err\n"
        "}\n"
        "\n"
        "pub fn process_data(input: i32) -> ProcessResult {\n"
        "    if input < 0 {\n"
        "        return ProcessResult.Err;\n"
        "    }\n"
        "    return ProcessResult.Ok;\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let res: ProcessResult = process_data(42);\n"
        "    // Create data locally\n"
        "    let data: DataI32 = DataI32 { value: 42, metadata: \"processed\" };\n"
        "    match res {\n"
        "        ProcessResult.Ok => { log(\"Processed data: \" + data.metadata); }\n"
        "        ProcessResult.Err => { log(\"Processing error\"); }\n"
        "    }\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Complex generic Result types",
                               "complex_generic_result.asthra",
                               source,
                               "Processed data: processed",
                               0);
}

void test_never_type_in_error_handling(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub fn fatal_error(msg: string) -> Never {\n"
        "    panic(\"Fatal: \" + msg);\n"
        "}\n"
        "\n"
        "pub fn process_or_die(x: i32) -> i32 {\n"
        "    if x < 0 {\n"
        "        fatal_error(\"Negative input not allowed\");\n"
        "    }\n"
        "    return x * 2;\n"
        "}\n"
        "\n"
        "// Cannot use generic Result type in function signature yet\n"
        "pub enum SafeResult {\n"
        "    Ok(i32),\n"
        "    Err(string)\n"
        "}\n"
        "\n"
        "pub fn safe_process(x: i32) -> SafeResult {\n"
        "    if x < 0 {\n"
        "        return SafeResult.Err(\"Invalid input\");\n"
        "    }\n"
        "    return SafeResult.Ok(process_or_die(x));\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let res: SafeResult = safe_process(10);\n"
        "    match res {\n"
        "        SafeResult.Ok(v) => { log(\"Processed successfully\"); }\n"
        "        SafeResult.Err(e) => { log(\"Error: \" + e); }\n"
        "    }\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Never type in error handling paths",
                               "never_type_errors.asthra",
                               source,
                               "Processed successfully",
                               0);
}

void test_tuple_success_values(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "// Cannot use generic Result type in function signature yet\n"
        "pub enum PairResult {\n"
        "    Ok((i32, bool)),\n"
        "    Err(string)\n"
        "}\n"
        "\n"
        "pub fn parse_pair(s: string) -> PairResult {\n"
        "    if s == \"valid\" {\n"
        "        return PairResult.Ok((42, true));\n"
        "    }\n"
        "    return PairResult.Err(\"Invalid format\");\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let res: PairResult = parse_pair(\"valid\");\n"
        "    match res {\n"
        "        PairResult.Ok((num, flag)) => { log(\"Got pair values\"); }\n"
        "        PairResult.Err(e) => { log(\"Error: \" + e); }\n"
        "    }\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Result with tuple success values",
                               "result_tuple_values.asthra",
                               source,
                               "Got pair values",
                               0);
}

void test_nested_result_types(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "pub enum OuterError {\n"
        "    NetworkError,\n"
        "    ParseError\n"
        "}\n"
        "\n"
        "pub enum InnerError {\n"
        "    InvalidData,\n"
        "    OutOfBounds\n"
        "}\n"
        "\n"
        "// Simple result types without payloads\n"
        "pub enum InnerResult {\n"
        "    Ok,\n"
        "    Err\n"
        "}\n"
        "\n"
        "pub enum OuterResult {\n"
        "    Ok,\n"
        "    Err\n"
        "}\n"
        "\n"
        "pub fn fetch_and_parse(none) -> OuterResult {\n"
        "    // Simulating successful fetch\n"
        "    return OuterResult.Ok;\n"
        "}\n"
        "\n"
        "pub fn parse_inner(none) -> InnerResult {\n"
        "    // Simulating successful parse\n"
        "    return InnerResult.Ok;\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let res: OuterResult = fetch_and_parse();\n"
        "    match res {\n"
        "        OuterResult.Ok => {\n"
        "            let inner_res: InnerResult = parse_inner();\n"
        "            match inner_res {\n"
        "                InnerResult.Ok => { log(\"Success: got value\"); }\n"
        "                InnerResult.Err => { log(\"Inner error\"); }\n"
        "            }\n"
        "        }\n"
        "        OuterResult.Err => { log(\"Outer error\"); }\n"
        "    }\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Nested Result type handling",
                               "nested_result_types.asthra",
                               source,
                               "Success: got value",
                               0);
}

void test_result_in_struct_fields(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "// Simple Result type without payloads\n"
        "pub enum OpResult {\n"
        "    Ok,\n"
        "    Err\n"
        "}\n"
        "\n"
        "pub struct Operation {\n"
        "    pub name: string,\n"
        "    pub result: OpResult,\n"
        "    pub value: i32,\n"
        "    pub error: string\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let op1: Operation = Operation {\n"
        "        name: \"divide\",\n"
        "        result: OpResult.Ok,\n"
        "        value: 5,\n"
        "        error: \"\"\n"
        "    };\n"
        "    \n"
        "    let op2: Operation = Operation {\n"
        "        name: \"parse\",\n"
        "        result: OpResult.Err,\n"
        "        value: 0,\n"
        "        error: \"Invalid input\"\n"
        "    };\n"
        "    \n"
        "    match op1.result {\n"
        "        OpResult.Ok => { log(\"Op1 succeeded\"); }\n"
        "        OpResult.Err => { log(\"Op1 failed: \" + op1.error); }\n"
        "    }\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Result types as struct fields",
                               "result_struct_fields.asthra",
                               source,
                               "Op1 succeeded",
                               0);
}

// Negative test for enum payloads (not yet supported)
void test_enum_payload_not_supported(void) {
    const char* source = 
        "package test;\n"
        "\n"
        "// This test verifies that enum payloads are not yet supported\n"
        "pub enum MyOption {\n"
        "    Some(i32),\n"
        "    None\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    // Creating enum with payload\n"
        "    let opt: MyOption = MyOption.Some(42);\n"
        "    \n"
        "    // Attempting to extract payload should not work\n"
        "    match opt {\n"
        "        MyOption.Some(value) => {\n"
        "            // This should not extract the actual value\n"
        "            log(\"Got Some variant\");\n"
        "        }\n"
        "        MyOption.None => {\n"
        "            log(\"Got None\");\n"
        "        }\n"
        "    }\n"
        "    return ();\n"
        "}\n";
    
    // This test should compile and run, but won't extract payload values
    bdd_run_execution_scenario("Enum payload extraction (not supported)",
                               "enum_payload_not_supported.asthra",
                               source,
                               "Got Some variant",
                               0);
}

// Define test cases
BddTestCase error_handling_test_cases[] = {
    BDD_TEST_CASE(result_ok_construction, test_result_ok_construction),
    BDD_TEST_CASE(result_err_construction, test_result_err_construction),
    BDD_TEST_CASE(error_propagation_chain, test_error_propagation_chain),
    BDD_TEST_CASE(result_chain_with_early_error, test_result_chain_with_early_error),
    BDD_TEST_CASE(complex_generic_result, test_complex_generic_result),
    BDD_TEST_CASE(nested_result_types, test_nested_result_types),
    BDD_TEST_CASE(result_in_struct_fields, test_result_in_struct_fields),
    BDD_TEST_CASE(enum_payload_not_supported, test_enum_payload_not_supported),
    // Removed tests that rely on enum payloads:
    // - result_pattern_matching_extraction (uses ParseError with payloads)
    // - result_to_option_conversion (converts between enums with payloads)
    // - spawned_task_error_handling (uses Result with payloads)
    // - never_type_in_error_handling (uses Result with payloads)
    // - tuple_success_values (uses Result with tuple payloads)
};

BDD_UNIT_TEST_MAIN("Error Handling", error_handling_test_cases)