#include "bdd_support.h"
#include "bdd_utilities.h"
#include "bdd_test_framework.h"
#include <sys/stat.h>

// Test scenarios for postfix expressions

// Function call tests
void test_basic_function_call_with_no_arguments(void) {
    const char* source = 
        "package test;\n"
        "pub fn greet(none) -> i32 {\n"
        "    return 42;\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "    return greet(none);\n"
        "}\n";
    
    bdd_run_execution_scenario("Basic function call with no arguments",
                               "function_call_no_args.asthra",
                               source,
                               NULL,
                               42);
}

void test_function_call_with_single_argument(void) {
    const char* source = 
        "package test;\n"
        "pub fn double(x: i32) -> i32 {\n"
        "    return x * 2;\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "    return double(21);\n"
        "}\n";
    
    bdd_run_execution_scenario("Function call with single argument",
                               "function_call_single_arg.asthra",
                               source,
                               NULL,
                               42);
}

void test_function_call_with_multiple_arguments(void) {
    const char* source = 
        "package test;\n"
        "pub fn add(a: i32, b: i32, c: i32) -> i32 {\n"
        "    return a + b + c;\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "    return add(10, 20, 12);\n"
        "}\n";
    
    bdd_run_execution_scenario("Function call with multiple arguments",
                               "function_call_multi_args.asthra",
                               source,
                               NULL,
                               42);
}

void test_nested_function_calls(void) {
    const char* source = 
        "package test;\n"
        "pub fn add(a: i32, b: i32) -> i32 {\n"
        "    return a + b;\n"
        "}\n"
        "pub fn double(x: i32) -> i32 {\n"
        "    return x * 2;\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "    return add(double(10), double(11));\n"
        "}\n";
    
    bdd_run_execution_scenario("Nested function calls",
                               "nested_function_calls.asthra",
                               source,
                               NULL,
                               42);
}

void test_function_call_with_expression_arguments(void) {
    const char* source = 
        "package test;\n"
        "pub fn calculate(x: i32, y: i32) -> i32 {\n"
        "    return x - y;\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "    let a: i32 = 50;\n"
        "    let b: i32 = 8;\n"
        "    return calculate(a, b);\n"
        "}\n";
    
    bdd_run_execution_scenario("Function call with expression arguments",
                               "function_call_expr_args.asthra",
                               source,
                               NULL,
                               42);
}

// Struct field access tests
void test_basic_struct_field_access(void) {
    const char* source = 
        "package test;\n"
        "pub struct Point {\n"
        "    x: i32,\n"
        "    y: i32\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "    let p: Point = Point { x: 42, y: 100 };\n"
        "    return p.x;\n"
        "}\n";
    
    bdd_run_execution_scenario("Basic struct field access",
                               "struct_field_access.asthra",
                               source,
                               NULL,
                               42);
}

void test_nested_struct_field_access(void) {
    const char* source = 
        "package test;\n"
        "pub struct Inner {\n"
        "    value: i32\n"
        "}\n"
        "pub struct Outer {\n"
        "    inner: Inner\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "    let i: Inner = Inner { value: 42 };\n"
        "    let o: Outer = Outer { inner: i };\n"
        "    return o.inner.value;\n"
        "}\n";
    
    bdd_run_execution_scenario("Nested struct field access",
                               "nested_struct_access.asthra",
                               source,
                               NULL,
                               42);
}

void test_struct_field_access_with_mutable_fields(void) {
    const char* source = 
        "package test;\n"
        "pub struct Counter {\n"
        "    count: i32\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "    let mut c: Counter = Counter { count: 40 };\n"
        "    c.count = 42;\n"
        "    return c.count;\n"
        "}\n";
    
    bdd_run_execution_scenario("Struct field access with mutable fields",
                               "struct_mutable_field.asthra",
                               source,
                               NULL,
                               42);
}

// Tuple access tests
void test_basic_tuple_element_access(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let t: (i32, i32) = (42, 100);\n"
        "    return t.0;\n"
        "}\n";
    
    bdd_run_execution_scenario("Basic tuple element access",
                               "tuple_element_access.asthra",
                               source,
                               NULL,
                               42);
}

void test_tuple_access_with_larger_index(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let t: (i32, i32, i32) = (10, 20, 42);\n"
        "    return t.2;\n"
        "}\n";
    
    bdd_run_execution_scenario("Tuple access with larger index",
                               "tuple_larger_index.asthra",
                               source,
                               NULL,
                               42);
}

