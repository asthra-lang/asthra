/**
 * String wrapper functions for Asthra runtime
 * Provides char* based wrappers around AsthraString functions
 */

#include <stdlib.h>
#include <string.h>

// Wrapper for string concatenation that works with char* instead of AsthraString
// This matches what the LLVM code generator expects
char* asthra_string_concat_wrapper(const char* left, const char* right) {
    // Handle NULL inputs
    if (!left) left = "";
    if (!right) right = "";
    
    size_t left_len = strlen(left);
    size_t right_len = strlen(right);
    size_t total_len = left_len + right_len;
    
    // Allocate memory for the concatenated string
    char* result = (char*)malloc(total_len + 1);
    if (!result) {
        return NULL;
    }
    
    // Copy the strings
    strcpy(result, left);
    strcat(result, right);
    
    return result;
}