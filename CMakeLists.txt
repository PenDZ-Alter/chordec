cmake_minimum_required(VERSION 3.14)
project(Chordec CXX)

option(BUILD_TESTS "Build tests" OFF)
option(BUILD_MAIN "Build main" OFF)
option(BUILD_SHARED_LIBS "Build shared libraries instead of static" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Hide symbol as default if using shared libraries (Best Practice)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)

if (NOT BUILD_MAIN AND NOT BUILD_TESTS)
    message(FATAL_ERROR "Must build either main or tests.")
endif()

# add_compile_definitions(SAMPLE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_definitions(test_parser PRIVATE SAMPLE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

# Include directories
# include_directories(src src/parser)

# --- Target Libraries ---
# Note: Jangan tentukan STATIC/SHARED di sini. 
# CMake akan otomatis menyesuaikan berdasarkan variabel BUILD_SHARED_LIBS.

# Create static library
add_library(chordec_parser src/parser.cpp)
target_include_directories(chordec_parser PUBLIC src src/parser)

add_library(chordec_gen src/chord.cpp)
target_include_directories(chordec_gen PUBLIC src)

add_library(chordec_math src/math.cpp)
target_include_directories(chordec_math PUBLIC src)

if (BUILD_MAIN)
    add_executable(chordec main.cpp)
    target_link_libraries(chordec PRIVATE chordec_parser chordec_gen chordec_math)
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
        
        # 2. Link with the chordec_parser library
        target_link_libraries(${TEST_NAME} PRIVATE chordec_parser chordec_gen chordec_math)
        target_compile_definitions(${TEST_NAME} PRIVATE SAMPLE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
        
        # 3. Register to CTest
        add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    endfunction()

    # Call function for testing
    add_audio_test(test_parser test/parser.cpp)
    add_audio_test(test_unsmooth test/unsmooth.cpp)
    add_audio_test(test_smooth test/smooth.cpp)
    add_audio_test(test_chroma test/chroma.cpp)
endif()