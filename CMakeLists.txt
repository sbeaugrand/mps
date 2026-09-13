# ---------------------------------------------------------------------------- #
## \file CMakeLists.txt
## \author Sebastien Beaugrand
## \sa http://beaugrand.chez.com/
## \copyright CeCILL 2.1 Free Software license
# ---------------------------------------------------------------------------- #
cmake_minimum_required(VERSION 3.18)
include(xc.cmake)
project(mps)
include(include.cmake)
message(CMAKE_HOST_SYSTEM_PROCESSOR=${CMAKE_HOST_SYSTEM_PROCESSOR})
add_compile_options(
    "-DARMPLAT"
)

set(RPC_ID
    json-rpc-cxx/include
    json-rpc-cxx/vendor
    json-rpc-cxx/examples
)
set(MRAA_ID
    mraa/api
    mraa/api/mraa
    mraa/include
)
set(LCD_ID
    ${MRAA_ID}
    upm/src/lcd
)
set(MRAA
    mraa/src/i2c/i2c.c
    mraa/src/mraa.c
    mraa/src/gpio/gpio.c
    mraa/src/gpio/gpio_chardev.c
    mraa/src/pwm/pwm.c
    mraa/src/spi/spi.c
    mraa/src/uart/uart.c
    mraa/src/aio/aio.c
    mraa/src/arm/arm.c
    mraa/src/grovepi/grovepi.c
    mraa/src/arm/raspberry_pi.c
    mraa/src/arm/beaglebone.c
    mraa/src/arm/phyboard.c
    mraa/src/arm/banana.c
    mraa/src/arm/radxa_cm3.c
    mraa/src/arm/radxa_e25.c
    mraa/src/arm/radxa_rock_3a.c
    mraa/src/arm/radxa_rock_3b.c
    mraa/src/arm/radxa_rock_3c.c
    mraa/src/arm/radxa_rock_5a.c
    mraa/src/arm/radxa_rock_5b.c
    mraa/src/arm/radxa_cm5_io.c
    mraa/src/arm/rockpi4.c
    mraa/src/arm/rockpis.c
    mraa/src/arm/orangepizero.c
    mraa/src/arm/nanopineo.c
    mraa/src/arm/de_nano_soc.c
    mraa/src/arm/adlink_ipi.c
    mraa/src/arm/siemens/iot2050.c
    mraa/src/arm/siemens/platform.c
    mraa/src/arm/siemens/platform_iot2050.c
    mraa/src/arm/orange_pi_prime.c
    mraa/src/arm/96boards.c
)
set(LCD
    ${MRAA}
    upm/src/lcd/ssd1306.cxx
    upm/src/lcd/lcd.cxx
)

add_executable(mpserver
    mpserver.cpp
    List.cpp
    Player.cpp
    Server.cpp
    path.cpp
)
target_include_directories(mpserver PRIVATE
    ${RPC_ID}
)
target_link_libraries(mpserver PRIVATE
    mpdclient
)

add_executable(mpclient
    ${LCD}
    mpclient.cpp
    Input.cpp
    Output.cpp
    Client.cpp
)
target_include_directories(mpclient PRIVATE
    ${RPC_ID}
    ${LCD_ID}
)
target_link_libraries(mpclient PRIVATE
    lirc_client
)

add_executable(mpssaver
    ${LCD}
    mpssaver.cpp
    Input.cpp
    Output.cpp
)
target_include_directories(mpssaver PRIVATE
    ${LCD_ID}
)
target_link_libraries(mpssaver PRIVATE
    lirc_client
)

add_executable(oledmesg
    ${LCD}
    oledmesg.cpp
)
target_include_directories(oledmesg PRIVATE
    ${LCD_ID}
)

install(TARGETS mpserver mpclient mpssaver oledmesg)

if(XC
        OR ${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL aarch64
        OR ${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL armv7l)
    target_link_libraries(mpclient PRIVATE
        lirc_client
    )
    target_link_libraries(mpssaver PRIVATE
        lirc_client
    )
endif()

add_custom_command(
    OUTPUT mpserver.1
    COMMAND help2man -N -o mpserver.1 -n 'music player server' ./mpserver || cp ../mpserver.1 .
    DEPENDS mpserver
)
add_custom_command(
    OUTPUT mpclient.1
    COMMAND help2man -N -o mpclient.1 -n 'music player client' ./mpclient || cp ../mpclient.1 .
    DEPENDS mpclient
)
add_custom_command(
    OUTPUT mpssaver.1
    COMMAND help2man -N -o mpssaver.1 -n 'music player screensaver' ./mpssaver || cp ../mpssaver.1 .
    DEPENDS mpssaver
)
add_custom_command(
    OUTPUT oledmesg.1
    COMMAND help2man -N -o oledmesg.1 -n 'display a message in oled module' ./oledmesg || cp ../oledmesg.1 .
    DEPENDS oledmesg
)

add_custom_target(mpserver-manpage ALL DEPENDS mpserver.1)
add_custom_target(mpclient-manpage ALL DEPENDS mpclient.1)
add_custom_target(mpssaver-manpage ALL DEPENDS mpssaver.1)
add_custom_target(oledmesg-manpage ALL DEPENDS oledmesg.1)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/mpserver.1
    ${CMAKE_CURRENT_BINARY_DIR}/mpclient.1
    ${CMAKE_CURRENT_BINARY_DIR}/mpssaver.1
    ${CMAKE_CURRENT_BINARY_DIR}/oledmesg.1
    DESTINATION share/man/man1
)
