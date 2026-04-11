cmake_minimum_required(VERSION 3.16)

# Enable ASM and C languages
project(Kernalite LANGUAGES C ASM)

# Match your Makefile variable names
set(IMG_NAME "Image")
set(TARGET_ELF "${IMG_NAME}.elf")
set(TARGET "${IMG_NAME}") # This will be the raw binary

# --- Flags ---
set(KERNEL_CFLAGS 
    -Wall -Wextra 
    -ffreestanding 
    -nostdlib 
    -nostartfiles
)

set(KERNEL_ASFLAGS "")

# Header search path
include_directories(src/include)

# Manual Source Selection
set(SRCS_C  src/kernel/main.c)
set(SRCS_AS src/boot/boot.s)

# --- Executable (TARGET_ELF) ---
add_executable(${TARGET_ELF} ${SRCS_C} ${SRCS_AS})

# Set the output name to exactly match TARGET_ELF
set_target_properties(${TARGET_ELF} PROPERTIES OUTPUT_NAME ${IMG_NAME})
set_target_properties(${TARGET_ELF} PROPERTIES SUFFIX ".elf")

# Apply Language-Specific Flags
target_compile_options(${TARGET_ELF} PRIVATE 
    # Global C flags
    $<$<COMPILE_LANGUAGE:C>:${KERNEL_CFLAGS}>
    
    # Debug/Release specifically for C
    $<$<AND:$<COMPILE_LANGUAGE:C>,$<CONFIG:Debug>>:-g>
    $<$<AND:$<COMPILE_LANGUAGE:C>,$<CONFIG:Release>>:-O3>
    
    # Global ASM flags
    $<$<COMPILE_LANGUAGE:ASM>:${KERNEL_ASFLAGS}>
    
    # Debug for ASM
    $<$<AND:$<COMPILE_LANGUAGE:ASM>,$<CONFIG:Debug>>:-g>
)
# Linker Script and Options
target_link_options(${TARGET_ELF} PRIVATE 
    -T ${CMAKE_CURRENT_SOURCE_DIR}/scripts/linker.ld
    -Wl,--no-warn-rwx-segments
    -nostdlib
)

# --- Binary Image Generation ---

set(BINARY_NAME "${CMAKE_BINARY_DIR}/${TARGET}")

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    # In Release mode, we OBJCOPY then STRIP
    add_custom_command(
        OUTPUT "${BINARY_NAME}"
        COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${TARGET_ELF}> "${BINARY_NAME}"
        COMMAND ${CMAKE_STRIP} --strip-all $<TARGET_FILE:${TARGET_ELF}>
        DEPENDS ${TARGET_ELF}
        COMMENT "Generating binary image and stripping ELF: ${TARGET}"
        VERBATIM
    )
else()
    # In Debug mode, we just OBJCOPY
    add_custom_command(
        OUTPUT "${BINARY_NAME}"
        COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${TARGET_ELF}> "${BINARY_NAME}"
        DEPENDS ${TARGET_ELF}
        COMMENT "Generating binary image: ${TARGET}"
        VERBATIM
    )
endif()

# Rename target to 'generate_image' to avoid circular dependency with file 'Image'
add_custom_target(generate_image ALL DEPENDS "${BINARY_NAME}")
