cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

include(pico_sdk_import.cmake)

project(Conversor_DAC C CXX ASM)

pico_sdk_init()


add_executable(Conversor_DAC 
    Conversor_DAC.c
    ssd1306/ssd1306.c  
)

pico_set_program_name(Conversor_DAC "Conversor_DAC")
pico_set_program_version(Conversor_DAC "0.1")

pico_enable_stdio_uart(Conversor_DAC 0)
pico_enable_stdio_usb(Conversor_DAC 0)

# Include SSD1306 headers
target_include_directories(Conversor_DAC PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/ssd1306  
)

# Link libraries
target_link_libraries(Conversor_DAC
    pico_stdlib
    hardware_adc
    hardware_pwm
    hardware_gpio
    hardware_i2c
 
)

pico_add_extra_outputs(Conversor_DAC)