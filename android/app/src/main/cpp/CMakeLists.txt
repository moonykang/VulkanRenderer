
cmake_minimum_required(VERSION 3.18.1)


project("dev.moon.littlerenderer")

add_library(
        renderer
        SHARED
        native-lib.cpp)


find_library(log-lib log)

target_link_libraries(
        renderer
        ${log-lib})