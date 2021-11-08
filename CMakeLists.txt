cmake_minimum_required(VERSION 3.20)
project(vari C)

set(CMAKE_C_STANDARD 99)

#add_executable(vari internet/fornaio.c)
add_executable(vari ese_OSD/filosofi/filosofi.c)
#add_executable(vari internet/distributore_aranciata.c)
#add_executable(vari Sistemi_Embedded/2/ese2p2.c)
#add_executable(vari varie/prove.c)

target_link_libraries(vari pthread)
