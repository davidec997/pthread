cmake_minimum_required(VERSION 3.20)
project(vari C)

set(CMAKE_C_STANDARD 99)

#add_executable(vari internet/fornaio.c)
#add_executable(vari ese_OSD/mensa_leonardi.c)
#add_executable(vari internet/fornaio.c)
add_executable(vari Sistemi_Embedded/incrocio_semplificato.c)
#add_executable(vari varie/prove3.c)

target_link_libraries(vari pthread)
