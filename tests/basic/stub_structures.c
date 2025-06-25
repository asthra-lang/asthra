/**
 * Structure definitions and constants for comprehensive stubs
 */

#include <stdbool.h>
#include <stdlib.h>

// Minimal structure definitions
typedef struct SourceLocation {
    int line;
    int column;
    const char *file;
} SourceLocation;

typedef struct ASTNode {
    int type;
    SourceLocation location;
    void *data;
} ASTNode;

typedef struct SemanticAnalyzer {
    void *data;
} SemanticAnalyzer;

typedef struct CodeGenerator {
    void *data;
} CodeGenerator;

typedef struct SymbolTable {
    void *data;
} SymbolTable;

typedef struct TypeInfo {
    void *data;
} TypeInfo;

// Type info constants
const void *TYPE_INFO_CHAR = (void *)0x1;
const void *TYPE_INFO_STRING = (void *)0x2;