cmake_minimum_required(VERSION 3.16)

project(sir_lsq_dialog VERSION 1.0 LANGUAGES CXX Fortran)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    lsqdialog.cpp
    lsqdialog.h
    lsqdialog.ui
    weightparamsdialog.cpp
    weightparamsdialog.h
    weightparamsdialog.ui
    lsq_fortran.f90
)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Core Qt6::Widgets)
