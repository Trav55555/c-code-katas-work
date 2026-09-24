CC ?= clang
CSTD = -std=c99
WARN = -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wstrict-prototypes -Werror
INC = -I.
SRC = luhn.c slist.c msort.c reservoir.c pfac.c
SAN = -fsanitize=address,undefined -fno-sanitize-recover=all

all: debug

debug:
	$(CC) $(CSTD) $(WARN) $(INC) -O0 -g $(SRC) tests.c -o run_tests
	./run_tests

release:
	$(CC) $(CSTD) $(WARN) $(INC) -O2 -DNDEBUG $(SRC) tests.c -o run_tests_release
	./run_tests_release

asan:
	$(CC) $(CSTD) $(WARN) $(INC) -O1 -g $(SAN) $(SRC) tests.c -o run_tests_asan
	./run_tests_asan

fuzz:
	$(CC) $(CSTD) $(WARN) $(INC) -O1 -g $(SAN) $(SRC) fuzz.c -o fuzz_test
	./fuzz_test

analyze:
	gcc $(CSTD) $(WARN) $(INC) -fanalyzer -c $(SRC)

# Allocation-failure injection: module TUs see -Dmalloc=fault_malloc.
fault:
	$(CC) $(CSTD) $(WARN) $(INC) -O0 -g -Dmalloc=fault_malloc -include fault_alloc.h -c luhn.c slist.c msort.c reservoir.c pfac.c
	$(CC) $(CSTD) $(WARN) $(INC) -O0 -g -c fault_alloc.c
	$(CC) $(CSTD) $(WARN) $(INC) -O0 -g -Dmalloc=fault_malloc -include fault_alloc.h -c fault_tests.c
	$(CC) $(CSTD) $(WARN) $(INC) -O0 -g luhn.o slist.o msort.o reservoir.o pfac.o fault_alloc.o fault_tests.o -o run_fault
	./run_fault
	$(CC) $(CSTD) $(WARN) $(INC) -O1 -g $(SAN) -Dmalloc=fault_malloc -include fault_alloc.h -c luhn.c slist.c msort.c reservoir.c pfac.c
	$(CC) $(CSTD) $(WARN) $(INC) -O1 -g $(SAN) -c fault_alloc.c
	$(CC) $(CSTD) $(WARN) $(INC) -O1 -g $(SAN) -Dmalloc=fault_malloc -include fault_alloc.h -c fault_tests.c
	$(CC) $(CSTD) $(WARN) $(INC) -O1 -g $(SAN) luhn.o slist.o msort.o reservoir.o pfac.o fault_alloc.o fault_tests.o -o run_fault_asan
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
	  done; \
	done
	@echo "OK: std matrix c99/c11/c17/c2x x gcc/clang"

# Portability matrix: both compilers across behavior, optimization, sanitizers.
gates:
	for cc in gcc clang; do $(MAKE) CC=$$cc debug release asan fault uaf || exit 1; done
	$(MAKE) fuzz analyze stdmatrix

clean:
	rm -f run_tests run_tests_release run_tests_asan run_fault run_fault_asan uaf_probe fuzz_test std_test *.o uaf_probe.out

.PHONY: all debug release asan fuzz analyze fault uaf stdmatrix gates clean