void test_nested_tuple_access(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let t: ((i32, i32), i32) = ((42, 50), 100);\n"
        "    return t.0.0;\n"
        "}\n";
    
    bdd_run_execution_scenario("Nested tuple access",
                               "nested_tuple_access.asthra",
                               source,
                               NULL,
                               42);
}

// Array indexing tests
void test_basic_array_indexing(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let arr: [5]i32 = [10, 20, 30, 40, 42];\n"
        "    return arr[4];\n"
        "}\n";
    
    bdd_run_execution_scenario("Basic array indexing",
                               "array_indexing.asthra",
                               source,
                               NULL,
                               42);
}

void test_array_indexing_with_expression(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let arr: [5]i32 = [10, 20, 30, 40, 42];\n"
        "    let idx: i32 = 2 + 2;\n"
        "    return arr[idx];\n"
        "}\n";
    
    bdd_run_execution_scenario("Array indexing with expression",
                               "array_index_expr.asthra",
                               source,
                               NULL,
                               42);
}

void test_multi_dimensional_array_indexing(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let matrix: [3][3]i32 = [[1, 2, 3], [4, 5, 6], [7, 8, 42]];\n"
        "    return matrix[2][2];\n"
        "}\n";
    
    bdd_run_execution_scenario("Multi-dimensional array indexing",
                               "multi_dim_array.asthra",
                               source,
                               NULL,
                               42);
}

// Slice operations tests
void test_basic_slice_indexing(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let arr: []i32 = [10, 20, 30, 40, 42];\n"
        "    return arr[4];\n"
        "}\n";
    
    bdd_run_execution_scenario("Basic slice indexing",
                               "slice_indexing.asthra",
                               source,
                               NULL,
                               42);
}

void test_slice_range_from_start(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let arr: []i32 = [10, 20, 30, 40, 50];\n"
        "    let slice: []i32 = arr[2:];\n"
        "    return slice[0];  // Should be 30\n"
        "}\n";
    
    bdd_run_execution_scenario("Slice range from start",
                               "slice_from_start.asthra",
                               source,
                               NULL,
                               30);
}

void test_slice_range_to_end(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let arr: []i32 = [10, 20, 30, 40, 50];\n"
        "    let slice: []i32 = arr[:3];\n"
        "    return slice[2];  // Should be 30\n"
        "}\n";
    
    bdd_run_execution_scenario("Slice range to end",
                               "slice_to_end.asthra",
                               source,
                               NULL,
                               30);
}

void test_slice_with_start_and_end(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let arr: []i32 = [10, 20, 30, 42, 50];\n"
        "    let slice: []i32 = arr[2:4];\n"
        "    return slice[1];  // Should be 42\n"
        "}\n";
    
    bdd_run_execution_scenario("Slice with start and end",
                               "slice_start_end.asthra",
                               source,
                               NULL,
                               42);
}

void test_full_slice_copy(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let arr: []i32 = [10, 20, 30, 40, 42];\n"
        "    let copy: []i32 = arr[:];\n"
        "    return copy[4];\n"
        "}\n";
    
    bdd_run_execution_scenario("Full slice copy",
                               "slice_full_copy.asthra",
                               source,
                               NULL,
                               42);
}

// Length operation tests
void test_array_length_operation(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let arr: [42]i32 = [0; 42];\n"
        "    return arr.len;\n"
        "}\n";
    
    bdd_run_execution_scenario("Array length operation",
                               "array_length.asthra",
                               source,
                               NULL,
                               42);
}

void test_slice_length_operation(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let slice: []i32 = [1, 2, 3, 4, 5];\n"
        "    let sub: []i32 = slice[1:4];\n"
        "    return sub.len;  // Should be 3\n"
        "}\n";
    
    bdd_run_execution_scenario("Slice length operation",
                               "slice_length.asthra",
                               source,
                               NULL,
                               3);
}

void test_string_length_operation(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let s: string = \"Hello, World!\";\n"
        "    return s.len;  // Should be 13\n"
        "}\n";
    
    bdd_run_execution_scenario("String length operation",
                               "string_length.asthra",
                               source,
                               NULL,
                               13);
}

