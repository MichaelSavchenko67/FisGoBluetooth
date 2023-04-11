cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_COMPILER /usr/local/usr/bin/arm-linux-g++)
set(CMAKE_C_COMPILER /usr/local/usr/bin/arm-linux-gcc)

set(CMAKE_EXE_LINKER_FLAGS "-lboost_regex")

add_library(FisGo_bluetooth SHARED FisGo_bluetooth.cpp FisGo_bluetooth.h)

add_executable(test_FisGo_bluetooth  main.cpp)
target_link_libraries(test_FisGo_bluetooth FisGo_bluetooth bluetooth)
