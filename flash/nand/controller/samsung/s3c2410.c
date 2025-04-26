/* Samsung S3C2410 NAND Controller */
#include "../controller.h"
#include "s3c2410.h"

#ifndef NAND_BASE
#define NAND_BASE 0x4e000000
#endif

#ifndef NAND_SYS_TYPE
#define NAND_SYS_TYPE SYSTYPE_S3C2410
#endif

static uint8_t bit_width;

void inline NAND_Ctrl_Command_Write(uint8_t cmd) {
    // Write command routines
#if NAND_SYS_TYPE == SYSTYPE_S3C2410
    WRITE_U16(NAND_BASE + S3C2410_NFCMD, (uint16_t)cmd);
#elif NAND_SYS_TYPE == SYSTYPE_S3C2440 || NAND_SYS_TYPE == SYSTYPE_S3C2412
    WRITE_U16(NAND_BASE + S3C2440_2412_NFCMD, (uint16_t)cmd);
#endif
}

void inline NAND_Ctrl_Address_Write(uint8_t addr) {
    // Write address routines
#if NAND_SYS_TYPE == SYSTYPE_S3C2410
    WRITE_U16(NAND_BASE + S3C2410_NFADDR, (uint16_t)addr);
#elif NAND_SYS_TYPE == SYSTYPE_S3C2440 || NAND_SYS_TYPE == SYSTYPE_S3C2412
    WRITE_U16(NAND_BASE + S3C2440_2412_NFADDR, (uint16_t)addr);
#endif
}

uint16_t inline NAND_Ctrl_Data_Read() {
    // Data read routines
#if NAND_SYS_TYPE == SYSTYPE_S3C2410
    return bit_width == 16 ? READ_U16(NAND_BASE + S3C2410_NFDATA) : READ_U8(NAND_BASE + S3C2410_NFDATA);
#elif NAND_SYS_TYPE == SYSTYPE_S3C2440 || NAND_SYS_TYPE == SYSTYPE_S3C2412
    return bit_width == 16 ? READ_U16(NAND_BASE + S3C2440_2412_NFDATA) : READ_U8(NAND_BASE + S3C2440_2412_NFDATA);
#endif
}

void inline NAND_Ctrl_Wait_Ready() {
    // Busy assert routines
#if NAND_SYS_TYPE == SYSTYPE_S3C2410
    do { wdog_reset(); } while (!GET_BIT8(NAND_BASE + S3C2410_NFSTAT, S3C2410_NFSTAT_BUSY));
#elif NAND_SYS_TYPE == SYSTYPE_S3C2440
    do { wdog_reset(); } while (!GET_BIT8(NAND_BASE + S3C2440_NFSTAT, S3C2440_NFSTAT_READY));
#elif NAND_SYS_TYPE == SYSTYPE_S3C2412
    do { wdog_reset(); } while (!GET_BIT8(NAND_BASE + S3C2412_NFSTAT, S3C2412_NFSTAT_READY));
#endif
}

uint32_t inline NAND_Ctrl_Check_Status() {
    return 1;
}

