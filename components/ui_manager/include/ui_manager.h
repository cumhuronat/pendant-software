#pragma once
#include "lvgl.h"
#include "ui.h"
#include "hx8357_cfg.h"
#include "hx8357.h"
#include "FT53xx.h"
#include "esp_timer.h"
#include "spi_init.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#ifdef __cplusplus
extern "C" {
#endif

void uimanager_init();

#ifdef __cplusplus
} /*extern "C"*/
#endif
