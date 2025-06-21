#ifndef ASTHRA_CONCURRENCY_PATTERNS_H
#define ASTHRA_CONCURRENCY_PATTERNS_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "asthra_concurrency_core.h"

#ifdef __cplusplus
extern "C" {
#endif

void Asthra_worker_pool_dump_stats(AsthraConcurrencyWorkerPool* pool, FILE* output);
AsthraConcurrencyMultiplexer* Asthra_multiplexer_create(AsthraConcurrencyChannel** channels, size_t channel_count, AsthraConcurrencyTaskFunction processor);
AsthraResult Asthra_multiplexer_start(AsthraConcurrencyMultiplexer* mux);
void Asthra_multiplexer_stop(AsthraConcurrencyMultiplexer* mux);
void Asthra_multiplexer_destroy(AsthraConcurrencyMultiplexer* mux);
AsthraConcurrencyLoadBalancer* Asthra_load_balancer_create(AsthraConcurrencyWorkerPool** pools, size_t pool_count, AsthraConcurrencyLoadBalanceStrategy strategy);
AsthraConcurrencyTaskHandleWithAwait* Asthra_load_balancer_submit(AsthraConcurrencyLoadBalancer* balancer, AsthraConcurrencyTaskFunction func, void* args, size_t args_size);
void Asthra_load_balancer_destroy(AsthraConcurrencyLoadBalancer* balancer);
void Asthra_dump_pattern_diagnostics(FILE* output);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_CONCURRENCY_PATTERNS_H
