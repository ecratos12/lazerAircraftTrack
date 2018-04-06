cmake_minimum_required(VERSION 3.9)
project(lazerAircraftTrack)

set(CMAKE_CXX_STANDARD 11)

include_directories(.)

add_executable(lazerAircraftTrack
        src/LazerOrientation.cpp
        src/LazerOrientation.h
        LICENSE
        src/main.cpp
        src/Radar.cpp
        src/Radar.h
        README.md
        src/SearchService.cpp
        src/SearchService.h
        src/SafetyManager.cpp
        src/SafetyManager.h)