// Method calls tests
void test_basic_method_call(void) {
    const char* source = 
        "package test;\n"
        "pub struct Counter {\n"
        "    value: i32\n"
        "}\n"
        "impl Counter {\n"
        "    pub fn get(self) -> i32 {\n"
        "        return self.value;\n"
        "    }\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "    let c: Counter = Counter { value: 42 };\n"
        "    return c.get(none);\n"
        "}\n";
    
    bdd_run_execution_scenario("Basic method call",
                               "method_call_basic.asthra",
                               source,
                               NULL,
                               42);
}

void test_method_call_with_arguments(void) {
    const char* source = 
        "package test;\n"
        "pub struct Calculator {\n"
        "    base: i32\n"
        "}\n"
        "impl Calculator {\n"
        "    pub fn add(self, x: i32) -> i32 {\n"
        "        return self.base + x;\n"
        "    }\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "    let calc: Calculator = Calculator { base: 30 };\n"
        "    return calc.add(12);\n"
        "}\n";
    
    bdd_run_execution_scenario("Method call with arguments",
                               "method_call_args.asthra",
                               source,
                               NULL,
                               42);
}

// Associated function calls tests
void test_associated_function_call(void) {
    const char* source = 
        "package test;\n"
        "pub struct Number {\n"
        "    value: i32\n"
        "}\n"
        "impl Number {\n"
        "    pub fn new(v: i32) -> Number {\n"
        "        return Number { value: v };\n"
        "    }\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "    let n: Number = Number::new(42);\n"
        "    return n.value;\n"
        "}\n";
    
    bdd_run_execution_scenario("Associated function call",
                               "associated_function.asthra",
                               source,
                               NULL,
                               42);
}

void test_generic_associated_function_call(void) {
    const char* source = 
        "package test;\n"
        "pub struct Container<T> {\n"
        "    value: T\n"
        "}\n"
        "impl Container<i32> {\n"
        "    pub fn create(v: i32) -> Container<i32> {\n"
        "        return Container<i32> { value: v };\n"
        "    }\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "    let c: Container<i32> = Container<i32>::create(42);\n"
        "    return c.value;\n"
        "}\n";
    
    bdd_run_execution_scenario("Generic associated function call",
                               "generic_associated_fn.asthra",
                               source,
                               NULL,
                               42);
}

// Complex chaining tests
void test_chained_field_and_method_access(void) {
    const char* source = 
        "package test;\n"
        "pub struct Inner {\n"
        "    value: i32\n"
        "}\n"
        "pub struct Outer {\n"
        "    inner: Inner\n"
        "}\n"
        "impl Inner {\n"
        "    pub fn double(self) -> i32 {\n"
        "        return self.value * 2;\n"
        "    }\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "    let i: Inner = Inner { value: 21 };\n"
        "    let o: Outer = Outer { inner: i };\n"
        "    return o.inner.double(none);\n"
        "}\n";
    
    bdd_run_execution_scenario("Chained field and method access",
                               "chained_access.asthra",
                               source,
                               NULL,
                               42);
}

void test_function_call_on_indexed_element(void) {
    const char* source = 
        "package test;\n"
        "pub fn triple(x: i32) -> i32 {\n"
        "    return x * 3;\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "    let arr: [3]i32 = [10, 14, 30];\n"
        "    return triple(arr[1]);\n"
        "}\n";
    
    bdd_run_execution_scenario("Function call on indexed element",
                               "function_on_indexed.asthra",
                               source,
                               NULL,
                               42);
}

