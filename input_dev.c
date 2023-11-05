#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>

#include "input_dev.h"

#include <dirent.h> 
#include <stdio.h> 

static const char *input_path = "/dev/input/";

static struct libevdev* ev_matches(const char* sysfs_entry, const uinput_filters_t* const filters) {
    struct libevdev *dev = NULL;

    int fd = open(sysfs_entry, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "Cannot open %s, device skipped.\n", sysfs_entry);
        return NULL;
    }

    if (libevdev_new_from_fd(fd, &dev) != 0) {
        fprintf(stderr, "Cannot initialize libevdev from this device (%s): skipping.\n", sysfs_entry);
        close(fd);
        return NULL;
    }

    const char* name = libevdev_get_name(dev);
    if (strcmp(name, filters->name) != 0) {
        fprintf(stderr, "The device name (%s) for device %s does not matches the expected one %s.\n", name, sysfs_entry, filters->name);
        libevdev_free(dev);
        close(fd);
        return NULL;
    }

    const int grab_res = libevdev_grab(dev, LIBEVDEV_GRAB);
    if (grab_res != 0) {
        fprintf(stderr, "Unable to grab the device (%s): %d.\n", sysfs_entry, grab_res);
        //libevdev_free(dev);
        //close(fd);
        return dev;
    }

    return dev;
}

static pthread_mutex_t input_acquire_mutex = PTHREAD_MUTEX_INITIALIZER;

static char* open_sysfs[] = {
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
};

void *input_dev_thread_func(void *ptr) {
    input_dev_t *in_dev = (input_dev_t*)ptr;

    struct libevdev* dev = NULL;
    int open_sysfs_idx = -1;

    for (;;) {
        const uint32_t flags = in_dev->crtl_flags;
        if (flags & INPUT_DEV_CTRL_FLAG_EXIT) {
            in_dev->crtl_flags &= ~INPUT_DEV_CTRL_FLAG_EXIT;
            break;
        }

        // clean up from previous iteration
        if (dev != NULL) {
            libevdev_free(dev);
            dev = NULL;
        }

        const int input_acquire_lock_result = pthread_mutex_lock(&input_acquire_mutex);
        if (input_acquire_lock_result != 0) {
            fprintf(stderr, "Cannot lock input mutex: %d, will retry later...\n", input_acquire_lock_result);
            usleep(250000);
            continue;
        }

        // clean up leftover from previous opening
        if (open_sysfs_idx >= 0) {
            free(open_sysfs[open_sysfs_idx]);
            open_sysfs[open_sysfs_idx] = NULL;
        }

        char path[512] = "\0";
        
        DIR *d;
        struct dirent *dir;
        d = opendir(input_path);
        if (d) {
            while ((dir = readdir(d)) != NULL) {
                if (dir->d_name[0] == '.') {
                    continue;
                } else if (dir->d_name[0] == 'b') { // by-id
                    continue;
                } else if (dir->d_name[0] == 'j') { // js-0
                    continue;
                }

                sprintf(path, "%s%s", input_path, dir->d_name);

                // check if that has been already opened
                // open_sysfs
                int skip = 0;
                for (int o = 0; o < (sizeof(open_sysfs) / sizeof(const char*)); ++o) {
                    if ((open_sysfs[o] != NULL) && (strcmp(open_sysfs[o], path) == 0)) {
                        skip = 1;
                        break;
                    }
                }

                if (skip) {
                    continue;
                }

                // try to open the device
                dev = ev_matches(path, in_dev->ev_filters);
                if (dev != NULL) {
                    open_sysfs_idx = 0;
                    while (open_sysfs[open_sysfs_idx] != NULL) {
                        ++open_sysfs_idx;
                    }
                    open_sysfs[open_sysfs_idx] = malloc(sizeof(path));
                    memcpy(open_sysfs[open_sysfs_idx], path, 512);    

                    printf("Opened device %s\n    name: %s", path, libevdev_get_name(dev));
                    break;
                }
            }
            closedir(d);
        }

        pthread_mutex_unlock(&input_acquire_mutex);

        if (dev == NULL) {
            usleep(250000);
            continue;
        }

        for (;;) {
            // TODO: do the required
            //process_events(dev);

            const uint32_t flags = in_dev->crtl_flags;
            if (flags & INPUT_DEV_CTRL_FLAG_EXIT) {
                break;
            }
        }
    }

    return NULL;
}
