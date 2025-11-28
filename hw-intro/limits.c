#include <stdio.h>
#include <sys/resource.h>

int main() {
    struct rlimit lim;
    int err, errcnt = 0;
    
    err = getrlimit(RLIMIT_STACK, &lim);
    if (err == 0) {
        printf("stack size: %ld\n", lim.rlim_cur);
    } else {
        errcnt <<= 1;
        errcnt++;
    }

    err = getrlimit(RLIMIT_NPROC, &lim); // ! Docker does not set a per-user process limit
    if (err == 0) {
        printf("process limit: %ld\n", lim.rlim_cur);
    } else {
        errcnt <<= 1;
        errcnt++;
    }

    err = getrlimit(RLIMIT_NOFILE, &lim);
    if (err == 0) {
        printf("max file descriptors: %ld\n", lim.rlim_cur);
    } else {
        errcnt <<= 1;
        errcnt++;
    }

    return errcnt;
}
