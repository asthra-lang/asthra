#ifndef ASTHRA_SAFETY_CORE_H
#define ASTHRA_SAFETY_CORE_H

#include "asthra_safety_common.h"
#include "core/asthra_runtime_core.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

AsthraSafetyConfig asthra_safety_get_config(void);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_SAFETY_CORE_H
