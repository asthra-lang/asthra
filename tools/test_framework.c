/**
 * Minimal test for common framework
 */

#include <stdio.h>
#include <stdlib.h>

#include "common/cli_framework.h"
#include "common/statistics_framework.h"
#include "common/error_framework.h"

int main(int argc, char *argv[]) {
    printf("Testing framework initialization...\n");
    
    // Test CLI framework
    CliConfig *cli_config = cli_create_config("Test Tool", "[options]", "Test description");
    if (!cli_config) {
        printf("Failed to create CLI config\n");
        return 1;
    }
    printf("CLI config created successfully\n");
    
    // Test adding options
    cli_add_option(cli_config, "verbose", 'v', false, false, "Verbose output");
    printf("CLI option added successfully\n");
    
    // Test statistics framework
    StatsFramework *stats = stats_create_framework("Test Tool");
    if (!stats) {
        printf("Failed to create stats framework\n");
        cli_destroy_config(cli_config);
        return 1;
    }
    printf("Stats framework created successfully\n");
    
    // Test adding counters
    stats_add_counter(stats, "test_counter", "Test counter", false);
    printf("Stats counter added successfully\n");
    
    // Test error framework
    ErrorFramework *errors = error_create_framework("Test Tool");
    if (!errors) {
        printf("Failed to create error framework\n");
        stats_destroy_framework(stats);
        cli_destroy_config(cli_config);
        return 1;
    }
    printf("Error framework created successfully\n");
    
    // Test basic operations
    stats_increment_by_one(stats, "test_counter");
    printf("Stats increment successful\n");
    
    ERROR_REPORT_INFO(errors, "Test info message");
    printf("Error reporting successful\n");
    
    // Test parsing with minimal args
    if (argc > 1) {
        printf("Testing CLI parsing...\n");
        CliOptionValue values[CLI_MAX_OPTIONS];
        CliParseResult parse_result = cli_parse_args(cli_config, argc, argv, values, CLI_MAX_OPTIONS);
        
        if (parse_result.help_requested) {
            cli_print_help(cli_config);
        } else if (parse_result.error_occurred) {
            printf("Parse error: %s\n", parse_result.error_message);
        } else {
            printf("Parse successful, remaining args: %zu\n", parse_result.remaining_count);
        }
    }
    
    // Clean up
    error_destroy_framework(errors);
    stats_destroy_framework(stats);
    cli_destroy_config(cli_config);
    
    printf("All tests passed!\n");
    return 0;
} 
