#ifndef RECMATH_H
#define RECMATH_H

#include <stdint.h>

/* Recursive math katas with explicit cost and overflow boundaries.
 * Status returns: 0 ok, 1 would overflow or exceed the documented work
 * bound (guarded before recursing). */

typedef void (*HanoiMoveFunc)(unsigned from_peg, unsigned to_peg, void *user);

/* n! in *out. Guard: n <= 20 (21! exceeds uint64). Depth O(n). */
int recmath_factorial(unsigned n, uint64_t *out);

/* F(n) in *out (F(0) = 0, F(1) = 1). Guard: n <= 93 (F(94) overflows).
 * Recursive fast doubling: depth O(log n), time O(log n). The internal
 * F(n+1) twin saturates at UINT64_MAX on overflow (possible only for the
 * requested n == 93), so F(93) is still returned exactly. */
int recmath_fibonacci(unsigned n, uint64_t *out);

/* Sum of decimal digits of |n| (INT64_MIN included). Depth O(digits). */
int recmath_sum_of_digits(int64_t n);

/* Ackermann A(m, n) in *out. Guard box: (m <= 3 and n <= 10) or
 * (m == 4 and n == 0). Naive recursion depth and call count both scale
 * with the RESULT (A(3, 10) = 8189 reaches ~16k frames and ~50k calls;
 * A(4, 1) = 65533 would need ~130k frames and A(3, 20) tens of millions
 * of calls), so values outside the box are rejected as work-bound
 * overflows before any recursion starts. */
int recmath_ackermann(unsigned m, unsigned n, uint64_t *out);

/* Emit the 2^disks - 1 moves solving Tower of Hanoi (pegs 1..3, all disks
 * start on peg 1) through `move` (non-NULL). Depth O(disks). Guard:
 * disks <= 20, a work bound like Ackermann's: 2^20 - 1 (about a million)
 * moves; each extra disk doubles the work. */
int recmath_tower_of_hanoi(unsigned disks, HanoiMoveFunc move, void *user);

#endif