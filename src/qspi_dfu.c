#include "qspi_dfu.h"
#include <bootloader.h>
#include <bootloader_settings.h>
#include <bootloader_types.h>
#include <dfu_types.h>

static uint32_t app_length = 0;

bool is_qspi_dfu_ready() {
    BootFlag magic_dfu;
    qspi_read(&magic_dfu, sizeof(magic_dfu), QSPI_ADDRESS_DFU_MAGIC_BLOCK + QSPI_OFFSET_DFU_MAGIC_PRESENT);
    qspi_read(&app_length, sizeof(app_length), QSPI_ADDRESS_DFU_MAGIC_BLOCK + QSPI_OFFSET_DFU_FILE_LENGTH);

    if (magic_dfu == BF_READY) {
        if (app_length && app_length <= DFU_IMAGE_MAX_SIZE_FULL) {
            return true;
        }
        magic_dfu = BF_EMPTY;
        uint8_t magicBlock[0x1000];
        qspi_read(&magicBlock, sizeof(magicBlock), QSPI_ADDRESS_DFU_MAGIC_BLOCK);
        memcpy(&magicBlock[QSPI_OFFSET_DFU_MAGIC_PRESENT], &magic_dfu, sizeof(magic_dfu));
        qspi_write_4_retry(magicBlock, QSPI_ADDRESS_DFU_MAGIC_BLOCK);
    }

    assertCustom(magic_dfu != BF_BAD);
    return false;
}

void qspi_dfu_process() {
    const uint32_t app_internal_offset = DFU_BANK_0_REGION_START;

    uint8_t pBufQSPI[0x1000]           = {};
    uint8_t pBufInternal[0x1000]       = {};

    uint8_t  attempt                   = 0;
    BootFlag magic_dfu                 = BF_BAD;

    while (1) {
        for (uint32_t app_block_addr = 0; app_block_addr + 0x1000 <= app_length; app_block_addr += 0x1000) {
            qspi_read(pBufQSPI, 0x1000, app_block_addr);
            flash_nrf5x_rewrite_page(app_block_addr + app_internal_offset, pBufQSPI);
        }

        for (uint32_t app_block_addr = 0; app_block_addr + 0x1000 <= app_length; app_block_addr += 0x1000) {
            qspi_read(pBufQSPI, 0x1000, app_block_addr);
            int errc = ecc_checkAppBlock(app_block_addr, app_block_addr + app_internal_offset, pBufQSPI, pBufInternal);
            if (errc == -1) {
                attempt++;
                if (attempt > 3) {
                    break;
                }
            } else {
                magic_dfu = BF_TRIAL_PENDING;
            }
        }
    }

    qspi_read(&pBufQSPI, sizeof(pBufQSPI), QSPI_ADDRESS_DFU_MAGIC_BLOCK);
    memcpy(&pBufQSPI[QSPI_OFFSET_DFU_MAGIC_PRESENT], &magic_dfu, sizeof(magic_dfu));
    qspi_write_4_retry(pBufQSPI, QSPI_ADDRESS_DFU_MAGIC_BLOCK);

    assertCustom(magic_dfu != BF_BAD);

    bootloader_settings_t const *p_bootloader_settings;
    bootloader_util_settings_get(&p_bootloader_settings);

    dfu_update_status_t update_status = {
        .app_crc        = 0,
        .app_size       = app_length,
        .bl_size        = p_bootloader_settings->bl_image_size,
        .sd_image_start = p_bootloader_settings->sd_image_start,
        .sd_size        = p_bootloader_settings->sd_image_size,
        .status_code    = DFU_UPDATE_APP_COMPLETE,
    };

    bootloader_dfu_update_process(update_status);
}