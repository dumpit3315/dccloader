// Qualcomm MSM7xxx NAND Controller
// OneNAND is handled by the SFLASHC controller (msm7200.c in OneNAND folder)
// https://github.com/chraso/GT-I5500_/blob/master/GT-I5500_OpenSource_Kernel/kernel/drivers/tfsr/PAM/MSM7k/FSR_PAM_MSM7k.c
// https://github.com/mik9/ThunderG-Kernel/blob/7642626bcf53beeeef2632571015a43a6057e037/drivers/mtd/devices/msm_nand.c
#include "../controller.h"
#include "msm7200.h"

#ifndef REGS_START
#define REGS_START 0xa0a00000
#endif

#ifndef REGS_INIT1
#define REGS_INIT1 0xaad400da
#endif

#ifndef REGS_INIT2
#define REGS_INIT2 0x44747c
#endif

static uint32_t nand_config_1;
static uint32_t nand_config_2;

#define MSM7200_AUTOPROBE_CMD (4 << MSM7200_NAND_FLASH_CMD_AUTO_DETECT_DATA_XFR_SIZE.bit_pos) | (1 << MSM7200_NAND_FLASH_CMD_AUTO_DETECT.bit_pos) | (1 << MSM7200_NAND_FLASH_CMD_LAST_PAGE.bit_pos) | (1 << MSM7200_NAND_FLASH_CMD_PAGE_ACC.bit_pos) | (MSM7200_NAND_FLASH_CMD_OP_CMD_PAGE_READ << MSM7200_NAND_FLASH_CMD_OP_CMD.bit_pos) // 0b100 1 1 1 0010 = 0x272

void inline RunCommand(uint32_t cmd) {
    wdog_reset();

    WRITE_U32(REGS_START + MSM7200_REG_FLASH_CMD, cmd);
    WRITE_U32(REGS_START + MSM7200_REG_EXEC_CMD, 1);
    do { wdog_reset(); } while (GET_BIT32(REGS_START + MSM7200_REG_FLASH_STATUS, MSM7200_NAND_FLASH_STATUS_OPER_STATUS));
}

void inline RunReadCommand(uint32_t addr1, uint32_t addr2, uint8_t subsequent) {
    wdog_reset();

    if (!subsequent) {
        WRITE_U32(REGS_START + MSM7200_REG_FLASH_CMD, MSM7200_CMD_PAGE_READ_ALL);
        WRITE_U32(REGS_START + MSM7200_REG_ADDR0, addr1);
        WRITE_U32(REGS_START + MSM7200_REG_ADDR1, addr2);
    }

    WRITE_U32(REGS_START + MSM7200_REG_EXEC_CMD, 1);
    do { wdog_reset(); } while (GET_BIT32(REGS_START + MSM7200_REG_FLASH_STATUS, MSM7200_NAND_FLASH_STATUS_OPER_STATUS));
}

