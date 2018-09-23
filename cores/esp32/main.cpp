#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "Arduino.h"

#if CONFIG_AUTOSTART_ARDUINO

#ifndef CONFIG_ARDUINO_STACK_SIZE
#define CONFIG_ARDUINO_STACK_SIZE 16384
#endif

#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

void loopTask(void *pvParameters)
{
    setup();
    for(;;) {
        loop();
    }
}

extern "C" void app_main()
{
    initArduino();
    xTaskCreatePinnedToCore(loopTask, "loopTask", CONFIG_ARDUINO_STACK_SIZE, NULL, 1, NULL, ARDUINO_RUNNING_CORE);
}

#endif
