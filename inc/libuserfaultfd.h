#include <stddef.h>

size_t race(size_t size, void (*func1)(void *), void (*func2)(void *));
