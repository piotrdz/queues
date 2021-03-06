cmake_minimum_required(VERSION 2.8.8)

project(queues C CXX)

exec_program(g++ ARGS --version OUTPUT_VARIABLE CMAKE_CXX_COMPILER_VERSION)

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    if ( (CMAKE_CXX_COMPILER_VERSION MATCHES ".* +[0-3]\\..*") OR
         (CMAKE_CXX_COMPILER_VERSION MATCHES ".* +4\\.[0-6].*") )
    message(FATAL_ERROR "GCC version < 4.7 not supported!")
    else()
        message(STATUS "Detected GCC version 4.7+")
        set(CXX11_FLAGS "-std=gnu++11")
    endif()
elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    message(STATUS "Detected Clang")
    set(CXX11_FLAGS "-std=c++11")
else()
    message(FATAL_ERROR "Compiler not supported!")
endif()

set(CMAKE_MODULE_PATH "${queues_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

if(NOT CMAKE_BUILD_TYPE)
    if(WIN32)
        set(CMAKE_BUILD_TYPE release)
    else()
        set(CMAKE_BUILD_TYPE debug)
    endif()
endif()

set(CMAKE_CXX_FLAGS_RELEASE "${CXX11_FLAGS} -O2 -Wall")
set(CMAKE_CXX_FLAGS_DEBUG "${CXX11_FLAGS} -g -O0 -Wall")

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5Widgets REQUIRED)
find_package(Qwt REQUIRED)

include_directories(
    src
    ${QWT_INCLUDE_DIR}
)

set(queues_SOURCES
    src/main.cpp

    src/engine/simulation.cpp
    src/engine/simulation_check_helper.cpp
    src/engine/simulation_input_output_helper.cpp
    src/engine/event_priority_queue.cpp

    src/stats/station_stats.cpp
    src/stats/system_stats.cpp

    src/ui/connection_item.cpp
    src/ui/distribution_params_widget.cpp
    src/ui/main_window.cpp
    src/ui/simulation_scene.cpp
    src/ui/simulation_thread.cpp
    src/ui/simulation_view.cpp
    src/ui/station_item.cpp
    src/ui/statistic_item_widget.cpp
    src/ui/statistics_series_data.cpp
    src/ui/statistics_window.cpp
)

qt5_add_resources(queues_RESOURCES
    resources.qrc
)

qt5_wrap_ui(queues_FORMS
    forms/distribution_params_widget.ui
    forms/main_window.ui
    forms/statistic_item_widget.ui
    forms/statistics_window.ui
)

set(queues_ALL_SOURCES
    ${queues_SOURCES}
    ${queues_FORMS}
    ${queues_RESOURCES}
)

add_executable(queues ${queues_ALL_SOURCES})
target_link_libraries(queues ${QWT_LIBRARY})
qt5_use_modules(queues Widgets)
