#ifndef STRS_H
#define STRS_H

/* In-place string katas (the Recursion set). `s` is non-NULL and
 * NUL-terminated in every function. No allocation.
 * Recursion depth is O(length): keep inputs within your stack budget
 * (the tests and fuzzer stay in the low kilobytes). */
void strs_reverse(char *s);                    /* in place */
void strs_replace(char *s, char from, char to);/* in place, all occurrences */
int  strs_is_palindrome(const char *s);        /* 1 yes, 0 no; empty -> 1 */

#endif