cmake_minimum_required(VERSION 3.20)
project(vari C)

set(CMAKE_C_STANDARD 99)

#add_executable(vari internet/fornaio.c)
add_executable(vari  ese_OSD/barbiere/barbiere_addormentato.c)
#add_executable(vari internet/fornaio.c)
#add_executable(vari Sistemi_Embedded/es7_barbieri.c)
#add_executable(vari varie/prove2.c)

target_link_libraries(vari pthread)
