idf_component_register(
    SRCS "main.c"
    PRIV_REQUIRES nofrendo nofrendo-esp32 nvs_flash esp_partition esp_driver_gpio
)
