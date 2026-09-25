#ifndef EULER_H
#define EULER_H

#include <stddef.h>
#include <stdint.h>

/* Project Euler katas (problems 1-18 as collected in ankitpokhrel/
 * c-code-katas) plus the sieve utility. Overflow policy: where a result
 * could leave uint64_t, the function returns 0 and documents it; checked
 * arithmetic is used in the hot formulas. Datasets live in euler_data.h.
 * Known answers are asserted in tests_rest.c. */

/* PE1: sum of multiples of 3 or 5 below `limit`. 0 on overflow. */
uint64_t euler_multiples_3_5(uint64_t limit);
/* PE2: sum of even Fibonacci numbers below `limit`. Stops on overflow. */
uint64_t euler_even_fibonacci_sum(uint64_t limit);
/* PE3: largest prime factor (0 for n < 2). */
uint64_t euler_largest_prime_factor(uint64_t n);
/* PE4: largest palindrome product of two `digits`-digit numbers.
 * Guard: 1 <= digits <= 5 (cost O(10^2d) with pruning). */
uint64_t euler_largest_palindrome_product(unsigned digits);
/* PE5: smallest number divisible by every integer in [lo, hi].
 * Guard: 1 <= lo <= hi. 0 on overflow. */
uint64_t euler_smallest_multiple(unsigned lo, unsigned hi);
/* PE6: (sum 1..n)^2 - (sum of squares 1..n). 0 on overflow. */
uint64_t euler_sum_square_difference(unsigned n);
/* PE8: largest product of `span` adjacent digits in the 1000-digit series.
 * Guard: 1 <= span <= 19 (beyond that the product leaves uint64). */
uint64_t euler_largest_product_in_series(size_t span);
/* PE9: product abc of the Pythagorean triplet with a + b + c == sum.
 * 0 when no triplet exists. */
uint64_t euler_pythagorean_triplet_product(unsigned sum);
/* PE11: largest product of `span` adjacent grid values in any of the four
 * directions over the 20x20 grid. Guard: 1 <= span <= 20. 0 on overflow
 * (span >= 11 on this grid). */
uint64_t euler_largest_product_in_grid(size_t span);
/* PE12: first triangle number with MORE than `min_divisors` divisors. */
uint64_t euler_highly_divisible_triangle(unsigned min_divisors);
/* PE13: first `digits` decimal digits of the sum of the 100 fifty-digit
 * numbers. Guard: 1 <= digits <= 19 (uint64 return). */
uint64_t euler_large_sum_prefix(size_t digits);
/* PE14: starting value below `limit` with the longest Collatz chain
 * (ties: the smallest). Guard: limit >= 2. */
uint64_t euler_longest_collatz(uint64_t limit);
/* PE15: lattice paths through a w x h grid (binomial C(w+h, w)).
 * 0 on overflow. */
uint64_t euler_lattice_paths(unsigned w, unsigned h);
/* PE16: sum of decimal digits of 2^exponent. Guard: exponent <= 200000.
 * 0 on allocation failure (a power of two never has digit sum 0). */
uint64_t euler_power_digit_sum(unsigned exponent);
/* PE17 helpers: British naming ("one hundred and forty-two").
 * euler_number_name_letters counts letters only (no spaces or hyphens);
 * 0 for n > 9999.
 * euler_number_to_words is snprintf-style: *needed is the full name's
 * length in characters, excluding the NUL; when cap > 0 the first
 * min(*needed, cap - 1) characters and a NUL are written, and that count is
 * returned. Nothing is written when cap == 0 (buf may then be NULL).
 * n > 9999 is out of range: *needed = 0 and an empty string. */
size_t euler_number_name_letters(unsigned n);
size_t euler_number_to_words(unsigned n, char *buf, size_t cap, size_t *needed);
/* PE17: sum of letter counts over the inclusive range [lo, hi].
 * Guard: 1 <= lo <= hi <= 9999, else 0. */
uint64_t euler_number_letter_counts(unsigned lo, unsigned hi);
/* PE18: maximum top-to-bottom path sum over a triangle of nrows rows,
 * given as flat row-major values: row i holds i + 1 entries, so `values`
 * has nrows (nrows + 1) / 2 of them (NULL only when nrows == 0).
 * 0 for an empty triangle or when the scratch allocation fails or its size
 * would overflow. Sums over fewer than 2^32 rows of unsigned values fit. */
uint64_t euler_maximum_path_sum(const unsigned *values, size_t nrows);
/* Sieve of Eratosthenes: primes <= `limit` ascending. snprintf-style
 * capacity: 0 fit, 1 truncated, -1 allocation failure (*count = 0). */
int euler_sieve(unsigned limit, unsigned *primes, size_t cap, size_t *count);

#endif
