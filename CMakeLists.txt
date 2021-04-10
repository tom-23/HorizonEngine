project(horizonengine LANGUAGES CXX)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.14")

if (APPLE)
    cmake_minimum_required(VERSION 3.1)
    set(HORIZONENGINE_EXECUTABLE_NAME "HorizonEngine")
else()
    cmake_minimum_required(VERSION 2.8.12)
    set(HORIZONENGINE_EXECUTABLE_NAME ${PROJECT_NAME})
endif()

# C++ 14 Support:
if (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 3.1)
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(HORIZONENGINE_DEBUG ON)
    add_definitions( -DHORIZONENGINE_DEBUG  )
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Network REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Network REQUIRED)
list(APPEND horizonengine_LIBRARIES Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Network)

# Compiling debug version
if(CMAKE_COMPILER_IS_GNUCXX OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-long-long")
    set(CMAKE_CXX_FLAGS_DEBUG
        "${CMAKE_CXX_FLAGS_DEBUG} -Wextra -Wall -pedantic -Wfloat-equal -Woverloaded-virtual -Wundef -Wno-inconsistent-missing-destructor-override")
endif()

add_subdirectory(lib/LabSound)
add_subdirectory(src)



