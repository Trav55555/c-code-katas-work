# C Code Katas — verified implementations

Five katas from [ankitpokhrel/c-code-katas](https://github.com/ankitpokhrel/c-code-katas),
implemented with explicit contracts and a verification matrix
(`c-development` skill workflow). C99, gcc 15 + clang 23, x86-64.

| Module | Kata | Contract highlight |
|---|---|---|
| `luhn.c/.h` | Luhn algorithm | total function over untrusted bytes; running-mod-10 accumulator (unbounded length) |
| `slist.c/.h` | Singly linked list | ownership/borrowing/invalidation rules; alloc failure leaves list unchanged |
| `msort.c/.h` | Merge sort | iterative; alloc failure leaves array unmodified; overflow-checked sizing |
| `reservoir.c/.h` | Reservoir sample | rejection sampling (unbiased); reproducible seeds; chi-square checked |
| `pfac.c/.h` | Prime factors | snprintf-style capacity reporting; `d <= n / d` bound survives 2^64-1 |

## Gates

```sh
make gates   # everything below, both compilers
make debug release asan   # behavior + optimization + sanitizers (per compiler)
make fault    # allocation-failure injection (plain + ASan)
make uaf      # negative test: borrowed-pointer UAF must trip ASan
make fuzz     # 185k bounded iterations with oracles (differential/model/product)
make analyze  # gcc -fanalyzer
make stdmatrix # c99/c11/c17/c2x x gcc/clang, compile + run
```

Warnings are errors: `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wstrict-prototypes -Werror`.

Known gaps: no 32-bit or Windows/MSVC testing (no multilib on this host);
no TSan (single-threaded); fuzzing demonstrates explored behavior for the
stated corpus and budget only.
