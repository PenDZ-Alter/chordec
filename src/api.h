// src/api.h
#ifndef API_H
#define API_H

#if defined(_WIN32) || defined(__CYGWIN__)
  #ifdef chordec_parser_EXPORTS // CMake otomatis buat macro <target_name>_EXPORTS saat build Shared
    #define CHORDEC_API __declspec(dllexport)
  #else
    #define CHORDEC_API __declspec(dllimport)
  #endif
#else
  #define CHORDEC_API __attribute__((visibility("default")))
#endif

#endif