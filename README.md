# C Code Katas — verified implementations

All katas from [ankitpokhrel/c-code-katas](https://github.com/ankitpokhrel/c-code-katas)
(46 programs) implemented with explicit contracts and a verification matrix
(`c-development` skill workflow). C99, gcc 15 + clang 23, x86-64.

| Module | Katas | Contract highlight |
|---|---|---|
| `luhn.c/.h` | Luhn algorithm | total function over untrusted bytes; running-mod-10 accumulator (unbounded length) |
| `slist.c/.h` | Singly linked list | ownership/borrowing/invalidation rules; alloc failure leaves list unchanged |
| `msort.c/.h` | Merge sort | iterative; alloc failure leaves array unmodified; overflow-checked sizing |
| `reservoir.c/.h` | Reservoir sample | rejection sampling (unbiased); reproducible seeds; chi-square checked |
| `pfac.c/.h` | Prime factors | snprintf-style capacity reporting; `d <= n / d` bound survives 2^64-1 |
| `sorts.c/.h` | bubble, insertion, selection, shell, quick, randomized quick | no allocation; three-way partition (equal keys stay linear); recurse smaller side, O(log n) stack |
| `search.c/.h` | linear, binary (+ recursive variants) | overflow-safe midpoints; `ptrdiff_t` indices; documented sorted precondition |
| `dlist.c/.h` | Doubly linked list | mirrors slist contract + reverse lookup |
| `bst.c/.h` | Binary search tree | allocation before mutation; non-recursive teardown; parent-link walk (O(1) space, tree unmodified, visitor may read) |
| `num.c/.h` | factors, common factors, gcd x2, is_prime | two-pass divisor pairing; snprintf-style counts |
| `strs.c/.h` | reverse, replace, palindrome | recursion depth O(length) documented |
| `recmath.c/.h` | factorial, fibonacci, sum_of_digits, ackermann, tower_of_hanoi | work-bound guards (Ackermann box; Hanoi <= 20 disks) |
| `euler.c/.h` | Project Euler 1-18 + sieve | known-answer oracles; checked arithmetic with overflow-boundary tests; snprintf-style number words; `euler_data.h` extracted from upstream |

## Gates

```sh
make gates   # everything below, both compilers
make debug release asan   # behavior + optimization + sanitizers (per compiler)
make fault    # allocation-failure injection (plain + ASan)
make uaf      # negative test: borrowed-pointer UAF must trip ASan
make fuzz     # 354k bounded iterations, 13 surfaces, with oracles
make libfuzz  # coverage-guided libFuzzer on luhn (LIBFUZZ_SECONDS, default 20)
make analyze  # gcc -fanalyzer
make stdmatrix # c99/c11/c17/c2x x gcc/clang, compile + run
```

Warnings are errors: `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wstrict-prototypes -Werror`.

Internal helpers shared across modules live in `checked.h` (overflow-checked
uint64 arithmetic) and `rng.h` (xorshift64* with rejection-sampled ranges).

`make fault` redefines `malloc` with `-Dmalloc=fault_malloc`, which is
undefined behavior by the letter of C99 7.1.3. It is a deliberate,
test-only project decision; see `fault_alloc.h`.

Known gaps: no 32-bit or Windows/MSVC testing (no multilib on this host);
no TSan (all single-threaded); fuzzing demonstrates explored behavior for
the stated corpus and budget only; `pfac`/`num_factors` are O(sqrt n) by
construction (large semiprimes are slow, documented); the fault build
covers `malloc` in slist/msort/dlist/bst only, so the `calloc`/`malloc`
failure paths in `euler.c` (Collatz cache, PE16, PE18, sieve) are reviewed
but not exercised; the quicksort duplicate test is a wall-clock budget.

## License

MIT; see `LICENSE`. `euler_data.h` is extracted from the upstream kata set, also MIT (Copyright (c) 2016 Ankit Pokhrel).
