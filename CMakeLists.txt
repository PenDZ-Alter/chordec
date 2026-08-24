cmake_minimum_required(VERSION 3.14)
project(Chordec CXX)

option(BUILD_TESTS "Build tests" OFF)
option(BUILD_MAIN "Build main" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (BUILD_MAIN AND BUILD_TESTS)
    message(FATAL_ERROR "Cannot build both main and tests at the same time.")
endif()

if (NOT BUILD_MAIN AND NOT BUILD_TESTS)
    message(FATAL_ERROR "Must build either main or tests.")
endif()

add_compile_definitions(SAMPLE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

# Include directories
include_directories(src src/parser)

# Create static library
add_library(audio_parser src/parser.cpp)
add_library(chord_generator src/chord.cpp)
add_library(math_utils src/math.cpp)

if (BUILD_MAIN)
    # Create executable
    add_executable(chordec main.cpp)
endif()


if (BUILD_TESTS)
    # ==========================================
    # Automated Testing Configuration (CTest)
    # ==========================================
    enable_testing()

    # Helper function
    function(add_audio_test TEST_NAME TEST_SRC)
        # 1. Build executable
        add_executable(${TEST_NAME} ${TEST_SRC})
        
        # 2. Link with the audio_parser library
        target_link_libraries(${TEST_NAME} PRIVATE audio_parser)
        
        # 3. Register to CTest
        add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    endfunction()

    # Call function for testing
    add_audio_test(test_parser test/parser.cpp)
    add_audio_test(test_unsmooth test/unsmooth.cpp)
    add_audio_test(test_smooth test/smooth.cpp)
endif()