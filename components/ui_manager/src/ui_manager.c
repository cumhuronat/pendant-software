#include "ui_manager.h"
#include "ble_manager.h"
#include "TCA8418.h"
#include "rotary_encoder.h"

#define TAG "UIMAN"



static lv_display_t *display;
static SemaphoreHandle_t xGuiSemaphore;
static WPosUpdate wPosUpdate;
static FSUpdate fsUpdate;
static PinUpdate pinUpdate;
static AccessoryUpdate accessoryUpdate;
static StatusUpdate statusUpdate;
static OvUpdate ovUpdate;
static WCOUpdate wcoUpdate;

typedef enum
{
    X,
    Y,
    Z,
    OFF
} Axis;

static Axis currentAxis = OFF;

static void lv_tick_task(void *param)
{
    (void)param;
    lv_tick_inc(portTICK_PERIOD_MS);
}

static void lvgl_task(void *param)
{
    (void)param;

    lv_init();
    vTaskDelay(pdMS_TO_TICKS(100));
    spi_full_init(SPI_INIT_BOTH);
    ft53xx_init(FT53XX_DEFAULT_ADDR);

    lv_color_t *buf1 = (lv_color_t *)heap_caps_malloc(HX8357_BUF_SIZE, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    lv_color_t *buf2 = (lv_color_t *)heap_caps_malloc(HX8357_BUF_SIZE, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);

    display = lv_display_create(HX8357_TFTHEIGHT, HX8357_TFTWIDTH);
    lv_display_set_flush_cb(display, hx8357_flush);
    lv_display_set_buffers(display, buf1, buf2, HX8357_BUF_SIZE, LV_DISPLAY_RENDER_MODE_PARTIAL);

    const esp_timer_create_args_t periodic_timer_args = {.callback = &lv_tick_task, .name = "periodic_gui"};
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, portTICK_PERIOD_MS * 1000));

    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, ft53xx_read);

    ui_init();

    while (1)
    {
        if (xSemaphoreTake(xGuiSemaphore, portMAX_DELAY) == pdTRUE)
        {
            uint32_t time_till_next = lv_timer_handler();
            xSemaphoreGive(xGuiSemaphore);
            vTaskDelay(pdMS_TO_TICKS(time_till_next));
        }
    }
}

const char *floatToConstChar(float value, int precision, const char *suffix)
{
    static char buffer[64];
    char format[16];
    snprintf(format, sizeof(format), "%%.%df%%s", precision);
    snprintf(buffer, sizeof(buffer), format, value, suffix);
    return buffer;
}

static void listen_wpos_task(void *param)
{
    while (1)
    {
        if (xQueueReceive(wPosUpdateQueue, &wPosUpdate, portMAX_DELAY))
        {
            if (xSemaphoreTake(xGuiSemaphore, portMAX_DELAY) == pdTRUE)
            {
                _ui_label_set_property(ui_xvalue, _UI_LABEL_PROPERTY_TEXT, floatToConstChar(wPosUpdate.x, 4, ""));
                _ui_label_set_property(ui_yvalue, _UI_LABEL_PROPERTY_TEXT, floatToConstChar(wPosUpdate.y, 4, ""));
                _ui_label_set_property(ui_zvalue, _UI_LABEL_PROPERTY_TEXT, floatToConstChar(wPosUpdate.z, 4, ""));

                _ui_label_set_property(ui_xmachinevalue, _UI_LABEL_PROPERTY_TEXT, floatToConstChar(wPosUpdate.x + wcoUpdate.x, 4, ""));
                _ui_label_set_property(ui_ymachinevalue, _UI_LABEL_PROPERTY_TEXT, floatToConstChar(wPosUpdate.y + wcoUpdate.y, 4, ""));
                _ui_label_set_property(ui_zmachinevalue, _UI_LABEL_PROPERTY_TEXT, floatToConstChar(wPosUpdate.z + wcoUpdate.z, 4, ""));
                xSemaphoreGive(xGuiSemaphore);
            }
        }
    }
}
static void listen_wco_task(void *param)
{
    while (1)
    {
        if (xQueueReceive(wcoUpdateQueue, &wcoUpdate, portMAX_DELAY))
        {
            if (xSemaphoreTake(xGuiSemaphore, portMAX_DELAY) == pdTRUE)
            {
                _ui_label_set_property(ui_xmachinevalue, _UI_LABEL_PROPERTY_TEXT, floatToConstChar(wPosUpdate.x + wcoUpdate.x, 4, ""));
                _ui_label_set_property(ui_ymachinevalue, _UI_LABEL_PROPERTY_TEXT, floatToConstChar(wPosUpdate.y + wcoUpdate.y, 4, ""));
                _ui_label_set_property(ui_zmachinevalue, _UI_LABEL_PROPERTY_TEXT, floatToConstChar(wPosUpdate.z + wcoUpdate.z, 4, ""));
                xSemaphoreGive(xGuiSemaphore);
            }
        }
    }
}

