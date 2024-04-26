cmake_minimum_required(VERSION 3.17)
project(chasm)

include(cmake/CompilerWarnings.cmake)


set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(CHASM_SOURCES
        ${INC_DIR}/${PROJECT_NAME}/*.hpp
        ${INC_DIR}/${PROJECT_NAME}/ds/*.hpp
        ${SRC_DIR}/*.cpp
        ${SRC_DIR}/ds/*.cpp)

file(GLOB SRC_FILES ${CHASM_SOURCES})

add_executable(${PROJECT_NAME} ${SRC_FILES})
set_project_warnings(${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PRIVATE ${INC_DIR}/)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)

add_subdirectory(test)
