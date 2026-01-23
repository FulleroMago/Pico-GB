add_library(morana INTERFACE)

set(LCD_ILI9225 ON)

target_sources(morana INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/input/buttons.c
    $<$<BOOL:${LCD_ILI9225}>:${CMAKE_CURRENT_LIST_DIR}/display/ili9225.c>
    $<$<BOOL:${LCD_ILI9341}>:${CMAKE_CURRENT_LIST_DIR}/display/ili9341.c>
)

target_include_directories(morana INTERFACE
    input
    display
    include
)

target_link_libraries(morana INTERFACE
        hardware_spi
        hardware_dma
        hardware_rtc
        pico_stdlib
)
