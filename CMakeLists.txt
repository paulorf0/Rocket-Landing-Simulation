cmake_minimum_required(VERSION 3.10)

project(FluidSimulator)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

set(SOURCES
    main.cpp      
        scr/rocket.cpp
  )

add_executable(sfml-app ${SOURCES})

target_include_directories(sfml-app PRIVATE ${CMAKE_SOURCE_DIR})

target_link_libraries(sfml-app PRIVATE sfml-graphics sfml-window sfml-system)
