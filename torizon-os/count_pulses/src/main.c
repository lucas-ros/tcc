// #include <stdio.h>
// #include <gpiod.h>
// #include <unistd.h>
// #include <glob.h>

// int main(int argc, char *argv[]) {
//     const char* gpiolineName = "SODIMM_135";
//     const char* gpioChip = "/dev/gpiochip1";

//     if (gpiolineName == NULL || gpioChip == NULL) {
//         glob_t glob_result;
//         glob("/dev/gpiochip*", GLOB_TILDE, NULL, &glob_result);
//         for (size_t i = 0; i < glob_result.gl_pathc; ++i) {
//             const char* gpioChip = glob_result.gl_pathv[i];
//             struct gpiod_chip* chip = gpiod_chip_open(gpioChip);
            
//             int num_lines = gpiod_chip_num_lines(chip);
//             printf("\n\n%s - %d lines\n", gpiod_chip_name(chip), num_lines);

//             for (unsigned int line_offset = 0; line_offset < num_lines; line_offset++) {
//                 struct gpiod_line* line = gpiod_chip_get_line(chip, line_offset);

//                 const char* line_name = gpiod_line_name(line);
//                 if (line_name == NULL) {
//                     line_name = "unnamed";
//                 }

//                 const char* line_consumer;
//                 if (gpiod_line_is_used(line)) {
//                     line_consumer = gpiod_line_consumer(line);
//                 } else {
//                     line_consumer = "unused";
//                 }

//                 const char* line_direction;
//                 if (gpiod_line_direction(line) == GPIOD_LINE_DIRECTION_INPUT) {
//                     line_direction = "input";
//                 } else {
//                     line_direction = "output";
//                 }

//                 printf("Line %-2u:  %-15s  %-20s  %-10s\n", line_offset, line_name, line_consumer, line_direction);

//                 gpiod_line_release(line);
//             }

//             gpiod_chip_close(chip);
//         }
//         globfree(&glob_result);
//     } else {
//         struct gpiod_line* gpioline = gpiod_line_find(gpiolineName);

//         if (gpioline == NULL) {
//             printf("Invalid line name.\n");
//         } else {
//             gpiod_line_request_output(gpioline, "GPIO application", 0);
//             while (1) {
//                 gpiod_line_set_value(gpioline, 0);
//                 usleep(1000000);
//                 gpiod_line_set_value(gpioline, 1);
//                 usleep(1000000);
//             }
//         }
//     }

//     return 0;
// }

// #include <stdio.h>
// #include <gpiod.h>
// #include <unistd.h>
// #include <stdlib.h>
// #include <signal.h>

// static volatile int keepRunning = 1;

// void intHandler(int dummy) {
//     keepRunning = 0;
// }

// int main(int argc, char *argv[]) {
//     const char* gpiolineName = "SODIMM_135";
//     const char* gpioChip = "/dev/gpiochip1";

//     // Handle Ctrl+C
//     signal(SIGINT, intHandler);

//     // Find line by name
//     struct gpiod_line* gpioline = gpiod_line_find(gpiolineName);
//     if (!gpioline) {
//         fprintf(stderr, "Failed to find GPIO line: %s\n", gpiolineName);
//         return 1;
//     }

//     struct gpiod_chip* chip = gpiod_line_get_chip(gpioline);
//     if (!chip) {
//         fprintf(stderr, "Failed to get GPIO chip for line: %s\n", gpiolineName);
//         gpiod_line_release(gpioline);
//         return 1;
//     }

//     // Request input with edge detection
//     if (gpiod_line_request_falling_edge_events(gpioline, "pulse-counter") < 0) {
//         fprintf(stderr, "Failed to request falling edge events\n");
//         gpiod_chip_close(chip);
//         return 1;
//     }

//     printf("Listening for pulses on %s...\n", gpiolineName);
//     unsigned long count = 0;

//     while (keepRunning) {
//         struct gpiod_line_event event;
//         int ret = gpiod_line_event_wait(gpioline, NULL);
//         if (ret < 0) {
//             perror("gpiod_line_event_wait");
//             break;
//         } else if (ret == 0) {
//             // timeout expired, you can ignore or handle it
//             continue;
//         }

//         if (gpiod_line_event_read(gpioline, &event) == 0) {
//             if (event.event_type == GPIOD_LINE_EVENT_FALLING_EDGE) {
//                 count++;
//                 printf("Pulse #%lu detected\n", count);
//             }
//         } else {
//             perror("gpiod_line_event_read");
//             break;
//         }
//     }

//     printf("Total pulses counted: %lu\n", count);

//     gpiod_line_release(gpioline);
//     gpiod_chip_close(chip);
//     return 0;
// }

#include <stdio.h>
#include <gpiod.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>

// static volatile int keepRunning = 1;

// void intHandler(int dummy) {
//     keepRunning = 0;
// }

int main(int argc, char *argv[]) {
    const char* gpiolineName = "SODIMM_135";
    const char* gpioChip = "/dev/gpiochip1";

    int is_running = 1;
    // signal(SIGINT, intHandler);

    struct gpiod_line* gpioline = gpiod_line_find(gpiolineName);
    if (!gpioline) {
        fprintf(stderr, "Failed to find GPIO line: %s\n", gpiolineName);
        return 1;
    }

    struct gpiod_chip* chip = gpiod_line_get_chip(gpioline);
    if (!chip) {
        fprintf(stderr, "Failed to get GPIO chip for line: %s\n", gpiolineName);
        gpiod_line_release(gpioline);
        return 1;
    }

    if (gpiod_line_request_falling_edge_events(gpioline, "pulse-counter") < 0) {
        fprintf(stderr, "Failed to request falling edge events\n");
        gpiod_chip_close(chip);
        return 1;
    }

    printf("Listening for pulses on %s...\n", gpiolineName);

    FILE *log = fopen("linux_pulses.csv", "w");
    if (!log) {
        perror("fopen");
        return 1;
    }
    fprintf(log, "count,timestamp_sec,timestamp_nsec\n");

    unsigned long count = 0;
    struct timespec ts;
    const struct timespec timeout = {
        .tv_sec = 60,   // segundos
        .tv_nsec = 0    // nanossegundos
    };

    while (is_running) {
        struct gpiod_line_event event;
        int ret = gpiod_line_event_wait(gpioline, &timeout);
        if (ret <= 0) continue;

        if (gpiod_line_event_read(gpioline, &event) == 0) {
            if (event.event_type == GPIOD_LINE_EVENT_FALLING_EDGE) {
                clock_gettime(CLOCK_MONOTONIC, &ts);
                count++;
                fprintf(log, "%lu,%ld,%ld\n", count, ts.tv_sec, ts.tv_nsec);
            }
        }
    }

    printf("Total pulses counted: %lu\n", count);
    fclose(log);
    gpiod_line_release(gpioline);
    gpiod_chip_close(chip);
    return 0;
}

