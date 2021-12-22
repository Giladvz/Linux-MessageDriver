cmake_minimum_required(VERSION 3.20)
project(3 C)

set(CMAKE_C_STANDARD 11)

add_executable(3 message_slot.c message_slot.h message_sender.c message_reader.c)
include_directories(/usr/src/linux-headers-5.0.0-32-generic/include)