cmake_minimum_required(VERSION 3.14)
project(Chordec CXX)

# Gunakan standar C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Definisikan Absolute Path dari project root
add_compile_definitions(SAMPLE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

# Include directory untuk header files
include_directories(src src/parser)

# Buat static library dari parser kamu biar hemat waktu build
add_library(audio_parser src/parser.cpp)

# ==========================================
# KONFIGURASI AUTOMATED TESTING (CTest)
# ==========================================
enable_testing()

# Helper function biar gak nulis berulang-ulang
function(add_audio_test TEST_NAME TEST_SRC)
    # 1. Build executable buat test tertentu
    add_executable(${TEST_NAME} ${TEST_SRC})
    
    # 2. Link dengan library parser utama
    target_link_libraries(${TEST_NAME} PRIVATE audio_parser)
    
    # 3. Daftarkan ke CTest
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endfunction()

# Panggil fungsi di atas untuk tiap file test
add_audio_test(test_parser test/parser.cpp)
# add_audio_test(test_mp3 test/test_mp3.cpp)