static void listen_fs_task(void *param)
{
    while (1)
    {
        if (xQueueReceive(fsUpdateQueue, &fsUpdate, portMAX_DELAY))
        {
            if (xSemaphoreTake(xGuiSemaphore, portMAX_DELAY) == pdTRUE)
            {
                _ui_state_modify(ui_spindlerpmpanel, LV_STATE_DISABLED, fsUpdate.spindleSpeed > 0 ? _UI_MODIFY_STATE_REMOVE : _UI_MODIFY_STATE_ADD);
                _ui_label_set_property(ui_spindlerpmvalue, _UI_LABEL_PROPERTY_TEXT, floatToConstChar(fsUpdate.spindleSpeed, 0, " RPM"));

                _ui_state_modify(ui_feedratepanel, LV_STATE_DISABLED, fsUpdate.feedRate > 0 ? _UI_MODIFY_STATE_REMOVE : _UI_MODIFY_STATE_ADD);
                _ui_label_set_property(ui_feedratevalue, _UI_LABEL_PROPERTY_TEXT, floatToConstChar(fsUpdate.feedRate, 0, " IPM"));

                xSemaphoreGive(xGuiSemaphore);
            }
        }
    }
}
static void listen_pin_task(void *param)
{
    while (1)
    {
        if (xQueueReceive(pinUpdateQueue, &pinUpdate, portMAX_DELAY))
        {
            if (xSemaphoreTake(xGuiSemaphore, portMAX_DELAY) == pdTRUE)
            {
                _ui_flag_modify(ui_xstatus, LV_OBJ_FLAG_HIDDEN, pinUpdate.x ? _UI_MODIFY_FLAG_REMOVE : _UI_MODIFY_FLAG_ADD);
                _ui_flag_modify(ui_ystatus, LV_OBJ_FLAG_HIDDEN, pinUpdate.y1 ? _UI_MODIFY_FLAG_REMOVE : _UI_MODIFY_FLAG_ADD);
                _ui_flag_modify(ui_zstatus, LV_OBJ_FLAG_HIDDEN, pinUpdate.z ? _UI_MODIFY_FLAG_REMOVE : _UI_MODIFY_FLAG_ADD);

                xSemaphoreGive(xGuiSemaphore);
            }
        }
    }
}
static void listen_accessory_task(void *param)
{
    while (1)
    {
        if (xQueueReceive(accessoryUpdateQueue, &accessoryUpdate, portMAX_DELAY))
        {
            if (xSemaphoreTake(xGuiSemaphore, portMAX_DELAY) == pdTRUE)
            {
                _ui_flag_modify(ui_coolantstatus, LV_OBJ_FLAG_HIDDEN, accessoryUpdate.coolant ? _UI_MODIFY_FLAG_REMOVE : _UI_MODIFY_FLAG_ADD);

                xSemaphoreGive(xGuiSemaphore);
            }
        }
    }
}
static void listen_status_task(void *param)
{
    while (1)
    {
        if (xQueueReceive(statusUpdateQueue, &statusUpdate, portMAX_DELAY))
        {
            if (xSemaphoreTake(xGuiSemaphore, portMAX_DELAY) == pdTRUE)
            {
                _ui_state_modify(ui_machinestatus, LV_STATE_CHECKED, strcmp(statusUpdate.status, "Idle") ? _UI_MODIFY_STATE_REMOVE : _UI_MODIFY_STATE_ADD);
                _ui_state_modify(ui_machinestatus, LV_STATE_DISABLED, strcmp(statusUpdate.status, "Alarm") ? _UI_MODIFY_STATE_REMOVE : _UI_MODIFY_STATE_ADD);
                _ui_label_set_property(ui_machinestatuslabel, _UI_LABEL_PROPERTY_TEXT, statusUpdate.status);
                xSemaphoreGive(xGuiSemaphore);
            }
        }
    }
}
static void listen_overrides_task(void *param)
{
    while (1)
    {
        if (xQueueReceive(ovUpdateQueue, &ovUpdate, portMAX_DELAY))
        {
            if (xSemaphoreTake(xGuiSemaphore, portMAX_DELAY) == pdTRUE)
            {
                lv_slider_set_value(ui_feedoverrideslider, ovUpdate.feed, LV_ANIM_OFF);
                lv_slider_set_value(ui_rapidoverrideslider, ovUpdate.rapid, LV_ANIM_OFF);
                lv_slider_set_value(ui_spindleoverrideslider, ovUpdate.spindle, LV_ANIM_OFF);

                lv_obj_send_event(ui_feedoverrideslider, LV_EVENT_VALUE_CHANGED, NULL);
                lv_obj_send_event(ui_rapidoverrideslider, LV_EVENT_VALUE_CHANGED, NULL);
                lv_obj_send_event(ui_spindleoverrideslider, LV_EVENT_VALUE_CHANGED, NULL);
                xSemaphoreGive(xGuiSemaphore);
            }
        }
    }
}



