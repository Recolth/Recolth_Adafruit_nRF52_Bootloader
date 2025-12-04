#include "qspi_dfu.h"

nrfx_err_t qspi_waitForReady() {
    while (nrfx_qspi_mem_busy_check() == NRFX_ERROR_BUSY) {
        ;
        ;
    }
    return NRFX_SUCCESS;
}

void qspi_erase_4(uint32_t blockStartAddress) {
    assertCustom((blockStartAddress % 0x1000) == 0);
    qspi_waitForReady();
    assertCustom(nrfx_qspi_erase(NRF_QSPI_ERASE_LEN_4KB, blockStartAddress) == NRFX_SUCCESS);
}

void qspi_write_4(const uint8_t *pData, uint32_t blockStartAddress) {
    assertCustom((blockStartAddress % 0x1000) == 0);
    qspi_waitForReady();
    assertCustom(nrfx_qspi_write(pData, 0x1000, blockStartAddress) == NRFX_SUCCESS);
}

void qspi_read(void *pData, size_t length, uint32_t address) {
    DEBUG_LCD_FUNC_LOWER();
    assertCustom((length % 4) == 0);

    assertCustom((address % 4) == 0);
    qspi_waitForReady();
    assertCustom(nrfx_qspi_read(pData, length, address) == NRFX_SUCCESS);
}

void qspi_write_4_retry(const uint8_t *pData, uint32_t blockStartAddress) {
    DEBUG_LCD_FUNC_LOWER();
    int     att            = 0;
    uint8_t blockr[0x1000] = {};
    while (1) {
        qspi_write_4(pData, blockStartAddress);

        qspi_read(blockr, 0x1000, blockStartAddress);
        int err = memcmp(pData, blockr, 0x1000);

        if (err == 0) {
            break;
        } else {
            qspi_erase_4(blockStartAddress);
            att++;
            assertCustom(att < 6);
        }
    }
}