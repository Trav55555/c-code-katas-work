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
| `sorts.c/.h` | bubble, insertion, selection, shell, quick, randomized quick | no allocation; bounded recursion (recurse smaller side, O(log n) stack) |
| `search.c/.h` | linear, binary (+ recursive variants) | overflow-safe midpoints; documented sorted precondition |
| `dlist.c/.h` | Doubly linked list | mirrors slist contract + reverse lookup |
| `bst.c/.h` | Binary search tree | allocation before mutation; non-recursive teardown; Morris walk (O(1) space) |
| `num.c/.h` | factors, common factors, gcd x2, is_prime | two-pass divisor pairing; snprintf-style counts |
| `strs.c/.h` | reverse, replace, palindrome | recursion depth O(length) documented |
| `recmath.c/.h` | factorial, fibonacci, sum_of_digits, ackermann, tower_of_hanoi | work-bound guards (naive Ackermann depth scales with the result) |
| `euler.c/.h` | Project Euler 1-18 + sieve | known-answer oracles; checked arithmetic; `euler_data.h` extracted from upstream |

## Gates

```sh
make gates   # everything below, both compilers
make debug release asan   # behavior + optimization + sanitizers (per compiler)
make fault    # allocation-failure injection (plain + ASan)
make uaf      # negative test: borrowed-pointer UAF must trip ASan
make fuzz     # 241k bounded iterations, 8 surfaces, with oracles
make analyze  # gcc -fanalyzer
make stdmatrix # c99/c11/c17/c2x x gcc/clang, compile + run
```

Warnings are errors: `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wstrict-prototypes -Werror`.

Known gaps: no 32-bit or Windows/MSVC testing (no multilib on this host);
no TSan (all single-threaded); fuzzing demonstrates explored behavior for
the stated corpus and budget only; `pfac`/`num_factors` are O(sqrt n) by
construction (large semiprimes are slow, documented).
