#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(pulse_counter, LOG_LEVEL_INF);


static const struct gpio_dt_spec pulse = GPIO_DT_SPEC_GET(DT_ALIAS(digitalinput1), gpios);
static struct gpio_callback pulse_cb_data;

static volatile uint32_t pulse_count = 0;

void pulse_callback(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    pulse_count++;
}

int main(void)
{
    int ret;

    if (!gpio_is_ready_dt(&pulse)) {
        printk("GPIO not ready");
        return 0;
    }

    ret = gpio_pin_configure_dt(&pulse, GPIO_INPUT);
    if (ret) {
        printk("Config failed");
        return 0;
    }

    ret = gpio_pin_interrupt_configure_dt(&pulse, GPIO_INT_EDGE_RISING);
    if (ret) {
        printk("Interrupt config failed");
        return 0;
    }

    gpio_init_callback(&pulse_cb_data, pulse_callback, BIT(pulse.pin));
    gpio_add_callback(pulse.port, &pulse_cb_data);

    printk("Pulse counter started\n");

    while (1) {
        printk("Count = %u\n", pulse_count);
        k_sleep(K_MSEC(1000));
    }
}