DCC_RETURN NAND_Ctrl_Probe(DCCMemory *mem) {
    wdog_reset();
    mem->type = MEMTYPE_NONE;
    nand_config_1 = REGS_INIT1;
    nand_config_2 = REGS_INIT2;

    WRITE_U32(REGS_START + MSM7200_REG_DEV0_CFG0, nand_config_1);
    WRITE_U32(REGS_START + MSM7200_REG_DEV0_CFG1, nand_config_2);
    WRITE_U32(REGS_START + MSM7200_REG_DEV_CMD_VLD, 0xd);
    WRITE_U32(REGS_START + MSM7200_REG_DEV_CMD0, 0x1080d060);
    WRITE_U32(REGS_START + MSM7200_REG_DEV_CMD1, 0xf00f3000);
    WRITE_U32(REGS_START + MSM7200_REG_DEV_CMD2, 0xf0ff7090);
    WRITE_U32(REGS_START + MSM7200_REG_DEV_CMD3, 0xf0ff7090);
    WRITE_U32(REGS_START + MSM7200_REG_DEV_CMD4, 0x800000);
    WRITE_U32(REGS_START + MSM7200_REG_DEV_CMD5, 0xf30094);
    WRITE_U32(REGS_START + MSM7200_REG_DEV_CMD6, 0x40e0);

    RunCommand(MSM7200_CMD_RESET);
    RunCommand(MSM7200_CMD_RESET_NAND);

    WRITE_U32(REGS_START + MSM7200_REG_ADDR0, 0x0);
    WRITE_U32(REGS_START + MSM7200_REG_ADDR1, 0x0);

    RunCommand(MSM7200_AUTOPROBE_CMD);
    if (GET_BIT32(REGS_START + MSM7200_REG_READ_STATUS, MSM7200_NAND_FLASH_STATUS_OP_ERR)) {
        return DCC_PROBE_ERROR;
    }

    do { wdog_reset(); } while (!GET_BIT32(REGS_START + MSM7200_REG_READ_STATUS, MSM7200_NAND_FLASH_STATUS_AUTO_DETECT_DONE));

    RunCommand(MSM7200_CMD_FETCH_ID);

    uint32_t nand_idcode = READ_U32(REGS_START + MSM7200_REG_READ_ID);
    uint8_t mfr_id = (uint8_t)nand_idcode;
    uint8_t dev_id = (uint8_t)(nand_idcode >> 8);
    uint8_t ext_id = (uint8_t)(nand_idcode >> 24);

    for (int i = 0; flash_ids[i].dev_id; i++) {
        if (dev_id == (uint8_t)flash_ids[i].dev_id) {
            mem->device_id = (uint16_t)dev_id;
            mem->manufacturer = (uint16_t)mfr_id;
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
        mem->page_size = 1 << (10 + (ext_id & 3));

        switch ((ext_id >> 4) & 3) {
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

        mem->device_id |= ext_id << 8;
    }

    BIT_SET_VAR(nand_config_1, MSM7200_NAND_DEV_CFG0_CW_PER_PAGE, (mem->page_size >> 9) - 1);
    BIT_SET_VAR(nand_config_2, MSM7200_NAND_DEV_CFG1_WIDE_FLASH, mem->bit_width >> 4);
    BIT_SET_VAR(nand_config_2, MSM7200_NAND_DEV_CFG1_BAD_BLOCK_IN_SPARE_AREA, mem->page_size <= 0x200);
    BIT_SET_VAR(nand_config_2, MSM7200_NAND_DEV_CFG1_BAD_BLOCK_BYTE_NUM, mem->page_size <= 0x200 ? (mem->bit_width == 16 ? 1 : 6) : 0x1d1);
    BIT_SET_VAR(nand_config_2, MSM7200_NAND_DEV_CFG1_ECC_DISABLE, 0);

    WRITE_U32(REGS_START + MSM7200_REG_DEV0_CFG0, nand_config_1);
    WRITE_U32(REGS_START + MSM7200_REG_DEV0_CFG1, nand_config_2);

    RunCommand(MSM7200_CMD_RESET);
    RunCommand(MSM7200_CMD_RESET_NAND);

    return DCC_OK;
}

DCC_RETURN NAND_Ctrl_Read(DCCMemory *mem, uint8_t *page_buf, uint8_t *spare_buf, uint32_t page) {
    wdog_reset();

    for (int i = 0; i < (mem->page_size >> 9); i++) {
        RunReadCommand((mem->page_size > 0x200 ? page << 16 : page << 8), (mem->page_size > 0x200 ? page >> 16 : page >> 24), i > 0);
        PLAT_MEMCPY(page_buf + (i << 9), (uint8_t *)(REGS_START + MSM7200_REG_FLASH_BUFFER), 0x200);
        PLAT_MEMCPY(spare_buf + (i << 4), (uint8_t *)(REGS_START + MSM7200_REG_FLASH_BUFFER + 0x200), 0x10);
    }

    return DCC_OK;
}