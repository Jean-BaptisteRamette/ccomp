cmake_minimum_required(VERSION 3.17)
project(ccomp)

include(cmake/CompilerWarnings.cmake)

set(CMAKE_CXX_STANDARD 23)

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(CCOMP_SOURCES
        ${INC_DIR}/${PROJECT_NAME}/*.hpp
        ${SRC_DIR}/*.cpp)

file(GLOB SRC_FILES ${CCOMP_SOURCES})

add_executable(${PROJECT_NAME} ${SRC_FILES})
set_project_warnings(${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PRIVATE ${INC_DIR}/)
add_subdirectory(test)
