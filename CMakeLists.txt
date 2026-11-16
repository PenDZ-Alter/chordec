cmake_minimum_required(VERSION 3.14)
project(Chordec VERSION 0.3.2 LANGUAGES C CXX)

set(PROJECT_VERSION_REVISE "r5")

if(PROJECT_VERSION_REVISE STREQUAL "")
    set(PROJECT_FULL_VERSION "${PROJECT_VERSION}")
else()
    set(PROJECT_FULL_VERSION "${PROJECT_VERSION}-${PROJECT_VERSION_REVISE}")
endif()

option(BUILD_TESTS "Build tests" OFF)
option(BUILD_MAIN "Build main application" OFF)
option(BUILD_SHARED_LIBS "Build shared libraries instead of static" OFF)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/version.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/generated/version.h"
)

if(NOT BUILD_MAIN AND NOT BUILD_TESTS)
    message(FATAL_ERROR "Must build either main or tests.")
endif()

# ==========================================
# Static build definition
# ==========================================

if(NOT BUILD_SHARED_LIBS)
    add_compile_definitions(CHORDEC_STATIC_BUILD)
endif()

# ==========================================
# Core libraries
# ==========================================

add_library(chordec_parser src/parser.cpp)
target_include_directories(chordec_parser PUBLIC
    includes
    libs
    libs/parser
)

add_library(chordec_gen src/chord.cpp)
target_include_directories(chordec_gen PUBLIC
    includes
    libs
)

add_library(chordec_math src/ca_math.cpp)
target_include_directories(chordec_math PUBLIC
    includes
    libs
)

add_library(chordec_info src/info.cpp)
target_include_directories(chordec_info PUBLIC
    includes
    libs
    "${CMAKE_CURRENT_BINARY_DIR}/generated"
)

add_library(chordec_export src/export.cpp)
target_include_directories(chordec_export PUBLIC
    includes
    libs
)

# ==========================================
# Analysis pipeline
# ==========================================

add_library(chordec_pipeline src/pipeline.cpp)

target_include_directories(chordec_pipeline PUBLIC
    includes
    libs
)

target_link_libraries(chordec_pipeline PRIVATE
    chordec_math
    chordec_gen
)

add_library(chordec_cli src/cli.cpp)

target_include_directories(chordec_cli PUBLIC
    includes
    libs
)

target_link_libraries(chordec_cli PRIVATE
    chordec_info
)

add_library(chordec_core src/core.cpp)

target_include_directories(chordec_core PUBLIC
    includes
    libs
    libs/parser
    "${CMAKE_CURRENT_BINARY_DIR}/generated"
)

target_link_libraries(chordec_core PRIVATE
    chordec_parser
    chordec_gen
    chordec_math
    chordec_info
    chordec_export
    chordec_cli
    chordec_pipeline
)

# ==========================================
# Main application
# ==========================================

if(BUILD_MAIN)
    add_executable(chordec main.cpp)

    target_link_libraries(chordec PRIVATE
        chordec_core
    )
endif()

# ==========================================
# Testing
# ==========================================

if(BUILD_TESTS)

    enable_testing()

    function(add_audio_test TEST_NAME TEST_SRC)

        add_executable(${TEST_NAME} ${TEST_SRC})

        target_link_libraries(${TEST_NAME} PRIVATE
            chordec_parser
            chordec_gen
            chordec_math
            chordec_info
            chordec_export
            chordec_pipeline
            chordec_cli
        )

        target_compile_definitions(${TEST_NAME} PRIVATE
            SAMPLE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
        )

        add_test(
            NAME ${TEST_NAME}
            COMMAND ${TEST_NAME}
        )

    endfunction()

    add_audio_test(test_parser test/parser.cpp)
    add_audio_test(test_unsmooth test/unsmooth.cpp)
    add_audio_test(test_smooth test/smooth.cpp)
    add_audio_test(test_chroma test/chroma.cpp)
    add_audio_test(test_weighted_7th test/weighted_7th.cpp)

endif()
