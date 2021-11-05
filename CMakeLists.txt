cmake_minimum_required(VERSION 3.20)
project(vari C)

set(CMAKE_C_STANDARD 99)

#add_executable(vari internet/fornaio.c)
add_executable(vari barbiere_addormentato.c)

target_link_libraries(vari pthread)
