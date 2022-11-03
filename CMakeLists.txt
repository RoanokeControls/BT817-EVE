# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

# Modify these to set the target GPU and display
set(FT8XX_TYPE BT817)
set(DISPLAY_RES WVGA)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(project_name "rp2040lcd")

# Pull in Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(${project_name} C CXX ASM)

set(PICO_CXX_ENABLE_EXCEPTIONS 1)

set(PICO_CXX_ENABLE_RTTI 1)

# Initialise the Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1
file(GLOB SOURCES_ ./**.cpp ./**.c)
file(GLOB HEADERS_ ./**.h)

include_directories(spi)
include_directories(eve)
include_directories(eve_ui)

include_directories(
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/assets
        ${CMAKE_CURRENT_LIST_DIR}/lib/eve/include
        ${CMAKE_CURRENT_LIST_DIR}/lib/eve_ui/include
)

add_executable(${project_name} ${SOURCES_} ${HEADERS_} 
        lcd.c
        # Source code for EVE library
        lib/eve/source/EVE_API.c
        lib/eve/source/EVE_HAL.c
        lib/eve/ports/eve_arch_rpi/EVE_MCU_RP2040.c
        # Source code for EVE UI library
        lib/eve_ui/source/eve_ui_main.c
        lib/eve_ui/source/eve_ui_images.c
        lib/eve_ui/source/eve_ui_ext_font.c
        lib/eve_ui/ports/eve_arch_rpi/eve_ui_mcu_rp2040.c
        lib/eve_ui/source/eve_ram_g.c
        # Source code
        assets/images_raw.s
        assets/images_array.c
)

# Tell EVE library to use RP2040 as base class
add_compile_definitions(PLATFORM_RP2040)

# Tell EVE library to use the correct EVE chip and display
add_compile_definitions(FT8XX_TYPE=${FT8XX_TYPE}
                        DISPLAY_RES=${DISPLAY_RES}
)

pico_set_program_name(${project_name} "${project_name}")
pico_set_program_version(${project_name} "0.1")
pico_enable_stdio_uart(${project_name} 1)
pico_enable_stdio_usb(${project_name} 1)

# Add the standard library to the build
target_link_libraries(${project_name} pico_stdlib hardware_adc)

# Add any user requested libraries
target_link_libraries(${project_name}
        hardware_spi
        hardware_timer
        hardware_dma
        hardware_pio
        pico_stdlib
        )

        pico_enable_stdio_usb(${PROJECT_NAME} 0)
        pico_enable_stdio_uart(${PROJECT_NAME} 1)