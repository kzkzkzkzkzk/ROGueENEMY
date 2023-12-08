#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <stdatomic.h>
#include <inttypes.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <termios.h>
#include <dirent.h>

#include <sys/types.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/syscall.h>

#include <linux/hidraw.h>
#include <linux/input-event-codes.h>
#include <linux/uinput.h>
#include <linux/input.h>

#include <semaphore.h>

#include <pthread.h>

#include <libevdev-1.0/libevdev/libevdev.h>

#define LSB_PER_RAD_S_2000_DEG_S ((double)0.001064724)
#define LSB_PER_RAD_S_2000_DEG_S_STR "0.001064724"

#define LSB_PER_16G ((double)0.004785)
#define LSB_PER_16G_STR "0.004785"

// courtesy of linux kernel
#ifndef __packed
#define __packed	__attribute__((packed))
#endif

// also courtesy of linux kernel
inline int32_t div_round_closest(int32_t x, int32_t divisor) {
    const int32_t __x = x;
    const int32_t __d = divisor;
    return ((__x) > 0) == ((__d) > 0) ? (((__x) + ((__d) / 2)) / (__d)) : (((__x) - ((__d) / 2)) / (__d));
}