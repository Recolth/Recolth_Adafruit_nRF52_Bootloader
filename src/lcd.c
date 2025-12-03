#include "lcd.h"
#include "nrfx.h"
#define false 0
#define true  1

#define COLS  16
#define LINES 2

const uint8_t _i2cAddr      = 0x27;
const uint8_t _data_mask[4] = {
    0x01 << 4,
    0x01 << 5,
    0x01 << 6,
    0x01 << 7,
};
const uint8_t _rs_mask        = 0x01;
const uint8_t _rw_mask        = 0x01 << 1;
const uint8_t _enable_mask    = 0x01 << 2;
const uint8_t _backlight_mask = 0x01 << 3;
const uint8_t _displaycontrol = 0x04;
const uint8_t _entrymode      = 0x02;
const uint8_t _cols           = COLS;
const uint8_t _lines          = LINES;
const uint8_t _row_offsets[4] = {
    0x00,
    0x40,
    0x00 + COLS,
    0x40 + COLS,
};

void _write2Wire(uint8_t data, uint8_t isData, uint8_t enable) {
    if (isData)
        data |= _rs_mask;
    if (enable)
        data |= _enable_mask;
    data |= _backlight_mask;
    data &= ~_rw_mask;

    if (I2C_master_start(_i2cAddr, I2C_WRITE) != I2C_STATUS_OK) {
        return;
    }
    I2C_master_write(data);
    I2C_master_stop();
}

void _writeNibble(uint8_t halfByte, uint8_t isData) {
    uint8_t data = isData ? _rs_mask : 0;

    data |= _backlight_mask;
    data &= ~_rw_mask;

    // map the data bits to PCF pins
    if (halfByte & 0x01)
        data |= _data_mask[0];
    if (halfByte & 0x02)
        data |= _data_mask[1];
    if (halfByte & 0x04)
        data |= _data_mask[2];
    if (halfByte & 0x08)
        data |= _data_mask[3];

    // E high, then low
    I2C_master_write(data | _enable_mask);
    I2C_master_write(data);
}

void _sendNibble(uint8_t halfByte, uint8_t isData) {
    if (I2C_master_start(_i2cAddr, I2C_WRITE) != I2C_STATUS_OK) {
        return;
    }
    _writeNibble(halfByte, isData);
    I2C_master_stop();
}

void _send(uint8_t value, uint8_t isData) {
    if (I2C_master_start(_i2cAddr, I2C_WRITE) != I2C_STATUS_OK) {
        return;
    }

    // high nibble
    _writeNibble((value >> 4) & 0x0F, isData);
    // low nibble
    _writeNibble(value & 0x0F, isData);

    I2C_master_stop();
}

void lcd_clear() {
    // Instruction: Clear display = 0x01
    _send(0x01, false);
    nrfx_coredep_delay_us(1600);
}

void lcd_setCursor(uint8_t col, uint8_t row) {
    if ((col < _cols) && (row < _lines)) {
        _send(0x80 | (_row_offsets[row] + col), false);
    }
}

void lcd_write(uint8_t ch) { _send(ch, true); }

void lcd_write_string(const char *ch) { // Write null-terminated string
    while (*ch) {
        _send(*ch++, true);
    }
}

void lcd_begin() {
    const uint8_t functionFlags = 0x08;
    _write2Wire(0x00, false, false);
    nrfx_coredep_delay_us(50000);

    // initialization sequence (4-bit mode)
    _sendNibble(0x03, false);
    nrfx_coredep_delay_us(4500);
    _sendNibble(0x03, false);
    nrfx_coredep_delay_us(200);
    _sendNibble(0x03, false);
    nrfx_coredep_delay_us(200);
    _sendNibble(0x02, false); // set 4-bit interface

    // Function set = 0x20
    _send(0x20 | functionFlags, false);

    // Display on, clear, left-to-right
    _send(0x08 | _displaycontrol, false);

    lcd_clear();

    _send(0x04 | _entrymode, false);

    lcd_setCursor(0, 0);
}