cmake_minimum_required(VERSION 2.8.11)
project(ArduinoScope)
add_definitions(-Wall -Wextra)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5Widgets REQUIRED)
find_package(Qt5PrintSupport REQUIRED)
find_package(Qt5SerialPort REQUIRED)

QT5_WRAP_UI(UIS
            mainwindow.ui
            parameters.ui
            arduino/arduinoconnectiondlg.ui)

include_directories(sensordatahandler)

add_executable(${CMAKE_PROJECT_NAME} ${UIS}
               main.cpp
               mainwindow.cpp
               qcustomplot/qcustomplot.cpp
               arduino/arduinoconnectiondlg.cpp
               arduino/serialportreader.cpp
               sensordatahandler/sensordatahandler.cpp
               resource.qrc)

set_property(TARGET ${CMAKE_PROJECT_NAME} APPEND_STRING PROPERTY COMPILE_FLAGS "-std=c++11")
target_link_libraries(${CMAKE_PROJECT_NAME} Qt5::Widgets Qt5::PrintSupport Qt5::SerialPort)

