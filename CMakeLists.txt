cmake_minimum_required(VERSION 3.28.2)

include(../sdk/config.cmake)
project(sieelf_dcalc C)

include_directories("../libsie/src/include")
link_libraries(crt crt_helper gcc c)
file(GLOB_RECURSE SOURCES "src/*.c")

add_executable(DCalc_ELKA ${SOURCES})
target_sdk_setup(DCalc_ELKA ELKA)
target_link_directories(DCalc_ELKA PUBLIC "../libsie/build/lib/ELKA")
target_compile_options(DCalc_ELKA BEFORE PUBLIC -std=c11 -Wall -Werror -DSWILIB_MODERN)
