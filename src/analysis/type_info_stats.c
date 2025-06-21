#include "type_info.h"
#include <stdio.h>

// Simple global counters for type info statistics
static int type_stats_counters[10] = {0}; // Support up to 10 categories

void type_info_stats_increment(int category) {
    if (category >= 0 && category < 10) {
        type_stats_counters[category]++;
    }
}

void type_info_stats_decrement(int category) {
    if (category >= 0 && category < 10) {
        type_stats_counters[category]--;
        if (type_stats_counters[category] < 0) {
            type_stats_counters[category] = 0;
        }
    }
}

int type_info_stats_get(int category) {
    if (category >= 0 && category < 10) {
        return type_stats_counters[category];
    }
    return 0;
}

void type_info_stats_reset(void) {
    for (int i = 0; i < 10; i++) {
        type_stats_counters[i] = 0;
    }
}

TypeInfoStats type_info_get_stats(void) {
    TypeInfoStats stats = {0};
    
    // Calculate total from all categories
    stats.total_types = 0;
    for (int i = 0; i < 10; i++) {
        stats.total_types += type_stats_counters[i];
    }
    
    // Map categories to specific type counts
    stats.primitive_types = type_stats_counters[0];  // TYPE_INFO_PRIMITIVE
    stats.struct_types = type_stats_counters[1];     // TYPE_INFO_STRUCT
    stats.slice_types = type_stats_counters[2];      // TYPE_INFO_SLICE
    stats.pointer_types = type_stats_counters[3];    // TYPE_INFO_POINTER
    stats.result_types = type_stats_counters[4];     // TYPE_INFO_RESULT
    stats.function_types = type_stats_counters[5];   // TYPE_INFO_FUNCTION
    stats.module_types = type_stats_counters[7];     // TYPE_INFO_MODULE
    
    // Estimate memory usage (simplified)
    stats.memory_usage = stats.total_types * sizeof(void*) * 10; // Rough estimate
    
    return stats;
}

void type_info_print(const TypeInfo *type_info, int indent) {
    if (!type_info) {
        printf("%*s(null TypeInfo)\n", indent, "");
        return;
    }
    
    printf("%*sTypeInfo {\n", indent, "");
    printf("%*s  type_id: %u\n", indent, "", type_info->type_id);
    printf("%*s  name: %s\n", indent, "", type_info->name ? type_info->name : "(null)");
    printf("%*s  category: ", indent, "");
    
    switch (type_info->category) {
        case TYPE_INFO_PRIMITIVE: printf("PRIMITIVE"); break;
        case TYPE_INFO_STRUCT: printf("STRUCT"); break;
        case TYPE_INFO_SLICE: printf("SLICE"); break;
        case TYPE_INFO_POINTER: printf("POINTER"); break;
        case TYPE_INFO_RESULT: printf("RESULT"); break;
        case TYPE_INFO_FUNCTION: printf("FUNCTION"); break;
        case TYPE_INFO_ENUM: printf("ENUM"); break;
        case TYPE_INFO_MODULE: printf("MODULE"); break;
        case TYPE_INFO_UNKNOWN: printf("UNKNOWN"); break;
        case TYPE_INFO_ERROR: printf("ERROR"); break;
        default: printf("INVALID(%d)", type_info->category); break;
    }
    printf("\n");
    
    printf("%*s  size: %zu bytes\n", indent, "", type_info->size);
    printf("%*s  alignment: %zu bytes\n", indent, "", type_info->alignment);
    printf("%*s}\n", indent, "");
} 
