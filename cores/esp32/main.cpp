#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_task_wdt.h"
#include "Arduino.h"

TaskHandle_t loopTaskHandle = NULL;

#if CONFIG_AUTOSTART_ARDUINO

#ifndef CONFIG_ARDUINO_STACK_SIZE
#define CONFIG_ARDUINO_STACK_SIZE 32768
#endif

#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

bool loopTaskWDTEnabled;

void loopTask(void *pvParameters)
{
    setup();
    for(;;) {
        if(loopTaskWDTEnabled){
            esp_task_wdt_reset();
        }
        loop();
    }
}

extern "C" void app_main()
{
    loopTaskWDTEnabled = false;
    initArduino();
    xTaskCreatePinnedToCore(loopTask, "loopTask", CONFIG_ARDUINO_STACK_SIZE, NULL, 1, &loopTaskHandle, ARDUINO_RUNNING_CORE);
}

#endif
