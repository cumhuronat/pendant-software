idf_component_register(SRCS "spi_init.c" "spi_helper.c"
                    INCLUDE_DIRS . 
                    REQUIRES driver drivers freertos fatfs vfs)
