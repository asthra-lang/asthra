#ifndef ASTHRA_CRYPTO_H
#define ASTHRA_CRYPTO_H

#include "collections/asthra_runtime_slices.h"
#include "core/asthra_runtime_core.h"
#include "strings/asthra_runtime_strings.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

AsthraString asthra_crypto_random_string(size_t length);
AsthraSliceHeader asthra_crypto_random_bytes(size_t size);
uint64_t asthra_hash_bytes(const void *data, size_t len);
uint64_t asthra_hash_string(AsthraString str);
bool asthra_crypto_string_equals(AsthraString a, AsthraString b);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_CRYPTO_H
