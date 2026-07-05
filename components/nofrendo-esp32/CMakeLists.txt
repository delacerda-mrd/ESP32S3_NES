idf_component_register(
    SRCS "osd.c" "video_audio.c" "spi_lcd.c" "touch_controller.c" "board_i2c.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES nofrendo esp_lcd driver esp_driver_i2s esp_driver_gpio esp_driver_spi es8311
    WHOLE_ARCHIVE
)
