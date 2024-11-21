#include "ble_manager.h"

#define TAG "BLEMAN"

static BLEManager bleManager;
QueueHandle_t statusUpdateQueue;
QueueHandle_t wPosUpdateQueue;
QueueHandle_t fsUpdateQueue;
QueueHandle_t pinUpdateQueue;
QueueHandle_t accessoryUpdateQueue;
QueueHandle_t queueItemsUpdateQueue;
QueueHandle_t wcoUpdateQueue;
QueueHandle_t ovUpdateQueue;

static NimBLEClient *pClient;
NimBLEUUID serviceUuid(SERVICE_UUID);
NimBLERemoteCharacteristic *writeCharacteristic;

std::unordered_map<std::string, std::pair<QueueHandle_t, size_t>> characteristicMap;

BLEManager::BLEManager()
{
}

struct CharacteristicInfo
{
    const char *uuid;
    BLECharacteristic *characteristic;
};

void BLEManager::bleManagerTask(void *pvParameters)
{
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void BLEManager::bleConnectionTask(void *pvParameters)
{
    while (1)
    {
        if (!pClient->isConnected())
        {
            if (BLEManager::scanAndConnect())
            {
                ESP_LOGI(TAG, "Connected to dongle");
            }
            else
            {
                ESP_LOGI(TAG, "Failed to connect to dongle");
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void BLEManager::init()
{
    NimBLEDevice::init("");
    pClient = NimBLEDevice::createClient();

    // Create a queue for each characteristic
    statusUpdateQueue = xQueueCreate(10, sizeof(StatusUpdate));
    wPosUpdateQueue = xQueueCreate(10, sizeof(WPosUpdate));
    fsUpdateQueue = xQueueCreate(10, sizeof(FSUpdate));
    pinUpdateQueue = xQueueCreate(10, sizeof(PinUpdate));
    accessoryUpdateQueue = xQueueCreate(10, sizeof(AccessoryUpdate));
    queueItemsUpdateQueue = xQueueCreate(10, sizeof(QueueItemsUpdate));
    wcoUpdateQueue = xQueueCreate(10, sizeof(WCOUpdate));
    ovUpdateQueue = xQueueCreate(10, sizeof(OvUpdate));

    // Initialize the characteristic map
    characteristicMap = {
        {CHARACTERISTIC_STATUS_UUID, {statusUpdateQueue, sizeof(StatusUpdate)}},
        {CHARACTERISTIC_WPOS_UUID, {wPosUpdateQueue, sizeof(WPosUpdate)}},
        {CHARACTERISTIC_FS_UUID, {fsUpdateQueue, sizeof(FSUpdate)}},
        {CHARACTERISTIC_PIN_UUID, {pinUpdateQueue, sizeof(PinUpdate)}},
        {CHARACTERISTIC_ACCESSORY_UUID, {accessoryUpdateQueue, sizeof(AccessoryUpdate)}},
        {CHARACTERISTIC_QUEUE_UUID, {queueItemsUpdateQueue, sizeof(QueueItemsUpdate)}},
        {CHARACTERISTIC_WCO_UUID, {wcoUpdateQueue, sizeof(WCOUpdate)}},
        {CHARACTERISTIC_OV_UUID, {ovUpdateQueue, sizeof(OvUpdate)}},
    };

    xTaskCreate(BLEManager::bleManagerTask, "ble_manager_task", 4096, NULL, 1, NULL);
    xTaskCreate(BLEManager::bleConnectionTask, "ble_connection_task", 4096, NULL, 1, NULL);
}

static void notifyCallback(
    BLERemoteCharacteristic *pBLERemoteCharacteristic,
    const uint8_t *pData,
    size_t length,
    bool isNotify)
{
    const std::string uuidStr = pBLERemoteCharacteristic->getUUID().toString();

    auto it = characteristicMap.find(uuidStr);
    if (it != characteristicMap.end())
    {
        QueueHandle_t queue = it->second.first;
        xQueueSendToBack(queue, pData, 0);
    }
}

bool BLEManager::scanAndConnect()
{
    NimBLEScan *pScan = NimBLEDevice::getScan();
    NimBLEScanResults results = pScan->getResults(3 * 1000);

    for (int i = 0; i < results.getCount(); i++)
    {
        NimBLEAdvertisedDevice device = results.getDevice(i);

        if (device.isAdvertisingService(serviceUuid))
        {
            if (pClient->connect(&device))
            {
                NimBLERemoteService *pService = pClient->getService(serviceUuid);

                if (pService != nullptr)
                {
                    for (const auto &entry : characteristicMap)
                    {
                        NimBLERemoteCharacteristic *pCharacteristic = pService->getCharacteristic(entry.first);
                        if (pCharacteristic != nullptr)
                        {
                            pCharacteristic->subscribe(true, notifyCallback);
                            NimBLEAttValue value = pCharacteristic->readValue();
                            notifyCallback(pCharacteristic, value.data(), value.length(), false);
                        }
                    }

                    writeCharacteristic = pService->getCharacteristic(CHARACTERISTIC_WRITE_UUID);
                }
                return true;
            }
        }
    }
    return false;
}

extern "C" void ble_manager_init()
{
    bleManager.init();
}

extern "C" void ble_write_command(char *command)
{
    if (writeCharacteristic != nullptr)
    {
        Command cmd = {0};
        strncpy(cmd.command, command, strlen(command));
        writeCharacteristic->writeValue(cmd);
    }
}