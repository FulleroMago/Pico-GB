add_library(morana
    morana.c
    display.c
    sd_hw_config.c
    sd_card.c
    controls.c
)

target_include_directories(morana PUBLIC
    .
)

# # Add any user requested libraries
target_link_libraries(morana
    ili9341
    gfx
    FatFs_SPI

    # no-OS-FatFS-SD-SDIO-SPI-RPi-Pico
)

add_subdirectory(libs/display/ili9341)
add_subdirectory(libs/display/gfx)
add_subdirectory(libs/storage/FatFs_SPI build)
# add_subdirectory(libs/storage/SD build)