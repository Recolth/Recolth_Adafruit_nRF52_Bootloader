#ifndef ASSERTCONTROLLER_H
#define ASSERTCONTROLLER_H
#include "boards.h"

// #define DEBUG_ASSERT_ON

inline static void _assertCustomImpl(const char *expr, const char *file, int line) __attribute__((__noreturn__));

inline static void _assertCustomImpl(const char *expr, const char *file, int line) {
    uint16_t i = 0;
    while (true) {
#ifdef DEBUG_ASSERT_ON
        if (i % 64000 == 0) {
            PRINTF("Assertion failed: %s in file %s at line %d", expr, file, line);
        }
#endif

        led_state(STATE_WRITING_STARTED);

        i++;
    }
}

#define assertCustom(expr) ((expr) ? (void)(0) : _assertCustomImpl(#expr, __FILE__, __LINE__))

#endif
