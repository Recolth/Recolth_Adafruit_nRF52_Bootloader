#include "I2C.h"
#include <nrf52840.h>
#include <nrf52840_bitfields.h>
#include <nrf_gpio.h>

#define I2C_SCL_PIN    5
#define I2C_SDA_PIN    4

#define I2C_TX_BUF_MAX 50 // adjust as needed

static uint8_t i2c_tx_buf[I2C_TX_BUF_MAX];
static uint8_t i2c_tx_len = 0;
static uint8_t i2c_addr   = 0;
static uint8_t i2c_active = 0;

//===================================MASTER_FUNCTIONS===================================

void I2C_master_init(void) {

    // Configure pins as open-drain style outputs (external pull-ups required)
    nrf_gpio_cfg(I2C_SCL_PIN, NRF_GPIO_PIN_DIR_INPUT, NRF_GPIO_PIN_INPUT_CONNECT, NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_S0D1, NRF_GPIO_PIN_NOSENSE);

    nrf_gpio_cfg(I2C_SDA_PIN, NRF_GPIO_PIN_DIR_INPUT, NRF_GPIO_PIN_INPUT_CONNECT, NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_S0D1, NRF_GPIO_PIN_NOSENSE);

    // Select pins in TWIM0
    NRF_TWIM0->PSEL.SCL = I2C_SCL_PIN;
    NRF_TWIM0->PSEL.SDA = I2C_SDA_PIN;

    // 100 kHz
    NRF_TWIM0->FREQUENCY = TWIM_FREQUENCY_FREQUENCY_K100;

    // Enable the peripheral
    NRF_TWIM0->ENABLE = (TWIM_ENABLE_ENABLE_Enabled << TWIM_ENABLE_ENABLE_Pos);
}

/**
 * Start a write transaction: buffer is cleared, address is stored.
 * Read direction is not implemented in this minimal version.
 */
uint8_t I2C_master_start(uint8_t target_addr, uint8_t read_write) {
    if (read_write != I2C_WRITE) {
        // For LCD we only implement write; you can extend for read later
        return I2C_STATUS_START_ERR;
    }

    i2c_addr   = (target_addr & 0x7F);
    i2c_tx_len = 0;
    i2c_active = 1;

    return I2C_STATUS_OK;
}

// Buffer a byte to be sent later at stop()
uint8_t I2C_master_write(uint8_t data) {
    if (!i2c_active) {
        return I2C_STATUS_ERROR;
    }
    if (i2c_tx_len >= I2C_TX_BUF_MAX) {
        return I2C_STATUS_ERROR; // buffer overflow
    }

    i2c_tx_buf[i2c_tx_len++] = data;
    return I2C_STATUS_OK;
}

// Perform the actual TWIM transfer and send STOP
void I2C_master_stop(void) {
    if (!i2c_active) {
        return;
    }

    if (i2c_tx_len == 0) {
        i2c_active = 0;
        return; // nothing to send
    }

    // 1) Peripheral enabled
    if (NRF_TWIM0->ENABLE != (TWIM_ENABLE_ENABLE_Enabled << TWIM_ENABLE_ENABLE_Pos)) { /* bug */
    }

    // 2) Pins connected
    if (NRF_TWIM0->PSEL.SCL == 0xFFFFFFFF || NRF_TWIM0->PSEL.SDA == 0xFFFFFFFF) { /* bug */
    }

    NRF_TWIM0->ADDRESS        = i2c_addr;
    NRF_TWIM0->TXD.PTR        = (uint32_t)i2c_tx_buf;
    NRF_TWIM0->TXD.MAXCNT     = i2c_tx_len;

    NRF_TWIM0->EVENTS_STOPPED = 0;
    NRF_TWIM0->EVENTS_ERROR   = 0;
    NRF_TWIM0->ERRORSRC       = NRF_TWIM0->ERRORSRC;                              // clear

    // Option A: use SHORTS
    NRF_TWIM0->SHORTS           = TWIM_SHORTS_LASTTX_STOP_Msk;

    NRF_TWIM0->EVENTS_TXSTARTED = 0;
    NRF_TWIM0->TASKS_STARTTX    = 1;

    uint32_t timeout            = 1000000;

    while (--timeout) {
        if (NRF_TWIM0->EVENTS_STOPPED) {
            NRF_TWIM0->EVENTS_STOPPED = 0;
            break;
        }
        if (NRF_TWIM0->EVENTS_ERROR) {
            NRF_TWIM0->EVENTS_ERROR = 0;
            break;
        }
    }
    if (!timeout) {
    }

    NRF_TWIM0->SHORTS = 0;
    i2c_active        = 0;
}
