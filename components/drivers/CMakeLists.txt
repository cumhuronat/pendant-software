idf_component_register(SRCS "hx8357.c" "FT53xx.c" "TCA8418.cpp" "rotary_encoder.c"
                    INCLUDE_DIRS .
                    REQUIRES driver spi_helper lvgl esp_rom i2c_helper esp_driver_pcnt)
