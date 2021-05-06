set(Boost_USE_STATIC_LIBS OFF)

find_package(Boost REQUIRED COMPONENTS unit_test_framework)

set(CCOMP_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include/)
set(CCOMP_SOURCE_DIR ${PROJECT_SOURCE_DIR}/src/)

file(GLOB INCLUDES ${CCOMP_INCLUDE_DIR}/ccomp/*.hpp)
file(GLOB SOURCES ${CCOMP_SOURCE_DIR}/*.cpp)

list(REMOVE_ITEM SOURCES ${CCOMP_SOURCE_DIR}/main.cpp)

add_executable(Boost_Tests_run test.cpp ${INCLUDES} ${SOURCES})
target_include_directories(Boost_Tests_run PRIVATE ${CCOMP_INCLUDE_DIR} ${Boost_INCLUDE_DIRS})
target_link_libraries(Boost_Tests_run ${Boost_LIBRARIES})
target_compile_definitions(Boost_Tests_run PUBLIC UNIT_TESTS_ON)