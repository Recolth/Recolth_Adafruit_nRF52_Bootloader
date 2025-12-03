#ifndef I2C_H
#define I2C_H

#include <stdint.h>

#define I2C_READ             1 // Least significant bit in TWAR is R/W intent in master
#define I2C_WRITE            0 // Least significant bit in TWAR is R/W intent in master

#define I2C_STATUS_OK        0
#define I2C_STATUS_ERROR     1
#define I2C_STATUS_START_ERR 2

//=====================================MASTER=============================================

void    I2C_master_init(void);
uint8_t I2C_master_start(uint8_t target_addr, uint8_t read_write);
void    I2C_master_stop(void);
uint8_t I2C_master_write(uint8_t data);

#endif // I2C_H
