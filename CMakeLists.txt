cmake_minimum_required(VERSION 3.5)

project(CatEditor VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

file(GLOB UI ui/*.ui)
file(GLOB HEADERS *.h)
file(GLOB SRCS *.cpp)

message(${UI})
message(${HEADERS})

qt5_wrap_cpp(headers ${HEADERS})
qt5_wrap_ui(ui ${UI})

set(PROJECT_SOURCES ${SRCS} ${ui} ${headers})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/run")

add_executable(CatEditor ${PROJECT_SOURCES})

add_subdirectory("Cat")
include_directories("Cat/include")
include_directories("Cat")
include_directories(${CMAKE_BINARY_DIR}/exports/)

target_link_libraries(CatEditor PRIVATE Qt${QT_VERSION_MAJOR}::Widgets cat)

set_target_properties(CatEditor PROPERTIES AUTOUIC_SEARCH_PATHS "ui")
