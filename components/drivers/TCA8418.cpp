#include "TCA8418.h"
#include "driver/gpio.h"
#include "i2c_helper.h"
#include "i2c_helper_cfg.h"
#include "esp_log.h"
#include <math.h>

static TCA8418 tca8418;
static SemaphoreHandle_t button_semaphore;
QueueHandle_t key_event_queue;

TCA8418::TCA8418(void) {}
TCA8418::~TCA8418(void) {}

bool TCA8418::begin(uint8_t address)
{
    _address = address;

    writeRegister(TCA8418_REG_GPI_EM_1, 0xFF);
    writeRegister(TCA8418_REG_GPI_EM_2, 0xFF);
    writeRegister(TCA8418_REG_GPI_EM_3, 0xFF);

    writeRegister(TCA8418_REG_CFG, TCA8418_REG_CFG_KE_IEN);

    return true;
}

uint8_t TCA8418::flush()
{
    uint8_t count = 0;
    while (getEvent() != 0)
        count++;
    writeRegister(TCA8418_REG_INT_STAT, 3);
    return count;
}

uint8_t TCA8418::getEvent()
{
    uint8_t event = readRegister(TCA8418_REG_KEY_EVENT_A);
    return event;
}

uint8_t TCA8418::readRegister(uint8_t reg)
{
    uint8_t buffer[1] = {0};
    esp_err_t err = i2c_read(I2C_PORT, _address, reg, buffer, 1);
    if (err != ESP_OK)
    {
        ESP_LOGE("TCA", "I2C read error: %d", err);
        return 0;
    }
    return buffer[0];
}

void TCA8418::writeRegister(uint8_t reg, uint8_t value)
{
    uint8_t buffer[1] = {value};
    esp_err_t err = i2c_write(I2C_PORT, _address, reg, buffer, 1);
    if (err != ESP_OK)
    {
        ESP_LOGE("TCA", "I2C write error: %d", err);
    }
}

void IRAM_ATTR tca8418_irq(void *arg)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(button_semaphore, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void tca8418_task(void *pvParameters)
{
    uint8_t keyCode;
    while (1)
    {
        if (xSemaphoreTake(button_semaphore, pdMS_TO_TICKS(3000)) == pdTRUE)
        {
            while (1)
            {
                while (1)
                {
                    bool pressed = false;
                    keyCode = tca8418.getEvent();
                    if (keyCode == 0)
                        break;
                    if (keyCode & 0x80)
                        pressed = true;
                    keyCode &= 0x7F;
                    keyCode -= 97;

                    KeyEvent keyEvent;
                    keyEvent.key = static_cast<Key>(keyCode);
                    keyEvent.type = pressed ? PRESSED : RELEASED;

                    if (xQueueSend(key_event_queue, &keyEvent, 0) != pdTRUE)
                    {
                        ESP_LOGE("TCA", "Failed to send key event to queue");
                    }
                }

                tca8418.writeRegister(TCA8418_REG_INT_STAT, 1);
                int intStat = tca8418.readRegister(TCA8418_REG_INT_STAT);
                if (intStat == 0)
                    break;
            }
        }
    }
}

extern "C" void tca8418_init()
{
    if (false == i2c_get_init_status())
        i2c_init(I2C_PORT);
    tca8418.begin();
    tca8418.flush();
    button_semaphore = xSemaphoreCreateBinary();
    key_event_queue = xQueueCreate(10, sizeof(KeyEvent));
    xTaskCreate(tca8418_task, "tca8418_task", 4096, NULL, 10, NULL);

    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << 1);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(GPIO_NUM_1, tca8418_irq, NULL);
}