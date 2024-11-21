
#include "ui_manager.h"
#include "driver/gpio.h"
#include "ble_manager.h"
#include "TCA8418.h"
#include "rotary_encoder.h"
#define TAG "main"


void app_main()
{
    gpio_set_direction(GPIO_NUM_7, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_7, 1);

    ble_manager_init();
    tca8418_init();
    rotary_encoder_init();
    uimanager_init();
}
