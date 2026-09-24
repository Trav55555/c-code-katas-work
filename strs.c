#include "strs.h"

#include <assert.h>
#include <string.h>

static void reverse_rec(char *s, size_t lo, size_t hi)
{
    if (lo >= hi)
        return;
    {
        char t = s[lo];
        s[lo] = s[hi];
        s[hi] = t;
    }
    reverse_rec(s, lo + 1, hi - 1);
}

void strs_reverse(char *s)
{
    size_t len;
    assert(s != NULL);
    len = strlen(s);
    if (len > 0)
        reverse_rec(s, 0, len - 1);
}

static void replace_rec(char *s, char from, char to)
{
    if (*s == '\0')
        return;
    if (*s == from)
        *s = to;
    replace_rec(s + 1, from, to);
}

void strs_replace(char *s, char from, char to)
{
    assert(s != NULL);
    replace_rec(s, from, to);
}

static int palindrome_rec(const char *s, size_t lo, size_t hi)
{
    if (lo >= hi)
        return 1;
    if (s[lo] != s[hi])
        return 0;
    return palindrome_rec(s, lo + 1, hi - 1);
}

int strs_is_palindrome(const char *s)
{
    size_t len;
    assert(s != NULL);
    len = strlen(s);
    return palindrome_rec(s, 0, (len > 0) ? len - 1 : 0);
}