#ifndef OS_DETECTION_H
#define OS_DETECTION_H

#ifdef _WIN32
    #define OS "Windows"
#elif __APPLE__
    #include "TargetConditionals.h"
    #if TARGET_OS_MAC
        #define OS "Mac OS X"
    #endif
#elif __linux__
    #define OS "Linux"
#elif __unix__
    #define OS "Unix"
#else
    #define OS "Other"
#endif

#endif //OS_DETECTION_H
