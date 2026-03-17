add_library(morana
    src/morana.c
    src/display/ili9341.c
    src/display/display.c
    src/display/display_bmp.c
    src/display/display_txt.c
    src/display/display_shape.c
    src/sd_card.c
    src/controls.c
    src/GUI/menu.c
    src/GUI/menu_item.c
)

target_include_directories(morana PUBLIC
    ./src
)

# # Add any user requested libraries
target_link_libraries(morana
    FatFs_SPI
)

add_subdirectory(libs/storage/FatFs_SPI build)