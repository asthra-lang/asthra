/**
 * Wrapper for generate_c_code to match test expectations
 */

#include "ast_types.h"
#include <stdbool.h>
#include <stdio.h>

// Forward declaration - the actual function is in code_generation.o
int generate_c_code(FILE *output, ASTNode *node);

// Wrapper with the signature expected by the test
bool generate_c_code_to_file(ASTNode *root, const char *output_file) {
    if (!root || !output_file) {
        return false;
    }

    FILE *output = fopen(output_file, "w");
    if (!output) {
        return false;
    }

    int result = generate_c_code(output, root);
    fclose(output);

    return result == 0;
}