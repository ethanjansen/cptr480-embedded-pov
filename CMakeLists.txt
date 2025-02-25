# CMake file for FRDM-KL25-cptr480
cmake_minimum_required(VERSION 3.15.3)

# Optional: print out extra messages to see what is going on. Comment it to have less verbose messages
set(CMAKE_VERBOSE_MAKEFILE ON)

# Path to toolchain file. This one has to be before 'project()' below
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/arm-none-eabi-gcc.cmake)

# Setup project -- nice for grading
if(EXISTS ${CMAKE_SOURCE_DIR}/.cptr480)
        project(cptr480)
else()
        project(blinky)
endif()

# Setup output and linker file
set(EXECUTABLE ${PROJECT_NAME}.elf)
set(LINKER_FILE ${CMAKE_SOURCE_DIR}/device/MKL25Z4.ld)

enable_language(C CXX ASM)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optional: issue a message to be sure it uses the correct toolchain file.
message(STATUS "CMAKE_TOOLCHAIN_FILE is: ${CMAKE_TOOLCHAIN_FILE}")

# List of source files
set(SRC_FILES
        # main
        src/main.cpp

        # interrupts
        src/interrupts.cpp

        # helpers
        src/int2str.cpp

        # drivers
        src/DGPIO.cpp
        src/DDMA.cpp
        src/DLEDS.cpp
        src/DMOTION.cpp
        src/DPIT.cpp
#       src/DROTENC.cpp
        src/DSPI.cpp
        src/DTPM.cpp
        src/DUART.cpp

        # data
        #src/rgb_data.cpp
        src/rgb_data2.cpp

        # other classes
#       src/LED.cpp
#       src/Morse.cpp

        # Device files
	device/startup_MKL25Z4.s
	device/system_MKL25Z4.c
        )

# Build the executable based on the source files
add_executable(${EXECUTABLE} ${SRC_FILES})

# List of compiler defines, prefix with -D compiler option
target_compile_definitions(${EXECUTABLE} PRIVATE
        -DNDEBUG
        )

# List of include directories
target_include_directories(${EXECUTABLE} PRIVATE
        CMSIS
        device
        src/include
        )

# Compiler options
target_compile_options(${EXECUTABLE} PRIVATE
        -mcpu=cortex-m0plus
        -mthumb
        -fdata-sections
        -ffunction-sections
        -fno-exceptions
        -Wall
        -O0
        -g3
        )

# Linker options
target_link_options(${EXECUTABLE} PRIVATE
        -T${LINKER_FILE}
        -mcpu=cortex-m0plus
        -mthumb
        -specs=nano.specs
        -specs=nosys.specs
        -lc
        -lm
        -Wl,-Map=${PROJECT_NAME}.map,--cref
        -Wl,--gc-sections
        -Xlinker -print-memory-usage -Xlinker
        )

# Optional: Print executable size as part of the post build process
add_custom_command(TARGET ${EXECUTABLE}
        POST_BUILD
        COMMAND ${CMAKE_SIZE_UTIL} ${EXECUTABLE})

# Optional: Create hex, bin and S-Record files after the build
add_custom_command(TARGET ${EXECUTABLE}
        POST_BUILD
#        COMMAND ${CMAKE_OBJCOPY} -O srec --srec-len=64 ${EXECUTABLE} ${PROJECT_NAME}.s19
#        COMMAND ${CMAKE_OBJCOPY} -O ihex ${EXECUTABLE} ${PROJECT_NAME}.hex
        COMMAND ${CMAKE_OBJCOPY} -O binary ${EXECUTABLE} ${PROJECT_NAME}.bin)
