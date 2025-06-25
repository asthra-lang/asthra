/*
 * Timer function implementations for concurrency performance tests
 */

#include "test_concurrency_benchmarks_common.h"

void timer_start(Timer *timer) {
    gettimeofday(&timer->start, NULL);
}

double timer_stop(Timer *timer) {
    gettimeofday(&timer->end, NULL);
    return (timer->end.tv_sec - timer->start.tv_sec) +
           (timer->end.tv_usec - timer->start.tv_usec) / 1000000.0;
}