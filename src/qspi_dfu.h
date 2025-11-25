#include "assertController.h"
#include "boards.h"
#include <flash_nrf5x.h>
#include <nrfx_errors.h>
#include <nrfx_qspi.h>

void qspi_erase_4(uint32_t blockStartAddress);
void qspi_write_4_retry(const uint8_t *pData, uint32_t blockStartAddress);
void qspi_read(void *pData, size_t length, uint32_t address);

int ecc_checkAppBlock(uint32_t qspiBlockAddress, uint32_t internalBlockAddress, uint8_t *pData, uint8_t *blockr);