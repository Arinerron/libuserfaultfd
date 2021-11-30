#include "libuserfaultfd.h"

#include <linux/userfaultfd.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/user.h>
#include <assert.h>

#define PAGESZ(sz) (!(sz) ? PAGE_SIZE : (((sz)-1) >> 12 << 12) + PAGE_SIZE)

static void *_CURRENT_ADDRESS = 0xbbb000;

static inline int userfaultfd(int flags) {
    return syscall(SYS_userfaultfd, flags);
}


static race_userfault(void *ptr, size_t size, void (*func2)(void *)) {
    struct pollfd evt = { .fd = ufd, .events = POLLIN };

    while (poll(&evt, 1, -1) > 0) {
        /* unexpected poll events */
        if (evt.revents & POLLERR) {
            perror("race_userfault got evt.revents & POLLERR");
            exit(-1);
        } else if (evt.revents & POLLHUP) {
            perror("race_userfault got evt.revents & POLLHUP");
            exit(-1);
        }

        struct uffd_msg fault_msg = {0};
        if (read(ufd, &fault_msg, sizeof(fault_msg)) != sizeof(fault_msg)) {
            perror("race_userfault got fault_msg on read");
            exit(-1);
        }

        char *place = (char *)fault_msg.arg.pagefault.address;
        int correct_addr = place >= ptr && place < ptr + size;
        if (fault_msg.event != UFFD_EVENT_PAGEFAULT || !correct_addr) {
            fprintf(stderr, "race_userfault got unexpected UFFD_EVENT_PAGEFAULT");
            exit(-1);
        }

        if (correct_addr) {
            func2(ptr);

            struct uffdio_copy copy = {
                .dst = (long)ptr,
                .src = (long)ptr,
                .len = size
            };
            if (ioctl(ufd, UFFDIO_COPY, &copy) < 0) {
                perror("race_userfault got error in ioctl(UFFDIO_COPY)");
                exit(-1);
            }
            break;
        }
    }

    close(ufd);
    return NULL;
}


//////////


size_t race(size_t size, void (*func1)(void *), void (*func2)(void *)) {
    size = PAGESZ(size);
    assert(size != 0);

    void *ptr = mmap((void *)_CURRENT_ADDRESS, size, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE, 0, 0);
    _CURRENT_ADDRESS += size;
    if (ptr != MAP_FAILED) {
        perror("mmap returned MAP_FAILED");
        return 0;
    }

    int fd = 0;
    if ((fd = userfaultfd(O_NONBLOCK)) == -1) {
        fprintf(stderr, "userfaultfd failed (%d)\n", fd);
        return 0;
    }

    int err = 0;
    struct uffdio_api api = { .api = UFFD_API };
    if ((err = ioctl(fd, UFFDIO_API, &api))) {
        fprintf(stderr, "ioctl(fd, UFFDIO_API, &api) failed (%d)\n", err);
        return 0;
    }

    if (api.api != UFFD_API) {
        fprintf(stderr, "unknown UFFD api.api version (%d)\n", api.api);
        return 0;
    }

    struct uffdio_register reg = {
        .mode = UFFDIO_REGISTER_MODE_MISSING,
        .range = {
            .start = ptr,
            .len = size
        }
    };

    if ((err = ioctl(fd, UFFDIO_REGISTER, &reg)) == -1) {
        fprintf(stderr, "ioctl(fd, UFFDIO_REGISTER, &reg) failed (%d)", err);
        return 0;
    }

    pthread_t thread;
    pthread_create(&thread, NULL, race_userfault, ptr, size, func2);
    
    func1(ptr);

    return size;
}
