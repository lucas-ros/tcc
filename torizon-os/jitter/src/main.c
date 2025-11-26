#include <stdio.h>
#include <gpiod.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>

int main(int argc, char *argv[]) {
    const char* gpiolineName_A = "SODIMM_135";
    const char* gpioChip_A = "/dev/gpiochip1";

    const char* gpiolineName_B = "SODIMM_133";
    const char* gpioChip_B = "/dev/gpiochip2";

    int is_running = 1;

    struct gpiod_line* gpioline_A = gpiod_line_find(gpiolineName_A);
    if (!gpioline_A) {
        fprintf(stderr, "Failed to find GPIO line: %s\n", gpiolineName_A);
        return 1;
    }

    struct gpiod_line* gpioline_B = gpiod_line_find(gpiolineName_B);
    if (!gpioline_B) {
        fprintf(stderr, "Failed to find GPIO line: %s\n", gpiolineName_B);
        return 1;
    }

    struct gpiod_chip* chip_A = gpiod_line_get_chip(gpioline_A);
    if (!chip_A) {
        fprintf(stderr, "Failed to get GPIO chip_A for line: %s\n", gpiolineName_A);
        gpiod_line_release(gpioline_A);
        return 1;
    }

    struct gpiod_chip* chip_B = gpiod_line_get_chip(gpioline_B);
    if (!chip_B) {
        fprintf(stderr, "Failed to get GPIO chip_B for line: %s\n", gpiolineName_B);
        gpiod_line_release(gpioline_B);
        return 1;
    }

    const char* gpiolineName_out = "SODIMM_98";
    const char* gpioChip_out = "/dev/gpiochip5";

    struct gpiod_line* gpioline_out = gpiod_line_find(gpiolineName_out);
    if (!gpioline_out) {
        fprintf(stderr, "Failed to find GPIO line: %s\n", gpiolineName_out);
        return 1;
    }
    
    struct gpiod_chip* chip_out = gpiod_line_get_chip(gpioline_out);
    if (!chip_out) {
        fprintf(stderr, "Failed to get GPIO chip_A for line: %s\n", gpiolineName_out);
        gpiod_line_release(gpioline_out);
        return 1;
    }
    
    if (gpiod_line_request_output(gpioline_out, "pulse-output", 0) < 0) {
        fprintf(stderr, "Failed to request output line\n");
        return 1;
    }
    
    gpiod_line_set_value(gpioline_out, 1);

    // ⭐ BOTH EDGES
    if (gpiod_line_request_both_edges_events(gpioline_A, "pulse-counter") < 0) {
        fprintf(stderr, "Failed to request both-edge events\n");
        gpiod_chip_close(chip_A);
        return 1;
    }

    // ⭐ BOTH EDGES
    if (gpiod_line_request_both_edges_events(gpioline_B, "pulse-counter") < 0) {
        fprintf(stderr, "Failed to request both-edge events\n");
        gpiod_chip_close(chip_B);
        return 1;
    }

    printf("Listening for pulses (rising + falling) on %s...\n", gpiolineName_A);
    printf("Listening for pulses (rising + falling) on %s...\n", gpiolineName_B);

    // FILE *log = fopen("linux_pulses.csv", "w");
    // if (!log) {
    //     perror("fopen");
    //     return 1;
    // }
    // fprintf(log, "count,event_type,timestamp_sec,timestamp_nsec\n");

    unsigned long rising_count_A = 0;
    unsigned long falling_count_A = 0;

    unsigned long rising_count_B = 0;
    unsigned long falling_count_B = 0;
    
    // struct timespec ts;
    const struct timespec timeout = {
        .tv_sec = 20,
        .tv_nsec = 0
    };

    while (is_running) {
        struct gpiod_line_event event;
        int ret = gpiod_line_event_wait(gpioline_A, &timeout);
        if (ret <= 0) {
            break;
        }

        if (gpiod_line_event_read(gpioline_A, &event) == 0) {

            // clock_gettime(CLOCK_MONOTONIC, &ts);

            // const char* type_str = "";
            if (event.event_type == GPIOD_LINE_EVENT_RISING_EDGE) {
                // type_str = "rising";
                rising_count_A++;
                gpiod_line_set_value(gpioline_out, 1);
            } else if (event.event_type == GPIOD_LINE_EVENT_FALLING_EDGE) {
                falling_count_A++;
                // type_str = "falling";
                gpiod_line_set_value(gpioline_out, 0);
            }

        }

        ret = gpiod_line_event_wait(gpioline_B, &timeout);
        if (ret <= 0) {
            break;
        }

        if (gpiod_line_event_read(gpioline_B, &event) == 0) {

            // clock_gettime(CLOCK_MONOTONIC, &ts);

            // const char* type_str = "";
            if (event.event_type == GPIOD_LINE_EVENT_RISING_EDGE) {
                // type_str = "rising";
                rising_count_B++;
                // gpiod_line_set_value(gpioline_out, 1);
            } else if (event.event_type == GPIOD_LINE_EVENT_FALLING_EDGE) {
                falling_count_B++;
                // type_str = "falling";
                // gpiod_line_set_value(gpioline_out, 0);
            }
        }
    }

    // printf("Total events counted: %lu\n", both_count);
    // fclose(log);
    gpiod_line_release(gpioline_A);
    gpiod_chip_close(chip_A);
    gpiod_line_release(gpioline_B);
    gpiod_chip_close(chip_B);
    gpiod_line_release(gpioline_out);
    gpiod_chip_close(chip_out);

    FILE *summary = fopen("pulses_summary.txt", "w");
    if (!summary) {
        perror("fopen summary");
    } else {
    fprintf(summary, "Resultados finais da contagem de bordas:\n\n");

    fprintf(summary, "Canal A (%s):\n", gpiolineName_A);
    fprintf(summary, "  Rising edges : %lu\n", rising_count_A);
    fprintf(summary, "  Falling edges: %lu\n", falling_count_A);
    fprintf(summary, "  Total        : %lu\n\n",
            rising_count_A + falling_count_A);

    fprintf(summary, "Canal B (%s):\n", gpiolineName_B);
    fprintf(summary, "  Rising edges : %lu\n", rising_count_B);
    fprintf(summary, "  Falling edges: %lu\n", falling_count_B);
    fprintf(summary, "  Total        : %lu\n",
            rising_count_B + falling_count_B);

    fclose(summary);
}
    return 0;
}
