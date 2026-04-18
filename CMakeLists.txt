cmake_minimum_required(VERSION 3.13)

# initialize the SDK directly
# include(../pico-sdk/pico_sdk_init.cmake)
# set(PICO_SDK_FETCH_FROM_GIT on)
include(pico_sdk_import.cmake)

project(serial_capture)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of your project

add_executable(serial_capture
    src/main.cpp
)
# target_compile_definitions(serial_capture PRIVATE
#     WIFI_SSID=\"${WIFI_SSID}\"
#     WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
#     PICO_DEFAULT_UART_TX_PIN=16
#     PICO_DEFAULT_UART_RX_PIN=17
# )

    # PICO_CYW43_ARCH_DEBUG_ENABLED=1
# pico_enable_stdio_uart(serial_capture 1)
# pico_enable_stdio_usb(serial_capture 0)

target_include_directories(serial_capture PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src)

target_compile_options(serial_capture PRIVATE
    -Wall
    -Wextra
    -Werror
    $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
)

target_compile_options(serial_capture PRIVATE
    -Os
    -ffunction-sections
    -fdata-sections
)

target_link_options(serial_capture PRIVATE
    -Wl,--gc-sections
)

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(serial_capture
    pico_stdlib
    pico_cyw43_arch_lwip_threadsafe_background
)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(serial_capture)
