#ifndef ASSERTCONTROLLER_H
#define ASSERTCONTROLLER_H
#include "boards.h"
#include "nrfx_rtc.h"

#ifdef CFG_DEBUG_CUSTOM
#  define DEBUG_LED(led) nrf_gpio_pin_set(led)
#else
#  define DEBUG_LED(led)
#endif

inline static void _assertCustomImpl(const char *expr, const char *file, int line) __attribute__((__noreturn__));

inline static void _assertCustomImpl(const char *expr, const char *file, int line) {
#ifdef CFG_DEBUG_CUSTOM
    nrf_rtc_task_trigger(NRF_RTC1, NRF_RTC_TASK_START);
    nrf_gpio_pin_clear(DEBUG_LED_0);
    nrf_gpio_pin_clear(DEBUG_LED_1);
    nrf_gpio_pin_clear(DEBUG_LED_2);
    nrf_gpio_pin_clear(DEBUG_LED_3);
    nrf_gpio_pin_clear(DEBUG_LED_4);
    nrf_gpio_pin_clear(DEBUG_LED_5);
    nrf_gpio_pin_clear(DEBUG_LED_6);
    const uint32_t period = (line > 127) ? 32000 : 64000;
#endif
    led_state(STATE_WRITING_STARTED);
    while (true) {
#ifdef CFG_DEBUG_CUSTOM
        if (nrf_rtc_counter_get(NRF_RTC1) % period == 0) {
            if (line & 0b1) {
                nrf_gpio_pin_toggle(DEBUG_LED_0);
            }
            if (line & 0b10) {
                nrf_gpio_pin_toggle(DEBUG_LED_1);
            }
            if (line & 0b100) {
                nrf_gpio_pin_toggle(DEBUG_LED_2);
            }
            if (line & 0b1000) {
                nrf_gpio_pin_toggle(DEBUG_LED_3);
            }
            if (line & 0b10000) {
                nrf_gpio_pin_toggle(DEBUG_LED_4);
            }
            if (line & 0b100000) {
                nrf_gpio_pin_toggle(DEBUG_LED_5);
            }
            if (line & 0b1000000) {
                nrf_gpio_pin_toggle(DEBUG_LED_6);
            }
        }
#endif
    }
}

#define assertCustom(expr) ((expr) ? (void)(0) : _assertCustomImpl(#expr, __FILE__, __LINE__))

#endif
