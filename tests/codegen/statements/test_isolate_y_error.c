#include <stdio.h>

// Declare these manually to avoid including headers that might have initialization
typedef struct CodeGenerator CodeGenerator;
typedef struct SemanticAnalyzer SemanticAnalyzer;

// Just the fixture structure
typedef struct {
    CodeGenerator* generator;
    SemanticAnalyzer* analyzer;
    char* output_buffer;
    size_t output_buffer_size;
    void* test_ast;
} CodeGenTestFixture;

// Declare the function we want to test
extern CodeGenTestFixture* setup_codegen_fixture(void);
extern void cleanup_codegen_fixture(CodeGenTestFixture* fixture);

int main(void) {
    printf("=== Test to isolate 'undefined variable: y' error ===\n");
    printf("Creating fixture...\n");
    fflush(stdout);
    
    CodeGenTestFixture* fixture = setup_codegen_fixture();
    
    printf("Fixture created: %p\n", (void*)fixture);
    fflush(stdout);
    
    if (fixture) {
        printf("Cleaning up fixture...\n");
        fflush(stdout);
        cleanup_codegen_fixture(fixture);
    }
    
    printf("Done.\n");
    return 0;
}