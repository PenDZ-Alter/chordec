cmake_minimum_required(VERSION 3.14)
project(Chordec VERSION 0.3.3 LANGUAGES C CXX)

set(PROJECT_VERSION_REVISE "r1")

if(PROJECT_VERSION_REVISE STREQUAL "")
    set(PROJECT_FULL_VERSION "${PROJECT_VERSION}")
else()
    set(PROJECT_FULL_VERSION "${PROJECT_VERSION}-${PROJECT_VERSION_REVISE}")
endif()

option(BUILD_TEST "Build tests" OFF)
option(BUILD_MAIN "Build main application" OFF)
option(BUILD_EXPERIMENTAL "Build with experimental analysis" OFF)
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

if (BUILD_EXPERIMENTAL AND NOT BUILD_MAIN)
    message(FATAL_ERROR "If you want to use experimental, you need to build main too!")
endif()

if(NOT BUILD_MAIN AND NOT BUILD_TEST)
    message(FATAL_ERROR "Must build either main or tests.")
endif()


# ==========================================
# Static build definition
# ==========================================

if(NOT BUILD_SHARED_LIBS)
    add_compile_definitions(CHORDEC_STATIC_BUILD)
endif()

# ==========================================
# Function for configure API
# ==========================================
function(chordec_configure_api TARGET_NAME)

    if(BUILD_SHARED_LIBS)
        target_compile_definitions(${TARGET_NAME}
            PRIVATE CHORDEC_EXPORTS
        )
    else()
        target_compile_definitions(${TARGET_NAME}
            PUBLIC CHORDEC_STATIC_BUILD
        )
    endif()

endfunction()

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

target_link_libraries(chordec_pipeline PUBLIC
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

chordec_configure_api(chordec_parser)
chordec_configure_api(chordec_gen)
chordec_configure_api(chordec_math)
chordec_configure_api(chordec_info)
chordec_configure_api(chordec_export)
chordec_configure_api(chordec_pipeline)
chordec_configure_api(chordec_cli)
chordec_configure_api(chordec_core)


# ==========================================
# Experimental Libraries
# ==========================================

if(BUILD_EXPERIMENTAL)

    add_library(chordec_exp_chroma
        src/experimental/chroma.cpp
    )

    target_include_directories(chordec_exp_chroma PUBLIC
        includes/experimental
        libs
    )

    add_library(chordec_exp_pipeline
        src/experimental/pipeline.cpp
    )

    target_include_directories(chordec_exp_pipeline PUBLIC
        includes/experimental
        libs
    )

    add_library(chordec_exp_cqt
        src/experimental/cqt.cpp
    )

    target_include_directories(chordec_exp_cqt PUBLIC
        includes/experimental
        libs
    )

    add_library(chordec_exp_harmonic
        src/experimental/harmonic_processing.cpp
    )

    target_include_directories(chordec_exp_harmonic PUBLIC
        includes/experimental
        libs
    )

    add_library(chordec_exp_nnls
        src/experimental/nnls.cpp
    )

    target_include_directories(chordec_exp_nnls PUBLIC
        includes/experimental
        libs
    )

    add_library(chordec_exp_spectral
        src/experimental/spectral_whitening.cpp
    )

    target_include_directories(chordec_exp_spectral PUBLIC
        includes/experimental
        libs
    )

endif()

if(BUILD_EXPERIMENTAL)

    target_link_libraries(chordec_core PRIVATE
        chordec_exp_pipeline
    )

    chordec_configure_api(chordec_exp_chroma)
    chordec_configure_api(chordec_exp_pipeline)
    chordec_configure_api(chordec_exp_cqt)
    chordec_configure_api(chordec_exp_harmonic)
    chordec_configure_api(chordec_exp_nnls)
    chordec_configure_api(chordec_exp_spectral)

endif()

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

if(BUILD_TEST)

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
