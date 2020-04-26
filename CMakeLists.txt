cmake_minimum_required(VERSION 3.0)
project(mpdwatch)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmakemodules/")
cmake_policy(SET CMP0054 NEW)
cmake_policy(SET CMP0057 NEW)

set(CMAKE_CXX_STANDARD 14)
find_package(LibMPDClient REQUIRED)

add_subdirectory(libSSD1306)

set(SSD1306_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libSSD1306/lib
        CACHE PATH "SSD1306 include directory")

add_executable(mpdwatch mpdwatch/mpdwatch.cpp)

target_include_directories(mpdwatch PUBLIC
                               $<BUILD_INTERFACE:${SSD1306_INCLUDE_DIR}>
                               $<INSTALL_INTERFACE:${some_dest}>)


target_link_libraries(mpdwatch SSD1306)
target_link_libraries(mpdwatch ${LIBMPDCLIENT_LIBRARY})
