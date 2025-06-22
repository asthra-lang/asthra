#include <stdio.h>
#include <string.h>
#include "bdd_support.h"

// Sample BDD unit test for basic compiler functionality

static char* source_code = NULL;
static int compilation_result = -1;

void given_valid_asthra_source(void) {
    source_code = "package main; pub fn main(none) -> void { return (); }";
    bdd_given("valid Asthra source code");
}

void when_compiler_processes_source(void) {
    // Simulate compilation (in real test, would call actual compiler)
    if (source_code && strstr(source_code, "package") && strstr(source_code, "main")) {
        compilation_result = 0; // Success
    } else {
        compilation_result = 1; // Failure
    }
    bdd_when("the compiler processes the source");
}

void then_compilation_succeeds(void) {
    bdd_then("compilation should succeed");
    BDD_ASSERT_EQ(compilation_result, 0);
}

int main(void) {
    bdd_init("Compiler Basic Functionality");
    
    bdd_scenario("Compile valid Asthra program");
    given_valid_asthra_source();
    when_compiler_processes_source();
    then_compilation_succeeds();
    
    return bdd_report();
}