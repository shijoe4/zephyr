#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

// Register the module and set its compile-time log level to Info
LOG_MODULE_REGISTER(main_app, LOG_LEVEL_INF);

int main(void)
{
    LOG_INF("Starting simple Zephyr logging example");

    int counter = 0;
    while (1) {
        // This debug message won't show unless the log level is set to Debug
        LOG_DBG("Current counter value: %d", counter);

        if (counter == 5) {
            LOG_WRN("Counter has reached 5!");
        }

        counter++;
        k_msleep(1000);
    }

    return 0;
}