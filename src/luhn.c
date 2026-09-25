#include "luhn.h"

#include <stddef.h>

int luhn_is_valid(const char *digits)
{
    size_t len = 0, i, pos = 0;
    int sum = 0;

    if (digits == NULL)
        return 0;

    /* Pass 1: validate the byte set and measure length.
     * Reject anything but ASCII digits and spaces (untrusted-input gate). */
    for (i = 0; digits[i] != '\0'; i++) {
        char c = digits[i];
        if (!((c >= '0' && c <= '9') || c == ' '))
            return 0;
    }
    len = i;

    /* Pass 2: right-to-left over digits only, doubling every second digit.
     * `pos` counts digits seen (spaces don't shift parity). sum stays in
     * [0,9] via running modulo, so arbitrarily long inputs cannot overflow. */
    for (i = len; i > 0; i--) {
        char c = digits[i - 1];
        int d;
        if (c == ' ')
            continue;
        d = c - '0';
        if ((pos % 2) == 1) {
            d *= 2;
            if (d > 9)
                d -= 9;
        }
        sum = (sum + d) % 10;
        pos++;
    }

    if (pos == 0)
        return 0; /* no digits at all */
    return sum == 0;
}
