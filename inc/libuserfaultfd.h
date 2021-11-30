#include <stddef.h>

/* 
 * race(request size here, 
 *      first function, 
 *      second function, 
 *      number of copy_from/to_user's before pausing);
 */
size_t race(size_t size, void (*func1)(void *), void (*func2)(void *), size_t n);
