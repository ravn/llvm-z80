/* unistd.h — CP/M stub (no file descriptors, no fork/exec) */
#ifndef _CPM_UNISTD_H
#define _CPM_UNISTD_H

typedef unsigned int size_t;

/* On CP/M, write to fd 1 (stdout) goes via our printf path. */
static inline int write(int fd, const void *buf, size_t n) {
    (void)fd; (void)buf; (void)n; return -1;
}
static inline int read(int fd, void *buf, size_t n) {
    (void)fd; (void)buf; (void)n; return -1;
}

#endif
