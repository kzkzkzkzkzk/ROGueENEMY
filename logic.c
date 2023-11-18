#include "logic.h"
#include "platform.h"
#include "virt_ds4.h"

int logic_create(logic_t *const logic) {
    logic->flags = 0x00000000U;

    memset(logic->gamepad.joystick_positions, 0, sizeof(logic->gamepad.joystick_positions));
    logic->gamepad.dpad = DPAD_RELEASED;
    logic->gamepad.l2_trigger = 0;
    logic->gamepad.r2_trigger = 0;
    logic->gamepad.triangle = 0;
    logic->gamepad.circle = 0;
    logic->gamepad.cross = 0;
    logic->gamepad.square = 0;
    logic->gamepad.r3 = 0;
    logic->gamepad.r3 = 0;
    logic->gamepad.option = 0;
    logic->gamepad.share = 0;
    logic->gamepad.center = 0;
    logic->gamepad.gyro_x = 0;
    logic->gamepad.gyro_y = 0;
    logic->gamepad.gyro_z = 0;
    logic->gamepad.accel_x = 0;
    logic->gamepad.accel_y = 0;
    logic->gamepad.accel_z = 0;
    
    const int mutex_creation_res = pthread_mutex_init(&logic->gamepad_mutex, NULL);
    if (mutex_creation_res != 0) {
        fprintf(stderr, "Unable to create mutex: %d\n", mutex_creation_res);
        return mutex_creation_res;
    }

    const int queue_init_res = queue_init(&logic->input_queue, 128);
    
    const int virt_ds4_thread_creation = pthread_create(&logic->virt_ds4_thread, NULL, virt_ds4_thread_func, (void*)(logic));
	if (virt_ds4_thread_creation != 0) {
		fprintf(stderr, "Error creating virtual DualShock4 thread: %d\n", virt_ds4_thread_creation);
	} else {
        logic->flags |= LOGIC_FLAGS_VIRT_DS4_ENABLE;
    }

    if (queue_init_res < 0) {
        fprintf(stderr, "Unable to create queue: %d\n", queue_init_res);
        return queue_init_res;
    }

    const int init_platform_res = init_platform(&logic->platform);
    if (init_platform_res == 0) {
        logic->flags |= LOGIC_FLAGS_PLATFORM_ENABLE;
    } else {
        fprintf(stderr, "Unable to initialize Asus RC71L MCU: %d", init_platform_res);
    }

    return 0;
}

int is_rc71l_ready(const logic_t *const logic) {
    return logic->flags & LOGIC_FLAGS_PLATFORM_ENABLE;
}

int logic_copy_gamepad_status(logic_t *const logic, gamepad_status_t *const out) {
    int res = 0;

    res = pthread_mutex_lock(&logic->gamepad_mutex);
    if (res != 0) {
        goto logic_copy_gamepad_status_err;
    }

    *out = logic->gamepad;
    pthread_mutex_unlock(&logic->gamepad_mutex);

logic_copy_gamepad_status_err:
    return res;
}


int logic_begin_status_update(logic_t *const logic) {
    int res = 0;

    res = pthread_mutex_lock(&logic->gamepad_mutex);
    if (res != 0) {
        goto logic_begin_status_update_err;
    }

logic_begin_status_update_err:
    return res;
}

void logic_end_status_update(logic_t *const logic) {
    pthread_mutex_unlock(&logic->gamepad_mutex);
}