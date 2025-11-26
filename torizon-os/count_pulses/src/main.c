#include <stdio.h>
#include <gpiod.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>

int main(int argc, char *argv[]) {
    const char* gpiolineName_A = "SODIMM_135";
    const char* gpioChip_A = "/dev/gpiochip1";

    int is_running = 1;

    struct gpiod_line* gpioline_A = gpiod_line_find(gpiolineName_A);
    if (!gpioline_A) {
        fprintf(stderr, "Failed to find GPIO line: %s\n", gpiolineName_A);
        return 1;
    }

    struct gpiod_chip* chip_A = gpiod_line_get_chip(gpioline_A);
    if (!chip_A) {
        fprintf(stderr, "Failed to get GPIO chip_A for line: %s\n", gpiolineName_A);
        gpiod_line_release(gpioline_A);
        return 1;
    }

    // ⭐ BOTH EDGES
    if (gpiod_line_request_both_edges_events(gpioline_A, "pulse-counter") < 0) {
        fprintf(stderr, "Failed to request both-edge events\n");
        gpiod_chip_close(chip_A);
        return 1;
    }

    printf("Listening for pulses (rising + falling) on %s...\n", gpiolineName_A);

    FILE *log = fopen("linux_pulses.csv", "w");
    if (!log) {
        perror("fopen");
        return 1;
    }
    fprintf(log, "count,event_type,timestamp_sec,timestamp_nsec\n");

    unsigned long rising_count = 0;
    unsigned long falling_count = 0;
    unsigned long both_count = 0;
    
    struct timespec ts;
    const struct timespec timeout = {
        .tv_sec = 15,
        .tv_nsec = 0
    };

    while (is_running) {
        struct gpiod_line_event event;
        int ret = gpiod_line_event_wait(gpioline_A, &timeout);
        if (ret <= 0) {
            break;
        }

        if (gpiod_line_event_read(gpioline_A, &event) == 0) {

            clock_gettime(CLOCK_MONOTONIC, &ts);

            const char* type_str = "";
            if (event.event_type == GPIOD_LINE_EVENT_RISING_EDGE) {
                type_str = "rising";
                rising_count++;
            } else if (event.event_type == GPIOD_LINE_EVENT_FALLING_EDGE) {
                falling_count++;
                type_str = "falling";
            }

            both_count = falling_count + rising_count;

            fprintf(log, "%lu,%s,%ld,%ld\n",
                both_count, type_str, ts.tv_sec, ts.tv_nsec);
        }
    }

    printf("Total events counted: %lu\n", both_count);
    fclose(log);
    gpiod_line_release(gpioline_A);
    gpiod_chip_close(chip_A);
    return 0;
}
