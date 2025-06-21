#ifndef ASTHRA_SAFETY_CORE_H
#define ASTHRA_SAFETY_CORE_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "core/asthra_runtime_core.h"

#ifdef __cplusplus
extern "C" {
#endif

AsthraSafetyConfig asthra_safety_get_config(void);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_SAFETY_CORE_H
