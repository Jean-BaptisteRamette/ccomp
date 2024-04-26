set(Boost_USE_STATIC_LIBS OFF)
set(Boost_NO_WARN_NEW_VERSIONS 1)

find_package(Boost 1.8.0 REQUIRED COMPONENTS unit_test_framework)

set(CHASM_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include/)
set(CHASM_SOURCE_DIR ${PROJECT_SOURCE_DIR}/src/)

file(GLOB INCLUDES_AS ${CHASM_INCLUDE_DIR}/${PROJECT_NAME}/*.hpp)
file(GLOB INCLUDES_DS ${CHASM_INCLUDE_DIR}/${PROJECT_NAME}/ds/*.hpp)
file(GLOB SOURCES_AS ${CHASM_SOURCE_DIR}/*.cpp)
file(GLOB SOURCES_DS ${CHASM_SOURCE_DIR}/ds/*.cpp)

list(REMOVE_ITEM SOURCES_AS ${CHASM_SOURCE_DIR}/main.cpp)

add_executable(Boost_Tests_run
        main.cpp
        options_fixture.hpp
        lexer_tokens.cpp
        lexer_numeric.cpp
        parser_statements.cpp
        symbols.cpp
        instructions.cpp
        codegen.cpp
        ds_flow.cpp
        ${INCLUDES_AS}
        ${INCLUDES_DS}
        ${SOURCES_AS}
        ${SOURCES_DS})

target_include_directories(Boost_Tests_run PRIVATE ${CHASM_INCLUDE_DIR} ${Boost_INCLUDE_DIRS})
target_link_libraries(Boost_Tests_run ${Boost_LIBRARIES})
target_compile_definitions(Boost_Tests_run PUBLIC UNIT_TESTS_ON)
target_compile_features(Boost_Tests_run PRIVATE cxx_std_23)