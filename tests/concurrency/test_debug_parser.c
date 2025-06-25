#include "parser_string_interface.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    const char *simple_code = "package test;\n"
                              "\n"
                              "pub fn test_function(none) -> Result<(), string> {\n"
                              "    Result.Ok(())\n"
                              "}\n";

    printf("Testing simple code:\n%s\n", simple_code);

    ParseResult result = parse_string(simple_code);

    printf("Parse result: success=%s\n", result.success ? "true" : "false");
    printf("Error count: %d\n", result.error_count);

    if (result.error_count > 0) {
        for (int i = 0; i < result.error_count; i++) {
            printf("Error %d: %s\n", i + 1, result.errors[i]);
        }
    }

    cleanup_parse_result(&result);
    return 0;
}
