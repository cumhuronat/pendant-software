#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

extern QueueHandle_t rotary_queue;
extern void rotary_encoder_init();