#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include <zephyr/debug/cpu_load.h>

LOG_MODULE_REGISTER(pulse_counter, LOG_LEVEL_INF);

static const struct gpio_dt_spec pulseA = GPIO_DT_SPEC_GET(DT_ALIAS(digitalinput1), gpios);
static const struct gpio_dt_spec pulseB = GPIO_DT_SPEC_GET(DT_ALIAS(digitalinput2), gpios);
static struct gpio_callback pulse_cb_data_a;
static struct gpio_callback pulse_cb_data_b;

static volatile uint32_t pulse_count_A = 0;
static volatile uint32_t pulse_count_B = 0;

void pulse_callback_A(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    pulse_count_A++;
}

void pulse_callback_B(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    pulse_count_B++;
}

int main(void)
{
    int ret;

    if (!gpio_is_ready_dt(&pulseA)) {
        printk("GPIO A not ready");
        return 0;
    }

    if (!gpio_is_ready_dt(&pulseB)) {
        printk("GPIO B not ready");
        return 0;
    }

    ret = gpio_pin_configure_dt(&pulseA, GPIO_INPUT);
    if (ret) {
        printk("Config A failed");
        return 0;
    }

    ret = gpio_pin_configure_dt(&pulseB, GPIO_INPUT);
    if (ret) {
        printk("Config B failed");
        return 0;
    }

    ret = gpio_pin_interrupt_configure_dt(&pulseA, GPIO_INT_EDGE_RISING);
    if (ret) {
        printk("Interrupt config failed");
        return 0;
    }

    ret = gpio_pin_interrupt_configure_dt(&pulseB, GPIO_INT_EDGE_RISING);
    if (ret) {
        printk("Interrupt config failed");
        return 0;
    }

    gpio_init_callback(&pulse_cb_data_a, pulse_callback_A, BIT(pulseA.pin));
    gpio_add_callback(pulseA.port, &pulse_cb_data_a);

    gpio_init_callback(&pulse_cb_data_b, pulse_callback_B, BIT(pulseB.pin));
    gpio_add_callback(pulseB.port, &pulse_cb_data_b);

    printk("Pulse counter started\n");

    while (1) {
        printk("Count A = %u\n", pulse_count_A);
        printk("Count B = %u\n", pulse_count_B);
        int cpu_load = cpu_load_get(true);
        printk("CPU load = %d.%d%%\n", cpu_load / 10, cpu_load % 10);
        k_sleep(K_MSEC(1000));
    }
}
