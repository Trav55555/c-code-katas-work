/* Coverage-guided libFuzzer target for luhn_is_valid, the one parser of
 * arbitrary bytes. Built only by `make libfuzz` (clang, ASan+UBSan).
 * Oracle: an independent reference that collects the digits first and runs
 * the textbook algorithm over them. Fuzzing covers the explored corpus and
 * time budget only. */
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "luhn.h"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size);

/* Textbook Luhn over a NUL-terminated string: spaces ignored, any other
 * non-digit invalid, at least one digit required. */
static int luhn_reference(const char *s, size_t len)
{
    size_t i, ndigits = 0, pos;
    unsigned long sum = 0;
    for (i = 0; i < len; i++) {
        if (s[i] >= '0' && s[i] <= '9')
            ndigits++;
        else if (s[i] != ' ')
            return 0;
    }
    if (ndigits == 0)
        return 0;
    pos = 0;
    for (i = len; i > 0; i--) {
        unsigned d;
        if (s[i - 1] == ' ')
            continue;
        d = (unsigned)(s[i - 1] - '0');
        if (pos % 2 == 1)
            d = (d * 2 > 9) ? d * 2 - 9 : d * 2;
        sum = (sum + d) % 10;
        pos++;
    }
    return sum == 0;
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    char *s = malloc(size + 1);
    size_t len;
    if (s == NULL)
        return 0;
    memcpy(s, data, size);
    s[size] = '\0';
    len = strlen(s); /* an embedded NUL ends the string, per the contract */
    if (luhn_is_valid(s) != luhn_reference(s, len))
        abort();
    free(s);
    return 0;
}
