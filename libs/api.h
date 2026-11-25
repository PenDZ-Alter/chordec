#ifndef API_H
#define API_H

#if defined(CHORDEC_STATIC_BUILD)

    #define CHORDEC_API

#elif defined(_WIN32) || defined(__CYGWIN__)

    #if defined(CHORDEC_EXPORTS)
        #define CHORDEC_API __declspec(dllexport)
    #else
        #define CHORDEC_API __declspec(dllimport)
    #endif

#else

    #define CHORDEC_API __attribute__((visibility("default")))

#endif

#endif // API_H
