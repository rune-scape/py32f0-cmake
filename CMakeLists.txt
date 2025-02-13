set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)
cmake_minimum_required(VERSION 3.21)

# specify cross-compilers and tools
if (GCC_PATH)
    if (WIN32)  # add .exe for windows
        set(CMAKE_C_COMPILER ${GCC_PATH}/arm-none-eabi-gcc.exe)
        set(CMAKE_CXX_COMPILER ${GCC_PATH}/arm-none-eabi-g++.exe)
        set(CMAKE_ASM_COMPILER ${GCC_PATH}/arm-none-eabi-gcc.exe)
        set(CMAKE_AR ${GCC_PATH}/arm-none-eabi-ar.exe)
        set(CMAKE_OBJCOPY ${GCC_PATH}/arm-none-eabi-objcopy.exe)
        set(CMAKE_OBJDUMP ${GCC_PATH}/arm-none-eabi-objdump.exe)
        set(SIZE ${GCC_PATH}/arm-none-eabi-size.exe)
    else ()
        set(CMAKE_C_COMPILER ${GCC_PATH}/arm-none-eabi-gcc)
        set(CMAKE_CXX_COMPILER ${GCC_PATH}/arm-none-eabi-g++)
        set(CMAKE_ASM_COMPILER ${GCC_PATH}/arm-none-eabi-gcc)
        set(CMAKE_AR ${GCC_PATH}/arm-none-eabi-ar)
        set(CMAKE_OBJCOPY ${GCC_PATH}/arm-none-eabi-objcopy)
        set(CMAKE_OBJDUMP ${GCC_PATH}/arm-none-eabi-objdump)
        set(SIZE ${GCC_PATH}/arm-none-eabi-size)
    endif ()
else ()
    set(CMAKE_C_COMPILER arm-none-eabi-gcc)
    set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
    set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
    set(CMAKE_AR arm-none-eabi-ar)
    set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
    set(CMAKE_OBJDUMP arm-none-eabi-objdump)
    set(SIZE arm-none-eabi-size)
endif ()
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

project(app C CXX ASM)

# override options
set(ENABLE_HAL_DRIVER ON CACHE BOOL "Enable HAL driver")
set(ENABLE_LL_DRIVER OFF CACHE BOOL "Enable LL driver")
set(ENABLE_SYSTEM_XXX_C ON CACHE BOOL "Enable system_XXX.c")
set(ENABLE_STARTUP_FILE ON CACHE BOOL "Enable startup file")
set(ENABLE_LD_SCRIPT ON CACHE BOOL "Enable linker script")
set(ENABLE_DEBUG_DEFINE ON CACHE BOOL "Enable debug define")
set(MCU_MODEL "PY32F030x8" CACHE STRING "MCU model, like PY32F030x8")
message("MCU_MODEL: ${MCU_MODEL}")

add_subdirectory(Drivers)

set(AUTO_GENERATE_PYOCD_FLASH_SCRIPT ON)

add_executable(${PROJECT_NAME}.elf)

target_link_libraries(${PROJECT_NAME}.elf PRIVATE drivers)

file(GLOB SOURCES ${CMAKE_CURRENT_LIST_DIR}/App/*.c)
target_sources(${PROJECT_NAME}.elf
    PRIVATE
    ${SOURCES}
)
target_include_directories(${PROJECT_NAME}.elf
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/App
)


target_compile_options(${PROJECT_NAME}.elf
    PRIVATE
    -mcpu=cortex-m0plus
    -mthumb
    -mthumb-interwork
    -ffunction-sections
    -fdata-sections
    -fno-common
    -fmessage-length=0
    $<$<COMPILE_LANGUAGE:ASM>:-xassembler-with-cpp>
)

target_compile_options(${PROJECT_NAME}.elf PRIVATE 
    $<$<CONFIG:Release>:-Ofast>
    $<$<CONFIG:RelWithDebInfo>:-Ofast -g>
    $<$<CONFIG:MinSizeRel>:-Os>
    $<$<CONFIG:Debug>:-Og -g>
)

target_link_options(${PROJECT_NAME}.elf
    PRIVATE
    -mcpu=cortex-m0plus 
    -mthumb 
    -mthumb-interwork
    -Wl,-gc-sections
    -Wl,--print-memory-usage
    -Wl,-Map=${PROJECT_BINARY_DIR}/${PROJECT_NAME}.map
    -specs=nano.specs
    -specs=nosys.specs
    -static
    -lc
    -lm
)

# printf float
# seems need 20KB Flash, 0.8KB RAM!
# target_link_options(${PROJECT_NAME}.elf PRIVATE -u _printf_float)

set(HEX_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.hex)
set(BIN_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.bin)

add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -Oihex $<TARGET_FILE:${PROJECT_NAME}.elf> ${HEX_FILE}
        COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:${PROJECT_NAME}.elf> ${BIN_FILE}
        COMMENT "Building ${HEX_FILE}\nBuilding ${BIN_FILE}")

# auto generate pyocd.yaml
if(AUTO_GENERATE_PYOCD_FLASH_SCRIPT)
    set(FLASH_SH_CONENT "#!/usr/bin/env bash\npyocd load ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.elf --format elf -t ${MCU_MODEL} --pack ${CMAKE_CURRENT_LIST_DIR}/Misc/Puya.PY32F0xx_DFP.1.1.9.pack -f 1000000")
    file(WRITE "${PROJECT_BINARY_DIR}/pyocd_flash.sh" ${FLASH_SH_CONENT})
    file(CHMOD "${PROJECT_BINARY_DIR}/pyocd_flash.sh" PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ)
endif()