DCC_RETURN NAND_Ctrl_Probe(DCCMemory *mem) {
    wdog_reset();
    mem->type = MEMTYPE_NONE;

    uint32_t NFCONF = 0;
#if NAND_SYS_TYPE == SYSTYPE_S3C2410
    BIT_SET_VAR(NFCONF, S3C2410_NFCONF_EN, 1);
    BIT_SET_VAR(NFCONF, S3C2410_NFCONF_INITECC, 0);
    BIT_SET_VAR(NFCONF, S3C2410_NFCONF_NFCE, 0);
    BIT_SET_VAR(NFCONF, S3C2410_NFCONF_TACLS, 3);
    BIT_SET_VAR(NFCONF, S3C2410_NFCONF_TWRPH0, 5);
    BIT_SET_VAR(NFCONF, S3C2410_NFCONF_TWRPH1, 3);
#elif NAND_SYS_TYPE == SYSTYPE_S3C2440
    uint32_t NFCONT = 0;

    BIT_SET_VAR(NFCONT, S3C2440_2412_NFCONT_ENABLE, 1);
    BIT_SET_VAR(NFCONT, S3C2440_NFCONT_NFCE, 0);
    BIT_SET_VAR(NFCONT, S3C2440_NFCONT_INITECC, 0);
    BIT_SET_VAR(NFCONF, S3C2440_2412_NFCONF_TACLS, 3);
    BIT_SET_VAR(NFCONF, S3C2440_2412_NFCONF_TWRPH0, 7);
    BIT_SET_VAR(NFCONF, S3C2440_2412_NFCONF_TWRPH1, 7);

    WRITE_U32(NAND_BASE + S3C2440_2412_NFCONT, NFCONT);
#elif NAND_SYS_TYPE == SYSTYPE_S3C2412
    uint32_t NFCONT = 0;

    BIT_SET_VAR(NFCONT, S3C2440_2412_NFCONT_ENABLE, 1);
    BIT_SET_VAR(NFCONT, S3C2412_NFCONT_nFCE0, 0);
    BIT_SET_VAR(NFCONT, S3C2412_NFCONT_nFCE1, 0);
    BIT_SET_VAR(NFCONT, S3C2412_NFCONT_INIT_MAIN_ECC, 0);
    BIT_SET_VAR(NFCONT, S3C2412_NFCONT_INIT_SECONDARY_ECC, 0);
    BIT_SET_VAR(NFCONF, S3C2410_NFCONF_TACLS, 3);
    BIT_SET_VAR(NFCONF, S3C2410_NFCONF_TWRPH0, 7);
    BIT_SET_VAR(NFCONF, S3C2410_NFCONF_TWRPH1, 7);

    WRITE_U32(NAND_BASE + S3C2440_2412_NFCONT, NFCONT);
#endif
    WRITE_U32(NAND_BASE + S3C2410_2440_2412_NFCONF, NFCONF);

    NAND_Ctrl_Command_Write(NAND_CMD_RESET);
    NAND_Ctrl_Command_Write(NAND_CMD_READID);
    NAND_Ctrl_Address_Write(0x0);

    uint16_t mfr_id = NAND_Ctrl_Data_Read();
    uint16_t dev_id = NAND_Ctrl_Data_Read();

    for (int i = 0; flash_ids[i].dev_id; i++) {
        if (dev_id == (uint16_t)flash_ids[i].dev_id) {
            mem->device_id = dev_id;
            mem->manufacturer = mfr_id;
            mem->bit_width = flash_ids[i].bits;
            mem->block_size = flash_ids[i].block_size;
            mem->page_size = flash_ids[i].page_size;
            mem->size = flash_ids[i].chip_size;
            mem->type = MEMTYPE_NAND;
            break;
        }
    }

    if (mem->type != MEMTYPE_NAND) return DCC_PROBE_ERROR;

    if (mem->page_size == 0) {
#if NAND_SYS_TYPE == SYSTYPE_S3C2440 || NAND_SYS_TYPE == SYSTYPE_S3C2412
        NAND_Ctrl_Data_Read();
        uint8_t extra_id = (uint8_t)NAND_Ctrl_Data_Read();

        mem->page_size = 1 << (10 + (extra_id & 3));

        switch ((extra_id >> 4) & 3) {
            case 0:
                mem->block_size = 64 << 10;
                break;
            case 1:
                mem->block_size = 128 << 10;
                break;
            case 2:
                mem->block_size = 256 << 10;
                break;
            case 3:
                mem->block_size = 512 << 10;
                break;
        }

        mem->device_id |= extra_id << 8;
#else
        // Not supported by HW
        mem->type = MEMTYPE_NONE;
        return DCC_PROBE_ERROR;
#endif
    }

    bit_width = mem->bit_width;

#if NAND_SYS_TYPE == SYSTYPE_S3C2410
    BIT_SET_VAR(NFCONF, S3C2410_NFCONF_INITECC, 1);
#elif NAND_SYS_TYPE == SYSTYPE_S3C2440
    BIT_SET_VAR(NFCONF, S3C2440_2412_NFCONF_BUSWIDTH, bit_width == 16 ? 1 : 0);
    BIT_SET_VAR(NFCONT, S3C2440_NFCONT_INITECC, 1);
    WRITE_U32(NAND_BASE + S3C2440_2412_NFCONT, NFCONT);
#elif NAND_SYS_TYPE == SYSTYPE_S3C2412
    BIT_SET_VAR(NFCONF, S3C2440_2412_NFCONF_BUSWIDTH, bit_width == 16 ? 1 : 0);
    BIT_SET_VAR(NFCONT, S3C2412_NFCONT_INIT_MAIN_ECC, 1);
    WRITE_U32(NAND_BASE + S3C2440_2412_NFCONT, NFCONT);
#endif
    WRITE_U32(NAND_BASE + S3C2410_2440_2412_NFCONF, NFCONF);

    NAND_Ctrl_Command_Write(NAND_CMD_RESET);
    return DCC_OK;
}

