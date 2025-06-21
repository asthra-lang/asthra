#ifndef ASTHRA_CONCURRENCY_CHANNELS_H
#define ASTHRA_CONCURRENCY_CHANNELS_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "asthra_concurrency_core.h"

#ifdef __cplusplus
extern "C" {
#endif

size_t Asthra_channel_len(AsthraConcurrencyChannel* channel);
size_t Asthra_channel_cap(AsthraConcurrencyChannel* channel);
bool Asthra_channel_is_empty(AsthraConcurrencyChannel* channel);
bool Asthra_channel_is_full(AsthraConcurrencyChannel* channel);
AsthraConcurrencyChannelInfo Asthra_channel_get_info(AsthraConcurrencyChannel* channel);
void Asthra_channel_dump_info(AsthraConcurrencyChannel* channel, FILE* output);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_CONCURRENCY_CHANNELS_H
