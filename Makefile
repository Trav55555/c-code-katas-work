CC ?= clang
CSTD = -std=c99
WARN = -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wstrict-prototypes -Werror
INC = -I.
SRC = luhn.c slist.c msort.c reservoir.c pfac.c sorts.c search.c dlist.c bst.c num.c strs.c recmath.c euler.c
FAULT_SRC = slist.c msort.c dlist.c bst.c
SAN = -fsanitize=address,undefined -fno-sanitize-recover=all

all: debug

debug:
	$(CC) $(CSTD) $(WARN) $(INC) -O0 -g $(SRC) tests.c -o run_tests
	./run_tests
	$(CC) $(CSTD) $(WARN) $(INC) -O0 -g $(SRC) tests_rest.c -o run_rest
	./run_rest

release:
	$(CC) $(CSTD) $(WARN) $(INC) -O2 -DNDEBUG $(SRC) tests.c -o run_tests_release
	./run_tests_release
	$(CC) $(CSTD) $(WARN) $(INC) -O2 -DNDEBUG $(SRC) tests_rest.c -o run_rest_release
	./run_rest_release

asan:
	$(CC) $(CSTD) $(WARN) $(INC) -O1 -g $(SAN) $(SRC) tests.c -o run_tests_asan
	./run_tests_asan
	$(CC) $(CSTD) $(WARN) $(INC) -O1 -g $(SAN) $(SRC) tests_rest.c -o run_rest_asan
	./run_rest_asan

fuzz:
	$(CC) $(CSTD) $(WARN) $(INC) -O1 -g $(SAN) $(SRC) fuzz.c -o fuzz_test
	./fuzz_test

# Coverage-guided fuzzing of the byte parser (clang libFuzzer). Bounded by
# time; crashes land in fuzz-artifacts/ and should become regression tests.
LIBFUZZ_SECONDS ?= 20
libfuzz:
	mkdir -p fuzz-artifacts
	clang $(CSTD) $(WARN) $(INC) -O1 -g -fsanitize=fuzzer,address,undefined -fno-sanitize-recover=all luhn.c fuzz_luhn_lf.c -o fuzz_luhn_lf
	./fuzz_luhn_lf -max_total_time=$(LIBFUZZ_SECONDS) -max_len=4096 \
		-artifact_prefix=fuzz-artifacts/ -print_final_stats=1 \
		> fuzz-artifacts/libfuzz.log 2>&1; status=$$?; \
		grep -E 'Done|number_of_executed_units|ERROR|SUMMARY|deadly' fuzz-artifacts/libfuzz.log; \
		exit $$status

analyze:
	gcc $(CSTD) $(WARN) $(INC) -fanalyzer -c $(SRC)

# Allocation-failure injection: module TUs see -Dmalloc=fault_malloc.
fault:
	$(CC) $(CSTD) $(WARN) $(INC) -O0 -g -Dmalloc=fault_malloc -include fault_alloc.h -c $(FAULT_SRC)
	$(CC) $(CSTD) $(WARN) $(INC) -O0 -g -c fault_alloc.c
	$(CC) $(CSTD) $(WARN) $(INC) -O0 -g -Dmalloc=fault_malloc -include fault_alloc.h -c fault_tests.c
	$(CC) $(CSTD) $(WARN) $(INC) -O0 -g slist.o msort.o dlist.o bst.o fault_alloc.o fault_tests.o -o run_fault
	./run_fault
	$(CC) $(CSTD) $(WARN) $(INC) -O1 -g $(SAN) -Dmalloc=fault_malloc -include fault_alloc.h -c $(FAULT_SRC)
	$(CC) $(CSTD) $(WARN) $(INC) -O1 -g $(SAN) -c fault_alloc.c
	$(CC) $(CSTD) $(WARN) $(INC) -O1 -g $(SAN) -Dmalloc=fault_malloc -include fault_alloc.h -c fault_tests.c
	$(CC) $(CSTD) $(WARN) $(INC) -O1 -g $(SAN) slist.o msort.o dlist.o bst.o fault_alloc.o fault_tests.o -o run_fault_asan
	./run_fault_asan

# Negative test: the documented borrowed-pointer invalidation must trip ASan.
uaf:
	$(CC) $(CSTD) $(WARN) $(INC) -O1 -g $(SAN) $(SRC) uaf_probe.c -o uaf_probe
	@if ./uaf_probe > uaf_probe.out 2>&1; then \
		echo "FAIL: use-after-free not detected"; exit 1; \
	elif grep -q heap-use-after-free uaf_probe.out; then \
		echo "OK: ASan caught the use-after-free"; \
	else \
		echo "FAIL: wrong failure mode"; cat uaf_probe.out; exit 1; \
	fi

# Language-mode portability: compile and run under every supported -std.
stdmatrix:
	for std in c99 c11 c17 c2x; do \
	  for cc in gcc clang; do \
	    $$cc -std=$$std $(WARN) $(INC) -O0 $(SRC) tests.c -o std_test || exit 1; \
	    ./std_test >/dev/null || exit 1; \
	    $$cc -std=$$std $(WARN) $(INC) -O0 $(SRC) tests_rest.c -o std_rest || exit 1; \
	    ./std_rest >/dev/null || exit 1; \
	  done; \
	done
	@echo "OK: std matrix c99/c11/c17/c2x x gcc/clang"

# Portability matrix: both compilers across behavior, optimization, sanitizers.
gates:
	for cc in gcc clang; do $(MAKE) CC=$$cc debug release asan fault uaf || exit 1; done
	$(MAKE) fuzz libfuzz analyze stdmatrix

clean:
	rm -f run_tests run_tests_release run_tests_asan run_rest run_rest_release run_rest_asan run_fault run_fault_asan uaf_probe fuzz_test std_test std_rest fuzz_luhn_lf *.o uaf_probe.out

.PHONY: all debug release asan fuzz libfuzz analyze fault uaf stdmatrix gates clean
