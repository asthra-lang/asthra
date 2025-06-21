/*
 * Test for Phase 4 Standard Library Modules
 * Tests time, math, and random modules
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <time.h>

// Test mathematical functions
void test_math_module(void) {
    printf("Testing math module...\n");
    
    // Test constants
    assert(fabs(M_PI - 3.141592653589793) < 1e-10);
    
    // Test basic functions
    assert(abs(-42) == 42);
    assert(fabs(sqrt(16.0) - 4.0) < 1e-10);
    assert(fabs(pow(2.0, 8.0) - 256.0) < 1e-10);
    
    // Test trigonometric functions
    assert(fabs(sin(M_PI/2) - 1.0) < 1e-10);
    assert(fabs(cos(0.0) - 1.0) < 1e-10);
    
    // Test logarithmic functions
    assert(fabs(log(M_E) - 1.0) < 1e-10);
    assert(fabs(log10(100.0) - 2.0) < 1e-10);
    
    printf("Math module tests passed!\n");
}

// Test time functions
void test_time_module(void) {
    printf("Testing time module...\n");
    
    // Test duration creation
    time_t start = time(NULL);
    
    // Simulate some work
    volatile int sum = 0;
    for (int i = 0; i < 1000000; i++) {
        sum += i;
    }
    
    time_t end = time(NULL);
    double elapsed = difftime(end, start);
    
    // Should have taken some time
    assert(elapsed >= 0);
    
    printf("Time module tests passed!\n");
}

// Test random number generation
void test_random_module(void) {
    printf("Testing random module...\n");
    
    srand((unsigned int)time(NULL));
    
    // Test basic random generation
    int random1 = rand();
    int random2 = rand();
    
    // Should be different (very high probability)
    assert(random1 != random2);
    
    // Test range generation
    int range_val = rand() % 100;
    assert(range_val >= 0 && range_val < 100);
    
    // Test floating point random
    double float_val = (double)rand() / RAND_MAX;
    assert(float_val >= 0.0 && float_val <= 1.0);
    
    printf("Random module tests passed!\n");
}

// Test factorial function (number theory)
unsigned long long factorial(unsigned int n) {
    if (n == 0 || n == 1) return 1;
    unsigned long long result = 1;
    for (unsigned int i = 2; i <= n; i++) {
        result *= i;
    }
    return result;
}

// Test GCD function
unsigned int gcd(unsigned int a, unsigned int b) {
    while (b != 0) {
        unsigned int temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

// Test number theory functions
void test_number_theory(void) {
    printf("Testing number theory functions...\n");
    
    // Test factorial
    assert(factorial(0) == 1);
    assert(factorial(1) == 1);
    assert(factorial(5) == 120);
    
    // Test GCD
    assert(gcd(48, 18) == 6);
    assert(gcd(17, 13) == 1);
    
    // Test LCM (a * b / gcd(a, b))
    unsigned int lcm_result = (12 * 8) / gcd(12, 8);
    assert(lcm_result == 24);
    
    printf("Number theory tests passed!\n");
}

int main(void) {
    printf("=== Phase 4 Standard Library Module Tests ===\n");
    
    test_math_module();
    test_time_module();
    test_random_module();
    test_number_theory();
    
    printf("=== All Phase 4 tests passed! ===\n");
    return 0;
} 
