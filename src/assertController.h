#ifndef ASSERTCONTROLLER_H
#define ASSERTCONTROLLER_H
#include "boards.h"
#include "lcd.h"
#include "nrfx_rtc.h"

#ifdef CFG_DEBUG_CUSTOM
#  define DEBUG_LED(led)    nrf_gpio_pin_set(led)
#  define DEBUG_LCD(string) lcd_write_string(string)
#  define DEBUG_LCD_FUNC_UPPER()                                                                                                                          \
      lcd_setCursor(0, 0);                                                                                                                                \
      lcd_clear();                                                                                                                                        \
      lcd_write_string(__func__)
#  define DEBUG_LCD_FUNC_LOWER()                                                                                                                          \
      lcd_setCursor(0, 1);                                                                                                                                \
      lcd_write_string(__func__)
#  define DEBUG_LCD_LINE()                                                                                                                                \
      lcd_setCursor(13, 0);                                                                                                                               \
      DEBUG_LCD("   ");                                                                                                                                   \
      lcd_setCursor(13, 0);                                                                                                                               \
      LCD_PRINT_NUM(__LINE__);
#  define DEBUG_LCD_CLEAR() lcd_clear()
#else
#  define DEBUG_LED(led)
#  define DEBUG_LCD(string)
#  define DEBUG_LCD_FUNC_UPPER()
#  define DEBUG_LCD_FUNC_LOWER()
#  define DEBUG_LCD_LINE()
#  define DEBUG_LCD_CLEAR()
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