void test_array_of_structs_with_field_access(void) {
    const char* source = 
        "package test;\n"
        "pub struct Point {\n"
        "    x: i32,\n"
        "    y: i32\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "    let points: [3]Point = [\n"
        "        Point { x: 10, y: 20 },\n"
        "        Point { x: 42, y: 50 },\n"
        "        Point { x: 30, y: 40 }\n"
        "    ];\n"
        "    return points[1].x;\n"
        "}\n";
    
    bdd_run_execution_scenario("Array of structs with field access",
                               "array_struct_field.asthra",
                               source,
                               NULL,
                               42);
}

// Edge cases
void test_empty_array_literal_with_none(void) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> i32 {\n"
        "    let arr: []i32 = [none];\n"
        "    return arr.len;  // Should be 0\n"
        "}\n";
    
    bdd_run_execution_scenario("Empty array literal with none",
                               "empty_array_none.asthra",
                               source,
                               NULL,
                               0);
}

void test_complex_expression_in_array_index(void) {
    const char* source = 
        "package test;\n"
        "pub fn get_index(none) -> i32 {\n"
        "    return 2;\n"
        "}\n"
        "pub fn main(none) -> i32 {\n"
        "    let arr: [5]i32 = [10, 20, 42, 30, 40];\n"
        "    return arr[get_index(none)];\n"
        "}\n";
    
    bdd_run_execution_scenario("Complex expression in array index",
                               "complex_array_index.asthra",
                               source,
                               NULL,
                               42);
}

// Define test cases
BddTestCase postfix_expressions_test_cases[] = {
    // Function call tests
    BDD_TEST_CASE(basic_function_call_with_no_arguments, test_basic_function_call_with_no_arguments),
    BDD_WIP_TEST_CASE(function_call_with_single_argument, test_function_call_with_single_argument),
    BDD_WIP_TEST_CASE(function_call_with_multiple_arguments, test_function_call_with_multiple_arguments),
    BDD_WIP_TEST_CASE(nested_function_calls, test_nested_function_calls),
    BDD_WIP_TEST_CASE(function_call_with_expression_arguments, test_function_call_with_expression_arguments),
    
    // Struct field access tests
    BDD_WIP_TEST_CASE(basic_struct_field_access, test_basic_struct_field_access),
    BDD_WIP_TEST_CASE(nested_struct_field_access, test_nested_struct_field_access),
    BDD_WIP_TEST_CASE(struct_field_access_with_mutable_fields, test_struct_field_access_with_mutable_fields),
    
    // Tuple access tests
    BDD_WIP_TEST_CASE(basic_tuple_element_access, test_basic_tuple_element_access),
    BDD_WIP_TEST_CASE(tuple_access_with_larger_index, test_tuple_access_with_larger_index),
    BDD_WIP_TEST_CASE(nested_tuple_access, test_nested_tuple_access),
    
    // Array indexing tests
    BDD_WIP_TEST_CASE(basic_array_indexing, test_basic_array_indexing),
    BDD_WIP_TEST_CASE(array_indexing_with_expression, test_array_indexing_with_expression),
    BDD_WIP_TEST_CASE(multi_dimensional_array_indexing, test_multi_dimensional_array_indexing),
    
    // Slice operations tests
    BDD_WIP_TEST_CASE(basic_slice_indexing, test_basic_slice_indexing),
    BDD_WIP_TEST_CASE(slice_range_from_start, test_slice_range_from_start),
    BDD_WIP_TEST_CASE(slice_range_to_end, test_slice_range_to_end),
    BDD_WIP_TEST_CASE(slice_with_start_and_end, test_slice_with_start_and_end),
    BDD_WIP_TEST_CASE(full_slice_copy, test_full_slice_copy),
    
    // Length operation tests
    BDD_WIP_TEST_CASE(array_length_operation, test_array_length_operation),
    BDD_WIP_TEST_CASE(slice_length_operation, test_slice_length_operation),
    BDD_WIP_TEST_CASE(string_length_operation, test_string_length_operation),
    
    // Method calls tests
    BDD_WIP_TEST_CASE(basic_method_call, test_basic_method_call),
    BDD_TEST_CASE(method_call_with_arguments, test_method_call_with_arguments),
    
    // Associated function calls tests
    BDD_WIP_TEST_CASE(associated_function_call, test_associated_function_call),
    BDD_WIP_TEST_CASE(generic_associated_function_call, test_generic_associated_function_call),
    
    // Complex chaining tests
    BDD_TEST_CASE(chained_field_and_method_access, test_chained_field_and_method_access),
    BDD_WIP_TEST_CASE(function_call_on_indexed_element, test_function_call_on_indexed_element),
    BDD_WIP_TEST_CASE(array_of_structs_with_field_access, test_array_of_structs_with_field_access),
    
    // Edge cases
    BDD_WIP_TEST_CASE(empty_array_literal_with_none, test_empty_array_literal_with_none),
    BDD_WIP_TEST_CASE(complex_expression_in_array_index, test_complex_expression_in_array_index),
};

// Main test runner
int main(void) {
    return bdd_run_test_suite("Postfix Expressions",
                              postfix_expressions_test_cases,
                              sizeof(postfix_expressions_test_cases) / sizeof(postfix_expressions_test_cases[0]),
                              bdd_cleanup_temp_files);
}