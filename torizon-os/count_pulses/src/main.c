#include <stdio.h>
#include <gpiod.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>

int main(int argc, char *argv[]) {
    const char* gpiolineName = "SODIMM_135";
    const char* gpioChip = "/dev/gpiochip1";

    int is_running = 1;

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
        .tv_sec = 60,
        .tv_nsec = 0
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

