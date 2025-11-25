#include "qspi_dfu.h"

#define QSPI_ADDRESS_DFU_ECC 0x000F0000

void compute256(const uint8_t *pData, uint8_t *eccChecksum) {
    uint32_t i;
    uint8_t  column_sum       = 0;
    uint8_t  even_line_code   = 0;
    uint8_t  odd_line_code    = 0;
    uint8_t  even_column_code = 0;
    uint8_t  odd_column_code  = 0;

    for (i = 0; i < 256; i++) {
        column_sum ^= pData[i];
        if ((__builtin_popcount(pData[i]) & 1) == 1) {
            even_line_code ^= (255 - i);
            odd_line_code ^= i;
        }
    }
    for (i = 0; i < 8; i++) {
        if (column_sum & 1) {
            even_column_code ^= (7 - i);
            odd_column_code ^= i;
        }
        column_sum >>= 1;
    }
    eccChecksum[0] = 0;
    eccChecksum[1] = 0;
    eccChecksum[2] = 0;

    for (i = 0; i < 4; i++) {
        eccChecksum[0] <<= 2;
        eccChecksum[1] <<= 2;
        eccChecksum[2] <<= 2;

        if ((odd_line_code & 0b10000000) != 0) {
            eccChecksum[0] |= 2;
        }

        if ((even_line_code & 0b10000000) != 0) {
            eccChecksum[0] |= 1;
        }
        if ((odd_line_code & 0b00001000) != 0) {
            eccChecksum[1] |= 2;
        }

        if ((even_line_code & 0b00001000) != 0) {
            eccChecksum[1] |= 1;
        }
        if ((odd_column_code & 0b00000100) != 0) {
            eccChecksum[2] |= 2;
        }

        if ((even_column_code & 0b00000100) != 0) {
            eccChecksum[2] |= 1;
        }

        odd_line_code <<= 1;
        even_line_code <<= 1;
        odd_column_code <<= 1;
        even_column_code <<= 1;
    }
    eccChecksum[0] = ~eccChecksum[0];
    eccChecksum[1] = ~eccChecksum[1];
    eccChecksum[2] = ~eccChecksum[2];
}

int verify256(uint8_t *pData, uint8_t *eccChecksum) {
    uint8_t comp_code[3];
    uint8_t corr_code[3];
    compute256(pData, comp_code);

    corr_code[0] = comp_code[0] ^ eccChecksum[0];
    corr_code[1] = comp_code[1] ^ eccChecksum[1];
    corr_code[2] = comp_code[2] ^ eccChecksum[2];

    int errc     = (__builtin_popcount(corr_code[0]) + __builtin_popcount(corr_code[1]) + __builtin_popcount(corr_code[2]));

    if (!errc) {
        return 0;
    }
    if (errc == 11) {
        uint8_t byte = corr_code[0] & 0b10000000;
        byte |= (corr_code[0] << 1) & 0b01000000;
        byte |= (corr_code[0] << 2) & 0b00100000;
        byte |= (corr_code[0] << 3) & 0b00010000;

        byte |= (corr_code[1] >> 4) & 0b00001000;
        byte |= (corr_code[1] >> 3) & 0b00000100;
        byte |= (corr_code[1] >> 2) & 0b00000010;
        byte |= (corr_code[1] >> 1) & 0b00000001;

        uint8_t bit = (corr_code[2] >> 5) & 0b00000100;
        bit |= (corr_code[2] >> 4) & 0b00000010;
        bit |= (corr_code[2] >> 3) & 0b00000001;

        pData[byte] ^= (1 << bit);

        return 1;
    }
    return -1;
}

int ecc_checkAppBlock(uint32_t qspiBlockAddress, uint32_t internalBlockAddress, uint8_t *pData, uint8_t *blockr) {
    assertCustom((qspiBlockAddress % 0x1000) == 0);
    assertCustom((internalBlockAddress % 0x1000) == 0);

    uint32_t eccIndex        = qspiBlockAddress / 0x1000;
    uint32_t eccAddress      = QSPI_ADDRESS_DFU_ECC + (eccIndex * 0x40);
    uint32_t eccBlockAddress = eccAddress - (eccAddress % 0x1000);

    qspi_read(blockr, 0x1000, eccBlockAddress);
    memcpy(pData, (void *)internalBlockAddress, 0x1000);
    qspi_read(pData, 0x1000, qspiBlockAddress);

    uint32_t eccChecksumOffset = 0;                            // Offset of ECC shecksum in the loop
    bool     needRewrite       = false;
    for (size_t i = 0; i < 16; i++) {
        int errCode = verify256(&pData[i * 0x100], &blockr[(eccAddress % 0x1000) + eccChecksumOffset]);
        assertCustom(errCode != -1);                           // File is bad

        if (errCode == 1) {
            needRewrite = true;                                // Rewrite fixed block into flash here and now
        }
        eccChecksumOffset += 4;                                // Shift to next 32-bit ECC checksum address
    }
    if (needRewrite) {
        flash_nrf5x_rewrite_page(internalBlockAddress, pData); // Rewrite fixed block into flash here and now
    }
    return 0;
}