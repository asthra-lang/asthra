#include "bdd_support.h"
#include "bdd_utilities.h"
#include "bdd_test_framework.h"

// Test scenarios using the new reusable framework

void test_simple_struct(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub struct Point {\n"
        "    x: i32,\n"
        "    y: i32\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let p: Point = Point { x: 10, y: 20 };\n"
        "    log(\"Simple struct works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Simple struct definition",
                               "simple_struct.asthra",
                               source,
                               "Simple struct works",
                               0);
}

void test_mixed_struct(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub struct Person {\n"
        "    name: string,\n"
        "    age: u32,\n"
        "    height: f64,\n"
        "    is_active: bool\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let p: Person = Person { \n"
        "        name: \"Alice\", \n"
        "        age: 30, \n"
        "        height: 1.75, \n"
        "        is_active: true \n"
        "    };\n"
        "    log(\"Mixed type struct works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Struct with mixed types",
                               "mixed_struct.asthra",
                               source,
                               "Mixed type struct works",
                               0);
}

void test_empty_struct(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub struct Empty {\n"
        "    none\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let e: Empty = Empty {};\n"
        "    log(\"Empty struct works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Empty struct",
                               "empty_struct.asthra",
                               source,
                               "Empty struct works",
                               0);
}

void test_nested_struct(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub struct Point {\n"
        "    x: i32,\n"
        "    y: i32\n"
        "}\n"
        "\n"
        "pub struct Rectangle {\n"
        "    top_left: Point,\n"
        "    bottom_right: Point\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let rect: Rectangle = Rectangle {\n"
        "        top_left: Point { x: 0, y: 0 },\n"
        "        bottom_right: Point { x: 100, y: 100 }\n"
        "    };\n"
        "    log(\"Nested structs work\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Nested structs",
                               "nested_struct.asthra",
                               source,
                               "Nested structs work",
                               0);
}

void test_simple_enum(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub enum Status {\n"
        "    Active,\n"
        "    Inactive,\n"
        "    Pending\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let s: Status = Status.Active;\n"
        "    log(\"Simple enum works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Simple enum definition",
                               "simple_enum.asthra",
                               source,
                               "Simple enum works",
                               0);
}

void test_enum_single_data(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub enum Option {\n"
        "    Some(i32),\n"
        "    None\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let val: Option = Option.Some(42);\n"
        "    let empty: Option = Option.None;\n"
        "    log(\"Enum with data works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Enum with single type data",
                               "enum_single_data.asthra",
                               source,
                               "Enum with data works",
                               0);
}

void test_enum_tuple_data(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub enum Message {\n"
        "    Move(i32, i32),\n"
        "    Write(string),\n"
        "    Color(u8, u8, u8),\n"
        "    Quit\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let msg1: Message = Message.Move(10, 20);\n"
        "    let msg2: Message = Message.Write(\"Hello\");\n"
        "    let msg3: Message = Message.Color(255, 0, 0);\n"
        "    let msg4: Message = Message.Quit;\n"
        "    log(\"Enum with tuple data works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Enum with tuple data",
                               "enum_tuple_data.asthra",
                               source,
                               "Enum with tuple data works",
                               0);
}

void test_empty_enum(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub enum Never {\n"
        "    none\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    log(\"Empty enum compiles\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Empty enum",
                               "empty_enum.asthra",
                               source,
                               "Empty enum compiles",
                               0);
}

void test_struct_methods(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub struct Counter {\n"
        "    value: i32\n"
        "}\n"
        "\n"
        "impl Counter {\n"
        "    pub fn new(none) -> Counter {\n"
        "        return Counter { value: 0 };\n"
        "    }\n"
        "    \n"
        "    pub fn increment(self) -> void {\n"
        "        self.value = self.value + 1;\n"
        "        return ();\n"
        "    }\n"
        "    \n"
        "    pub fn get_value(self) -> i32 {\n"
        "        return self.value;\n"
        "    }\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let mut c: Counter = Counter.new();\n"
        "    c.increment();\n"
        "    log(\"Struct methods work\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Struct with methods",
                               "struct_methods.asthra",
                               source,
                               "Struct methods work",
                               0);
}

void test_field_visibility(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub struct Account {\n"
        "    pub username: string,\n"
        "    priv password: string,\n"
        "    balance: i64\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let acc: Account = Account {\n"
        "        username: \"alice\",\n"
        "        password: \"secret\",\n"
        "        balance: 1000\n"
        "    };\n"
        "    log(\"Field visibility works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Struct field visibility",
                               "field_visibility.asthra",
                               source,
                               "Field visibility works",
                               0);
}

void test_variant_visibility(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub enum Response {\n"
        "    pub Success(string),\n"
        "    priv InternalError(i32),\n"
        "    Pending\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let r: Response = Response.Success(\"OK\");\n"
        "    log(\"Variant visibility works\");\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_execution_scenario("Enum variant visibility",
                               "variant_visibility.asthra",
                               source,
                               "Variant visibility works",
                               0);
}

void test_duplicate_field(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub struct Point {\n"
        "    x: i32,\n"
        "    x: i32\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_compilation_scenario("Duplicate struct field error",
                                 "duplicate_field.asthra",
                                 source,
                                 0,  // should fail
                                 "duplicate field");
}

void test_duplicate_variant(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub enum Status {\n"
        "    Active,\n"
        "    Active\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_compilation_scenario("Duplicate enum variant error",
                                 "duplicate_variant.asthra",
                                 source,
                                 0,  // should fail
                                 "duplicate variant");
}

void test_missing_fields(void) {
    const char* source = 
        "package main;\n"
        "\n"
        "pub struct Point {\n"
        "    x: i32,\n"
        "    y: i32\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let p: Point = Point { x: 10 };\n"
        "    return ();\n"
        "}\n";
    
    bdd_run_compilation_scenario("Missing struct fields in initialization",
                                 "missing_fields.asthra",
                                 source,
                                 0,  // should fail
                                 "missing field 'y'");
}

// Define test cases using the new framework - no @wip tags based on original file
BddTestCase user_defined_types_test_cases[] = {
    BDD_TEST_CASE(simple_struct, test_simple_struct),
    BDD_TEST_CASE(mixed_struct, test_mixed_struct),
    BDD_TEST_CASE(empty_struct, test_empty_struct),
    BDD_TEST_CASE(nested_struct, test_nested_struct),
    BDD_TEST_CASE(simple_enum, test_simple_enum),
    BDD_TEST_CASE(enum_single_data, test_enum_single_data),
    BDD_TEST_CASE(enum_tuple_data, test_enum_tuple_data),
    BDD_TEST_CASE(empty_enum, test_empty_enum),
    BDD_TEST_CASE(struct_methods, test_struct_methods),
    BDD_TEST_CASE(field_visibility, test_field_visibility),
    BDD_TEST_CASE(variant_visibility, test_variant_visibility),
    BDD_TEST_CASE(duplicate_field, test_duplicate_field),
    BDD_TEST_CASE(duplicate_variant, test_duplicate_variant),
    BDD_TEST_CASE(missing_fields, test_missing_fields),
};

// Main test runner using the new framework
int main(void) {
    return bdd_run_test_suite("User-Defined Types",
                              user_defined_types_test_cases,
                              sizeof(user_defined_types_test_cases) / sizeof(user_defined_types_test_cases[0]),
                              bdd_cleanup_temp_files);
}