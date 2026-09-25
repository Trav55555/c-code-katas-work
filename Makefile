CC ?= clang
CSTD = -std=c99
WARN = -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wstrict-prototypes -Werror
INC = -Isrc -Itests
SAN = -fsanitize=address,undefined -fno-sanitize-recover=all
B = build

MODULES = luhn slist msort reservoir pfac sorts search dlist bst num strs recmath euler
SRC = $(MODULES:%=src/%.c)
FAULT_MODULES = slist msort dlist bst

all: debug

$(B):
	mkdir -p $(B)

debug: | $(B)
	$(CC) $(CSTD) $(WARN) $(INC) -O0 -g $(SRC) tests/tests.c -o $(B)/run_tests
	./$(B)/run_tests
	$(CC) $(CSTD) $(WARN) $(INC) -O0 -g $(SRC) tests/tests_rest.c -o $(B)/run_rest
	./$(B)/run_rest

release: | $(B)
	$(CC) $(CSTD) $(WARN) $(INC) -O2 -DNDEBUG $(SRC) tests/tests.c -o $(B)/run_tests_release
	./$(B)/run_tests_release
	$(CC) $(CSTD) $(WARN) $(INC) -O2 -DNDEBUG $(SRC) tests/tests_rest.c -o $(B)/run_rest_release
	./$(B)/run_rest_release

asan: | $(B)
	$(CC) $(CSTD) $(WARN) $(INC) -O1 -g $(SAN) $(SRC) tests/tests.c -o $(B)/run_tests_asan
	./$(B)/run_tests_asan
	$(CC) $(CSTD) $(WARN) $(INC) -O1 -g $(SAN) $(SRC) tests/tests_rest.c -o $(B)/run_rest_asan
	./$(B)/run_rest_asan

fuzz: | $(B)
	$(CC) $(CSTD) $(WARN) $(INC) -O1 -g $(SAN) $(SRC) fuzz/fuzz.c -o $(B)/fuzz_test
	./$(B)/fuzz_test

# Coverage-guided fuzzing of the byte parser (clang libFuzzer). Bounded by
# time; crashes land in build/fuzz-artifacts/ and should become regression tests.
LIBFUZZ_SECONDS ?= 20
libfuzz: | $(B)
	mkdir -p $(B)/fuzz-artifacts
	clang $(CSTD) $(WARN) $(INC) -O1 -g -fsanitize=fuzzer,address,undefined -fno-sanitize-recover=all src/luhn.c fuzz/fuzz_luhn_lf.c -o $(B)/fuzz_luhn_lf
	./$(B)/fuzz_luhn_lf -max_total_time=$(LIBFUZZ_SECONDS) -max_len=4096 \
		-artifact_prefix=$(B)/fuzz-artifacts/ -print_final_stats=1 \
		> $(B)/fuzz-artifacts/libfuzz.log 2>&1; status=$$?; \
		grep -E 'Done|number_of_executed_units|ERROR|SUMMARY|deadly' $(B)/fuzz-artifacts/libfuzz.log; \
		exit $$status

analyze: | $(B)
	cd $(B) && gcc $(CSTD) $(WARN) -I../src -fanalyzer -c $(SRC:%=../%)

# Allocation-failure injection: module TUs see -Dmalloc=fault_malloc.
# Plain and ASan objects live in separate directories so they never mix.
fault: | $(B)
	$(MAKE) fault-build FB=$(B)/fault FFLAGS="-O0 -g" RUN=run_fault
	$(MAKE) fault-build FB=$(B)/fault-asan FFLAGS="-O1 -g $(SAN)" RUN=run_fault_asan

fault-build:
	mkdir -p $(FB)
	for m in $(FAULT_MODULES); do \
	  $(CC) $(CSTD) $(WARN) $(INC) $(FFLAGS) -Dmalloc=fault_malloc -include tests/fault_alloc.h -c src/$$m.c -o $(FB)/$$m.o || exit 1; \
	done
	$(CC) $(CSTD) $(WARN) $(INC) $(FFLAGS) -c tests/fault_alloc.c -o $(FB)/fault_alloc.o
	$(CC) $(CSTD) $(WARN) $(INC) $(FFLAGS) -Dmalloc=fault_malloc -include tests/fault_alloc.h -c tests/fault_tests.c -o $(FB)/fault_tests.o
	$(CC) $(CSTD) $(WARN) $(INC) $(FFLAGS) $(FAULT_MODULES:%=$(FB)/%.o) $(FB)/fault_alloc.o $(FB)/fault_tests.o -o $(B)/$(RUN)
	./$(B)/$(RUN)

# Negative test: the documented borrowed-pointer invalidation must trip ASan.
uaf: | $(B)
	$(CC) $(CSTD) $(WARN) $(INC) -O1 -g $(SAN) $(SRC) tests/uaf_probe.c -o $(B)/uaf_probe
	@if ./$(B)/uaf_probe > $(B)/uaf_probe.out 2>&1; then \
		echo "FAIL: use-after-free not detected"; exit 1; \
	elif grep -q heap-use-after-free $(B)/uaf_probe.out; then \
		echo "OK: ASan caught the use-after-free"; \
	else \
		echo "FAIL: wrong failure mode"; cat $(B)/uaf_probe.out; exit 1; \
	fi

# Language-mode portability: compile and run under every supported -std.
stdmatrix: | $(B)
	for std in c99 c11 c17 c2x; do \
	  for cc in gcc clang; do \
	    $$cc -std=$$std $(WARN) $(INC) -O0 $(SRC) tests/tests.c -o $(B)/std_test || exit 1; \
	    ./$(B)/std_test >/dev/null || exit 1; \
	    $$cc -std=$$std $(WARN) $(INC) -O0 $(SRC) tests/tests_rest.c -o $(B)/std_rest || exit 1; \
	    ./$(B)/std_rest >/dev/null || exit 1; \
	  done; \
	done
	@echo "OK: std matrix c99/c11/c17/c2x x gcc/clang"

# Portability matrix: both compilers across behavior, optimization, sanitizers.
gates:
	for cc in gcc clang; do $(MAKE) CC=$$cc debug release asan fault uaf || exit 1; done
	$(MAKE) fuzz libfuzz analyze stdmatrix

clean:
	rm -rf $(B)

.PHONY: all debug release asan fuzz libfuzz analyze fault fault-build uaf stdmatrix gates clean
