# Call Cmake from the 'build' subfolder with the command below.
# For using Make:
# cmake -DCMAKE_MAKE_PROGRAM=make.exe -DCMAKE_TOOLCHAIN_FILE="arm-none-eabi-gcc.cmake" -G "Unix Makefiles" ..
# followed by
# 'make' or 'cmake --build .' to build it
#
# For using Ninja:
# cmake -DCMAKE_MAKE_PROGRAM=ninja.exe -DCMAKE_TOOLCHAIN_FILE="arm-none-eabi-gcc.cmake" -G "Ninja" ..
# followed by
# 'ninja' or 'cmake --build .' to build it

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR ARM)

set(ARM_TOOLCHAIN_DIR "/opt/arm-gnu-toolchain-13.2.Rel1-x86_64-arm-none-eabi/bin")
set(BINUTILS_PATH ${ARM_TOOLCHAIN_DIR}) 

set(TOOLCHAIN_PREFIX ${ARM_TOOLCHAIN_DIR}/arm-none-eabi-)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_C_COMPILER "${TOOLCHAIN_PREFIX}gcc")
set(CMAKE_ASM_COMPILER ${CMAKE_C_COMPILER})
set(CMAKE_CXX_COMPILER "${TOOLCHAIN_PREFIX}g++")

set(CMAKE_OBJCOPY ${TOOLCHAIN_PREFIX}objcopy CACHE INTERNAL "objcopy tool")
set(CMAKE_SIZE_UTIL ${TOOLCHAIN_PREFIX}size CACHE INTERNAL "size tool")

set(CMAKE_FIND_ROOT_PATH ${BINUTILS_PATH})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
