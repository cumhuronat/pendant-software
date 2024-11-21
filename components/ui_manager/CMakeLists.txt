
idf_component_register(SRCS "src/ui_manager.c"
                       INCLUDE_DIRS "include"
                       REQUIRES "ui" "lvgl" "drivers" "esp_timer" "esp_psram" "ble_manager")
