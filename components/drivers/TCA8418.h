#pragma once

#include <TCA8418_registers.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#define TCA8418_DEFAULT_ADDR 0x34

typedef enum
{
    KEY_SETZERO_Z = 0,
    KEY_GOTOZERO_XY,
    KEY_GOTOZERO_Z,
    KEY_START,
    KEY_PAUSE,
    KEY_STOP,
    KEY_SETTINGS,
    KEY_COOLANT,
    KEY_SETZERO_Y,
    KEY_SETZERO_X,
    KEY_A,
    KEY_B,
    KEY_AXIS_X,
    KEY_AXIS_Y,
    KEY_AXIS_Z,
    KEY_SPEED_1,
    KEY_SPEED_2,
    KEY_SPEED_3,
    KEY_COUNT // Total number of keys
} Key;

typedef enum
{
    RELEASED,
    PRESSED
} KeyEventType;

typedef struct
{
    Key key;
    KeyEventType type;
} KeyEvent;

#ifdef __cplusplus
class TCA8418
{
public:
    TCA8418();
    ~TCA8418();
    bool begin(uint8_t address = TCA8418_DEFAULT_ADDR);

    uint8_t flush();
    uint8_t getEvent();
    uint8_t readRegister(uint8_t reg);
    void writeRegister(uint8_t reg, uint8_t value);

private:
    uint8_t _address;
};

#endif

#ifdef __cplusplus
extern "C"
{
#endif
    void tca8418_init();
    extern QueueHandle_t key_event_queue;
#ifdef __cplusplus
} /*extern "C"*/
#endif
