#ifndef ASTHRA_SAFETY_CONCURRENCY_ERRORS_H
#define ASTHRA_SAFETY_CONCURRENCY_ERRORS_H

#include "asthra_safety_common.h"
#include "asthra_safety_core.h"
#include "types/asthra_runtime_result.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

void asthra_safety_log_task_lifecycle_event_enhanced(uint64_t task_id, AsthraTaskEvent event,
                                                     const char *details, AsthraResult *result);
void asthra_safety_concurrency_errors_cleanup(void);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_SAFETY_CONCURRENCY_ERRORS_H
