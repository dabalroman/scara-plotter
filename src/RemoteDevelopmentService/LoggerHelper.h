#ifndef LOGGER_HELPER
#define LOGGER_HELPER

#include "RemoteDevelopmentService.h"

extern RemoteDevelopmentService *gRemoteDevelopmentService;

inline void printLn(const char *format, ...) {
    char buf[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);

    Serial.println(buf);
    // if (!!gRemoteDevelopmentService) {
    //     gRemoteDevelopmentService->remotePrintLn("%s", buf);
    // };
}

#endif //LOGGER_HELPER
