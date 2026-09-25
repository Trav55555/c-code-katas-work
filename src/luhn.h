#ifndef LUHN_H
#define LUHN_H

/* Luhn checksum validation over an untrusted ASCII digit string.
 *
 * Contract:
 *  - `digits` is NUL-terminated, or NULL (treated as invalid: returns 0);
 *    arbitrary untrusted bytes are accepted. ASCII digits count; a space ' ' is skipped anywhere; any other
 *    byte (control bytes included) makes the number invalid.
 *  - At least one digit is required: empty and all-spaces strings are invalid.
 *  - Length is unbounded. Cost is linear, no allocation, no mutation of input.
 *  - Digit-sum accumulation is kept in [0,9] (running modulo 10), so no
 *    integer width bounds the accepted length.
 *
 * Returns 1 when the checksum validates, 0 otherwise. Total function: cannot
 * fail for any input bytes. Embedded NUL ends the string (C string semantics). */
int luhn_is_valid(const char *digits);

#endif