DCC_RETURN NAND_Ctrl_Read(DCCMemory *mem, uint8_t *page_buf, uint8_t *spare_buf, uint32_t page) {
    wdog_reset();
    
    if (mem->page_size <= 512) {
        NAND_Ctrl_Command_Write(NAND_CMD_READ0);
        NAND_Ctrl_Address_Write(0);
        NAND_Ctrl_Address_Write(page);
        NAND_Ctrl_Address_Write(page >> 8);
        if (mem->size > 0x02000000) NAND_Ctrl_Address_Write(page >> 16);

        NAND_Ctrl_Wait_Ready();
        if (!NAND_Ctrl_Check_Status()) return DCC_READ_ERROR;

        for (int i = 0; i < 0x100; i++) {
            wdog_reset();
            if (mem->bit_width == 16) {
                ((uint16_t *)(page_buf))[i] = NAND_Ctrl_Data_Read();
            } else {
                page_buf[i] = (uint8_t)NAND_Ctrl_Data_Read();
            }
        }

        if (mem->bit_width == 8) {
            NAND_Ctrl_Command_Write(NAND_CMD_READ1);
            NAND_Ctrl_Address_Write(0);
            NAND_Ctrl_Address_Write(page);
            NAND_Ctrl_Address_Write(page >> 8);
            if (mem->size > 0x02000000) NAND_Ctrl_Address_Write(page >> 16);
    
            NAND_Ctrl_Wait_Ready();
            if (!NAND_Ctrl_Check_Status()) return DCC_READ_ERROR;
    
            for (int i = 0; i < 0x100; i++) {
                wdog_reset();
                page_buf[i + 0x100] = (uint8_t)NAND_Ctrl_Data_Read();
            }
        }

        NAND_Ctrl_Command_Write(NAND_CMD_READOOB);
        NAND_Ctrl_Address_Write(0);
        NAND_Ctrl_Address_Write(page);
        NAND_Ctrl_Address_Write(page >> 8);
        if (mem->size > 0x02000000) NAND_Ctrl_Address_Write(page >> 16);

        NAND_Ctrl_Wait_Ready();
        if (!NAND_Ctrl_Check_Status()) return DCC_READ_ERROR;

        for (int i = 0; i < (0x10 >> (mem->bit_width >> 4)); i++) {
            wdog_reset();
            if (mem->bit_width == 16) {
                ((uint16_t *)(spare_buf))[i] = NAND_Ctrl_Data_Read();
            } else {
                spare_buf[i] = (uint8_t)NAND_Ctrl_Data_Read();
            }
        }
    } else {
        NAND_Ctrl_Command_Write(NAND_CMD_READ0);
        NAND_Ctrl_Address_Write(0);
        NAND_Ctrl_Address_Write(0);
        NAND_Ctrl_Address_Write(page);
        NAND_Ctrl_Address_Write(page >> 8);
        if (mem->size > 0x08000000) NAND_Ctrl_Address_Write(page >> 16);

        NAND_Ctrl_Command_Write(NAND_CMD_READSTART);
        NAND_Ctrl_Wait_Ready();
        if (!NAND_Ctrl_Check_Status()) return DCC_READ_ERROR;

        for (int i = 0; i < (mem->page_size >> (mem->bit_width >> 4)); i++) {
            wdog_reset();
            if (mem->bit_width == 16) {
                ((uint16_t *)(page_buf))[i] = NAND_Ctrl_Data_Read();
            } else {
                page_buf[i] = (uint8_t)NAND_Ctrl_Data_Read();
            }
        }

        for (int i = 0; i < ((mem->page_size >> 5) >> (mem->bit_width >> 4)); i++) {
            wdog_reset();
            if (mem->bit_width == 16) {
                ((uint16_t *)(spare_buf))[i] = NAND_Ctrl_Data_Read();
            } else {
                spare_buf[i] = (uint8_t)NAND_Ctrl_Data_Read();
            }
        }
    }

    return DCC_OK;
}