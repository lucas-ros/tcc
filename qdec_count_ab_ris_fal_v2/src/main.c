#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <zephyr/debug/cpu_load.h>

LOG_MODULE_REGISTER(pulse_counter, LOG_LEVEL_INF);

// ---- INPUTS ----
static const struct gpio_dt_spec pulseA = GPIO_DT_SPEC_GET(DT_ALIAS(digitalinput1), gpios);
static const struct gpio_dt_spec pulseB = GPIO_DT_SPEC_GET(DT_ALIAS(digitalinput2), gpios);

// ---- OUTPUT ----
static const struct gpio_dt_spec out1   = GPIO_DT_SPEC_GET(DT_ALIAS(digitaloutput1), gpios);

// ---- CALLBACK STRUCTS ----
static struct gpio_callback cb_A;
static struct gpio_callback cb_B;

// ---- COUNTERS ----
static volatile uint32_t rising_A = 0;
static volatile uint32_t falling_A = 0;

static volatile uint32_t rising_B = 0;
static volatile uint32_t falling_B = 0;

// ================================================================
// CALLBACK A
// ================================================================
static void gpio_callback_A(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    bool state = gpio_pin_get_dt(&pulseA);

    if (state) {
        rising_A++;
        gpio_pin_set_dt(&out1, 1);  // exemplo simples
    } else {
        falling_A++;
        gpio_pin_set_dt(&out1, 0);
    }
}

// ================================================================
// CALLBACK B
// ================================================================
static void gpio_callback_B(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    bool state = gpio_pin_get_dt(&pulseB);

    if (state) {
        rising_B++;
    } else {
        falling_B++;
    }
}

// ================================================================
// MAIN
// ================================================================
int main(void)
{
    int ret;

    // ----------------- CHECK READY -----------------
    if (!gpio_is_ready_dt(&pulseA) || !gpio_is_ready_dt(&pulseB)) {
        printk("Input GPIOs not ready\n");
        return 0;
    }

    if (!gpio_is_ready_dt(&out1)) {
        printk("Output GPIO not ready\n");
        return 0;
    }

    // ----------------- CONFIG INPUTS -----------------
    ret = gpio_pin_configure_dt(&pulseA, GPIO_INPUT);
    if (ret) return printk("PulseA cfg fail %d\n", ret), 0;

    ret = gpio_pin_configure_dt(&pulseB, GPIO_INPUT);
    if (ret) return printk("PulseB cfg fail %d\n", ret), 0;

    // ----------------- CONFIG OUTPUT -----------------
    ret = gpio_pin_configure_dt(&out1, GPIO_OUTPUT_INACTIVE);
    if (ret) return printk("Output cfg fail %d\n", ret), 0;

    // ----------------- CONFIG INTERRUPTS (BOTH EDGES) -----------------
    gpio_pin_interrupt_configure_dt(&pulseA, GPIO_INT_EDGE_BOTH);
    gpio_pin_interrupt_configure_dt(&pulseB, GPIO_INT_EDGE_BOTH);

    // ----------------- REGISTER CALLBACKS -----------------
    gpio_init_callback(&cb_A, gpio_callback_A, BIT(pulseA.pin));
    gpio_add_callback(pulseA.port, &cb_A);

    gpio_init_callback(&cb_B, gpio_callback_B, BIT(pulseB.pin));
    gpio_add_callback(pulseB.port, &cb_B);

    printk("Pulse counter started\n");

    // ----------------- LOOP -----------------
    while (1) {
        printk("A: rising=%u, falling=%u   B: rising=%u, falling=%u\n",
               rising_A, falling_A, rising_B, falling_B);

        int load = cpu_load_get(true);
        printk("CPU load = %d.%d%%\n", load / 10, load % 10);

        k_sleep(K_MSEC(500));
    }
}
