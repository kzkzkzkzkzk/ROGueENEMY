#pragma once

#include "rogue_enemy.h"

#define OUTPUT_DEV_VENDOR_ID      0x4532
#define OUTPUT_DEV_PRODUCT_ID     0x0924

#define OUTPUT_DEV_CTRL_FLAG_EXIT 0x00000001U
#define OUTPUT_DEV_CTRL_FLAG_DATA 0x00000002U

#define ACCEL_RANGE 512
#define GYRO_RANGE  2000 // max range is +/- 35 radian/s

#define GYRO_DEADZONE 1 // degrees/s to count as zero movement

typedef enum output_dev_type {
    output_dev_gamepad,
    output_dev_imu,
} output_dev_type_t;

typedef struct output_dev {
    int fd;

    pthread_mutex_t ctrl_mutex;
    uint32_t crtl_flags;

    uint32_t max_events;
    uint32_t events_count;
    struct input_event *events_list;
} output_dev_t;

int create_output_dev(const char* uinput_path, const char* name, output_dev_type_t type);

void *output_dev_thread_func(void *ptr);