#include "assertController.h"
#include "boards.h"
#include <flash_nrf5x.h>
#include <nrfx_errors.h>
#include <nrfx_qspi.h>

typedef enum {
    BF_EMPTY         = 0xFFFFffff,               // erased state
    BF_READY         = 0xFFFFf0f0,               // fresh image on QSPI
    BF_TRIAL_PENDING = 0xF0F0f0f0,               // about to attempt first boot
    BF_BAD           = 0xE0E0e0e0,               // trial failed, device bricked
} BootFlag;

#define QSPI_ADDRESS_DFU_MAGIC_BLOCK  0x000FF000
#define QSPI_OFFSET_DFU_MAGIC_PRESENT 0x00000FF7 // BootFlag location
#define QSPI_OFFSET_DFU_FILE_LENGTH   0x00000FFB // app size location

void       qspi_erase_4(uint32_t blockStartAddress);
void       qspi_write_4_retry(const uint8_t *pData, uint32_t blockStartAddress);
void       qspi_read(void *pData, size_t length, uint32_t address);
nrfx_err_t qspi_waitForReady();

bool is_qspi_dfu_ready();
void qspi_dfu_process();

int ecc_checkAppBlock(uint32_t qspiBlockAddress, uint32_t internalBlockAddress, uint8_t *pBufQSPI, uint8_t *pBufInternal);