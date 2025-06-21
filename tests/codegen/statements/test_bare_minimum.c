#include <stdio.h>
#include "statement_generation_test_fixtures.h"

int main(void) {
    printf("Test with statement generation fixtures\n");
    
    printf("About to create fixture...\n");
    CodeGenTestFixture* fixture = setup_codegen_fixture();
    printf("Fixture created\n");
    
    if (fixture) {
        cleanup_codegen_fixture(fixture);
        printf("Fixture cleaned up\n");
    }
    
    return 0;
}