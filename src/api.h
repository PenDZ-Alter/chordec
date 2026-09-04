// src/api.h
#ifndef API_H
#define API_H

#if defined(CHORDEC_STATIC_BUILD)
  #define CHORDEC_API
#else
  #if defined(_WIN32) || defined(__CYGWIN__)
  // All Shared libraries on Windows should use __declspec(dllexport) to export symbols and __declspec(dllimport) to import them. This macro handles that automatically.
    #if defined(chordec_parser_EXPORTS) || defined(chordec_gen_EXPORTS) || defined(chordec_math_EXPORTS) || defined(chordec_info_EXPORTS)
      #define CHORDEC_API __declspec(dllexport)
    #else
      #define CHORDEC_API __declspec(dllimport)
    #endif
  #else
    #define CHORDEC_API __attribute__((visibility("default")))
  #endif
#endif

#endif