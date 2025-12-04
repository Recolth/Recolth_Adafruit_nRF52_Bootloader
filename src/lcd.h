#ifndef H_LCD_H
#define H_LCD_H

#include "I2C.h"

#define LCD_PRINT_NUM(num)                                                                                                                                \
    {                                                                                                                                                     \
        char buf[3];                                                                                                                                      \
        sprintf(buf, "%d", num);                                                                                                                          \
        lcd_write_string(buf);                                                                                                                            \
    }

void lcd_begin();
void lcd_write(uint8_t ch);
void lcd_setCursor(uint8_t col, uint8_t row);
void lcd_clear();
void lcd_write_string(const char *ch);

#endif