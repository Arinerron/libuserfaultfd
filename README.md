# About
libuserfaultfd a library for CTF to help speed up your race condition kernel pwning (so you don't have to copy/paste as much code).

* [What is userfaultfd?](https://ctf-wiki.org/pwn/linux/kernel-mode/exploitation/userfaultfd/) (gotta use Google Translate sorry lol)

# Installation

This project requires the linux kernel headers to build.

```
$ git clone https://github.com/Arinerron/libuserfaultfd.git && cd libuserfaultfd
$ make
# make install
```

# Usage

```c
#include <libuserfaultfd.h>

struct request_t {
    ...
};

//////////

void func1(void *ptr) {
    // setup request here
    struct request_t *request = (struct request_t *)ptr;
    request->data = ...;
    request->xyz = ...;

    // make the first ioctl call here
    int fd = open(...);
    ioctl(fd, ..., ptr);
}

void func2(void *ptr) {
    // make the second ioctl here (the first ioctl is "paused" until this func returns)
    int fd = open(...);
    ioctl(fd, ..., ptr);
}

//////////

int main() {
    /* 
     * race(request size here, 
     *      first function, 
     *      second function, 
     *      number of copy_from/to_user's before pausing);
     */
    race(sizeof(struct request_t), func1, func2, 1);
}
```
