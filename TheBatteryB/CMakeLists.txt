# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.3.0)
set(toolchainVersion 15_2_Rel1)
set(picotoolVersion 2.3.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.3.0)
set(toolchainVersion 15_2_Rel1)
set(picotoolVersion 2.3.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()

set(PICO_BOARD pico_w CACHE STRING "Board type")

include(pico_sdk_import.cmake)

project(TheBatteryB C CXX ASM)

pico_sdk_init()

add_executable(TheBatteryB
        TheBatteryB.c
)

pico_set_program_name(TheBatteryB "TheBatteryB")
pico_set_program_version(TheBatteryB "0.1")

# Nếu vẫn muốn giữ file PIO hiện tại trong tree thì cứ để dòng này.
pico_generate_pio_header(TheBatteryB ${CMAKE_CURRENT_LIST_DIR}/blink.pio)

# Không dùng stdio USB vì nó chiếm USB device.
pico_enable_stdio_uart(TheBatteryB 0)
pico_enable_stdio_usb(TheBatteryB 0)

target_link_libraries(TheBatteryB
        pico_stdlib
        pico_unique_id
        tinyusb_device
        tinyusb_board
        hardware_spi
        hardware_i2c
        hardware_pio
        pico_cyw43_arch_none
)

target_include_directories(TheBatteryB PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/LIB
)

pico_add_extra_outputs(TheBatteryB)