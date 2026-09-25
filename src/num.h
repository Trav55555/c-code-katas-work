#ifndef NUM_H
#define NUM_H

#include <stddef.h>
#include <stdint.h>

/* Divisibility utilities over uint64_t. No allocation, overflow-safe loop
 * bounds (d <= n / d, never d * d).
 *
 * Capacity contract (snprintf-style, like pfac): *count is always set to
 * the number of entries needed; 0 means everything fit, 1 means truncated.
 * `factors` may be NULL only when cap == 0 (count-only pass).
 * num_factors(0) has zero entries by definition; num_factors(1) = { 1 }. */
int num_factors(uint64_t n, uint64_t *factors, size_t cap, size_t *count);
int num_common_factors(uint64_t a, uint64_t b,
                       uint64_t *factors, size_t cap, size_t *count);

/* Greatest common divisor; gcd(0, 0) == 0. The naive variant scans
 * downward (cost O(min(a, b))) and agrees with the Euclid variant. */
uint64_t num_gcd(uint64_t a, uint64_t b);
uint64_t num_gcd_naive(uint64_t a, uint64_t b);

/* 1 for primes, 0 otherwise (0 and 1 are not prime). Cost O(sqrt n). */
int num_is_prime(uint64_t n);

#endif