static void listen_key_task(void *param)
{
    KeyEvent key_event;
    while (1)
    {
        if (xQueueReceive(key_event_queue, &key_event, portMAX_DELAY))
        {
            ESP_LOGI(TAG, "Key event: %d %d", key_event.key, key_event.type);
            if (key_event.type == PRESSED)
            {
                switch (key_event.key)
                {
                case KEY_GOTOZERO_XY:
                    ble_write_command("$J=G21 G90 X0 Y0  F2540.0");
                    break;

                case KEY_GOTOZERO_Z:
                    ble_write_command("$J=G21 G90 Z0  F2540.0");
                    break;
                case KEY_SETZERO_X:
                    ble_write_command("G20 G10 P1 L20 X 0.0000");
                    break;
                case KEY_SETZERO_Y:
                    ble_write_command("G20 G10 P1 L20 Y 0.0000");
                    break;
                case KEY_SETZERO_Z:
                    ble_write_command("G20 G10 P1 L20 Z 0.0000");
                    break;
                case KEY_AXIS_X:
                    currentAxis = X;
                    break;
                case KEY_AXIS_Y:
                    currentAxis = Y;
                    break;
                case KEY_AXIS_Z:
                    currentAxis = Z;
                    break;
                default:
                    break;
                }
            }
            else if (key_event.key == KEY_AXIS_X || key_event.key == KEY_AXIS_Y || key_event.key == KEY_AXIS_Z)
            {
                currentAxis = OFF;
            }
        }
    }
}

static void listen_rotary_task(void *param)
{
    int direction;
    while (1)
    {
        if (xQueueReceive(rotary_queue, &direction, portMAX_DELAY))
        {
            if (currentAxis == OFF)
            {
                continue;
            }
            float movement = 0.1000 * (direction >0 ? 1 : -1);
            char axis = currentAxis == X ? 'X' : currentAxis == Y ? 'Y' : 'Z';
            ESP_LOGI(TAG, "Rotary event: %d", direction);   
            char command[32];

            snprintf(command, sizeof(command), "$J=G20 G91 %c %.4f F 100", axis, movement);
            ble_write_command(command);
        }
    }
}

void uimanager_init(void)
{
    xGuiSemaphore = xSemaphoreCreateMutex();
    xTaskCreatePinnedToCore(lvgl_task, "lvgl_task", 8192, NULL, 5, NULL, 1);
    xTaskCreatePinnedToCore(listen_wpos_task, "listen_wpos_task", 2048, NULL, 5, NULL, 1);
    xTaskCreatePinnedToCore(listen_fs_task, "listen_fs_task", 2048, NULL, 5, NULL, 1);
    xTaskCreatePinnedToCore(listen_pin_task, "listen_pin_task", 2048, NULL, 5, NULL, 1);
    xTaskCreatePinnedToCore(listen_accessory_task, "listen_accessory_task", 2048, NULL, 5, NULL, 1);
    xTaskCreatePinnedToCore(listen_status_task, "listen_status_task", 2048, NULL, 5, NULL, 1);
    xTaskCreatePinnedToCore(listen_overrides_task, "listen_overrides_task", 2048, NULL, 5, NULL, 1);
    xTaskCreatePinnedToCore(listen_wco_task, "listen_wco_task", 2048, NULL, 5, NULL, 1);
    xTaskCreatePinnedToCore(listen_key_task, "listen_key_task", 4096, NULL, 5, NULL, 1);
    xTaskCreatePinnedToCore(listen_rotary_task, "listen_rotary_task", 4096, NULL, 5, NULL, 1);
}