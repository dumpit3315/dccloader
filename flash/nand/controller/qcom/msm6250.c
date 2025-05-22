// Qualcomm MSM625x NAND Controller
#include "../controller.h"
#include "msm6250.h"

#ifndef REGS_START
#define REGS_START 0x64000000
#endif

#ifndef INTERRUPT_WRITE
#define INTERRUPT_WRITE 0x8400024c
#endif

#ifndef INTERRUPT_READ
#define INTERRUPT_READ 0x84000244
#endif

#ifndef INTERRUPT_CLEAR_VALUE
#define INTERRUPT_CLEAR_VALUE 0x6
#endif

#ifndef INTERRUPT_ASSERT_VALUE
#define INTERRUPT_ASSERT_VALUE 0x2
#endif

#ifndef IS_MSM6550
#define IS_MSM6550 0
#endif

static uint32_t nand_config;

void inline RunCommand(uint32_t cmd) {
    wdog_reset();
    
    WRITE_U32(INTERRUPT_WRITE, INTERRUPT_CLEAR_VALUE);
    do { wdog_reset(); } while (READ_U32(INTERRUPT_READ) & INTERRUPT_CLEAR_VALUE);

    WRITE_U32(REGS_START + MSM6250_REG_FLASH_CMD, cmd);
    do { wdog_reset(); } while ((READ_U32(INTERRUPT_READ) & INTERRUPT_ASSERT_VALUE) == 0);
}

DCC_RETURN NAND_Ctrl_Probe(DCCMemory *mem) {
    wdog_reset();
    mem->type = MEMTYPE_NONE;

#if IS_MSM6550
    nand_config = 0x2;
#else
    nand_config = 0x253;
#endif

    WRITE_U32(REGS_START + MSM6250_REG_FLASH_CFG1, nand_config);
#if IS_MSM6550
    WRITE_U32(REGS_START + MSM6250_REG_FLASH_SPARE_DATA, 0x06541463);
#endif

    RunCommand(MSM6250_CMD_FLASH_RESET_NAND);
    RunCommand(MSM6250_CMD_FLASH_ID_FETCH);
    RunCommand(MSM6250_CMD_FLASH_STATUS_CHECK);

    uint8_t mfr_id = (uint8_t)GET_BIT32(REGS_START + MSM6250_REG_FLASH_STATUS, MSM6250_STATUS_NAND_MFRID);
    uint8_t dev_id = (uint8_t)GET_BIT32(REGS_START + MSM6250_REG_FLASH_STATUS, MSM6250_STATUS_NAND_DEVID);

    for (int i = 0; flash_ids[i].dev_id; i++) {
        if (dev_id == (uint8_t)flash_ids[i].dev_id && flash_ids[i].page_size) {
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

#if IS_MSM6550
    BIT_SET_VAR(nand_config, MSM6250_CONFIG_WIDE_NAND, mem->bit_width == 16 ? 1 : 0);
#else
    nand_config = 0x25a;
#endif

    BIT_SET_VAR(nand_config, MSM6250_CONFIG_ECC_DISABLED, 0);
    WRITE_U32(REGS_START + MSM6250_REG_FLASH_CFG1, nand_config);
#if IS_MSM6550
    WRITE_U32(REGS_START + MSM6250_REG_FLASH_SPARE_DATA, mem->bit_width == 16 ? 0x06719C63 : 0x06541463);
#endif

    RunCommand(MSM6250_CMD_FLASH_RESET);
    RunCommand(MSM6250_CMD_FLASH_RESET_NAND);

    return DCC_OK;
}

DCC_RETURN NAND_Ctrl_Read(DCCMemory *mem, uint8_t *page_buf, uint8_t *spare_buf, uint32_t page) {
    wdog_reset();
    
    SET_BIT32(REGS_START + MSM6250_REG_FLASH_ADDR, MSM6250_ADDR_FLASH_PAGE_ADDRESS, page);
    RunCommand(MSM6250_CMD_FLASH_PAGE_READ);

    PLAT_MEMCPY(page_buf, (uint8_t *)(REGS_START + MSM6250_REG_FLASH_BUFFER), 0x200);
    PLAT_MEMCPY(spare_buf, (uint8_t *)(REGS_START + MSM6250_REG_FLASH_BUFFER + 0x200), 0x10);

    return DCC_OK;
}