#pragma once

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_STATUS_UUID "00000001-0000-1000-8000-00805f9b34fb"
#define CHARACTERISTIC_WPOS_UUID "00000002-0000-1000-8000-00805f9b34fb"
#define CHARACTERISTIC_FS_UUID "00000003-0000-1000-8000-00805f9b34fb"
#define CHARACTERISTIC_PIN_UUID "00000004-0000-1000-8000-00805f9b34fb"
#define CHARACTERISTIC_QUEUE_UUID "00000005-0000-1000-8000-00805f9b34fb"
#define CHARACTERISTIC_WCO_UUID "00000006-0000-1000-8000-00805f9b34fb"
#define CHARACTERISTIC_OV_UUID "00000007-0000-1000-8000-00805f9b34fb"
#define CHARACTERISTIC_WRITE_UUID "00000008-0000-1000-8000-00805f9b34fb"
#define CHARACTERISTIC_ACCESSORY_UUID "00000009-0000-1000-8000-00805f9b34fb"

typedef struct
{
    char status[10];
} StatusUpdate;

typedef struct
{
    float x, y, z;
} WPosUpdate;

typedef struct
{
    float feedRate, spindleSpeed;
} FSUpdate;

typedef struct
{
    int p, x, y1, y2, z, h;
} PinUpdate;

typedef struct
{
    int coolant;
} AccessoryUpdate;

typedef struct
{
    int queueItems;
} QueueItemsUpdate;

typedef struct
{
    char command[128];
} Command;

typedef struct
{
    float x, y, z;
} WCOUpdate;

typedef struct
{
    int feed, rapid, spindle;
} OvUpdate;

#ifdef __cplusplus

#include <NimBLEDevice.h>
#include <cstdio>
#include <map>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

class BLEManager
{
public:
    BLEManager();
    void init();

private:
    static bool scanAndConnect();
    static void bleConnectionTask(void *pvParameters);
    static void bleManagerTask(void *pvParameters);
};

#endif

#ifdef __cplusplus
extern "C"
{
#endif
    extern QueueHandle_t statusUpdateQueue;
    extern QueueHandle_t wPosUpdateQueue;
    extern QueueHandle_t fsUpdateQueue;
    extern QueueHandle_t pinUpdateQueue;
    extern QueueHandle_t accessoryUpdateQueue;
    extern QueueHandle_t queueItemsUpdateQueue;
    extern QueueHandle_t wcoUpdateQueue;
    extern QueueHandle_t ovUpdateQueue;

    void ble_write_command(char *command);
    void ble_manager_init();
#ifdef __cplusplus
} /*extern "C"*/
#endif
