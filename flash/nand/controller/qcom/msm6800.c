// Qualcomm MSM62xx (except MSM625x group) NAND Controller
// OneNAND is handled by default onenand controller at 0x40000000 (Boot from OneNAND is supported on XMEM2_CS_N[3])
#include "../controller.h"
#include "msm6800.h"

#ifndef REGS_START
#define REGS_START 0x60000000
#endif

#ifndef INTERRUPT_WRITE
#define INTERRUPT_WRITE 0x80000414
#endif

#ifndef INTERRUPT_READ
#define INTERRUPT_READ 0x80000488
#endif

#ifndef INTERRUPT_CLEAR_VALUE
#define INTERRUPT_CLEAR_VALUE 0x2
#endif

#ifndef INTERRUPT_ASSERT_VALUE
#define INTERRUPT_ASSERT_VALUE 0x2
#endif

static uint32_t nand_config_1;
static uint32_t nand_config_2;

void inline RunCommand(uint32_t cmd) {
    wdog_reset();
    
    WRITE_U32(INTERRUPT_WRITE, INTERRUPT_CLEAR_VALUE);
    do { wdog_reset(); } while (READ_U32(INTERRUPT_READ) & INTERRUPT_CLEAR_VALUE);

    WRITE_U32(REGS_START + MSM6800_REG_FLASH_CMD, cmd);
    do { wdog_reset(); } while ((READ_U32(INTERRUPT_READ) & INTERRUPT_ASSERT_VALUE) == 0);
}

DCC_RETURN NAND_Ctrl_Probe(DCCMemory *mem) {
    wdog_reset();
    mem->type = MEMTYPE_NONE;
    nand_config_1 = 0xa2;
    nand_config_2 = 0x22;
    // nand_config_1 = READ_U32(REGS_START + MSM6800_REG_FLASH_CFG1_FLASH1);
    // nand_config_2 = READ_U32(REGS_START + MSM6800_REG_FLASH_CFG2_FLASH1);

    WRITE_U32(REGS_START + MSM6800_REG_FLASH_CFG1_FLASH1, nand_config_1);
    WRITE_U32(REGS_START + MSM6800_REG_FLASH_CFG2_FLASH1, nand_config_2);

    RunCommand(MSM6800_CMD_FLASH_RESET_NAND);
    if (!GET_BIT32(REGS_START + MSM6800_REG_FLASH_STATUS, MSM6800_STATUS_NAND_AUTOPROBE_DONE)) {
        uint32_t prev_common = READ_U32(REGS_START + MSM6800_REG_FLASH_COMMON_CFG);
        WRITE_U32(REGS_START + MSM6800_REG_FLASH_COMMON_CFG, BIT_SET(0, MSM6800_COMMONCFG_NAND_AUTOPROBE, 1));
        RunCommand(MSM6800_CMD_FLASH_PAGE_READ);
        WRITE_U32(REGS_START + MSM6800_REG_FLASH_COMMON_CFG, prev_common);
    }

    uint32_t page_size_type = GET_BIT32(REGS_START + MSM6800_REG_FLASH_STATUS, MSM6800_STATUS_NAND_AUTOPROBE_ISLARGE);
    uint32_t page_width_type = GET_BIT32(REGS_START + MSM6800_REG_FLASH_STATUS, MSM6800_STATUS_NAND_AUTOPROBE_IS16BIT);

    nand_config_1 = 0xa;
    nand_config_2 = 0x4219442;

    BIT_SET_VAR(nand_config_1, MSM6800_CONFIG1_PAGE_IS_2KB, page_size_type);
    BIT_SET_VAR(nand_config_1, MSM6800_CONFIG1_WIDE_NAND, page_width_type);

    WRITE_U32(REGS_START + MSM6800_REG_FLASH_CFG1_FLASH1, nand_config_1);
    WRITE_U32(REGS_START + MSM6800_REG_FLASH_CFG2_FLASH1, nand_config_2);
    WRITE_U32(REGS_START + MSM6800_REG_FLASH_COMMON_CFG, 0x3);

    RunCommand(MSM6800_CMD_FLASH_ID_FETCH);
    RunCommand(MSM6800_CMD_FLASH_STATUS_CHECK);

    uint8_t mfr_id = (uint8_t)GET_BIT32(REGS_START + MSM6800_REG_FLASH_ID_DATA, MSM6800_FLASH_NAND_MFRID);
    uint8_t dev_id = (uint8_t)GET_BIT32(REGS_START + MSM6800_REG_FLASH_ID_DATA, MSM6800_FLASH_NAND_DEVID);
    uint8_t ext_id = (uint8_t)GET_BIT32(REGS_START + MSM6800_REG_FLASH_ID_DATA, MSM6800_FLASH_NAND_EXTID);

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

    BIT_SET_VAR(nand_config_1, MSM6800_CONFIG1_ECC_DISABLED, 0);
    WRITE_U32(REGS_START + MSM6800_REG_FLASH_CFG1_FLASH1, nand_config_1);

    RunCommand(MSM6800_CMD_FLASH_RESET);
    RunCommand(MSM6800_CMD_FLASH_RESET_NAND);

    return DCC_OK;
}

DCC_RETURN NAND_Ctrl_Read(DCCMemory *mem, uint8_t *page_buf, uint8_t *spare_buf, uint32_t page) {
    wdog_reset();
    
    SET_BIT32(REGS_START + MSM6800_REG_FLASH_ADDR, MSM6800_ADDR_FLASH_PAGE_ADDRESS, page);

    for (int i = 0; i < (mem->page_size >> 9); i++) {
        RunCommand(MSM6800_CMD_FLASH_PAGE_READ);
        PLAT_MEMCPY(page_buf + (i << 9), (uint8_t *)(REGS_START + MSM6800_REG_FLASH_BUFFER), 0x200);
        PLAT_MEMCPY(spare_buf + (i << 4), (uint8_t *)(REGS_START + MSM6800_REG_FLASH_BUFFER + 0x200), 0x10);
    }

    return DCC_OK;
}