// Qualcomm MSM7xxx OneNAND through SFLASH Interface
// Queueing looks something like this:
// Low 16-bit is processed first, then the high 16-bit (ADDR for OneNAND address and GENP_REG0 for OneNAND data)

#include "../../onenand.h"
#include "msm7200.h"
#include "../controller.h"
#include <stdint.h>
#include "dcc/dn_dcc_proto.h"

#ifndef REGS_START
#define REGS_START 0xa0a00000
#endif

#define MSM_NAND_SFTRNSTYPE_DATXS       0x0
#define MSM_NAND_SFTRNSTYPE_CMDXS       0x1

#define MSM_NAND_SFMODE_BURST           0x0
#define MSM_NAND_SFMODE_ASYNC           0x1

#define MSM_NAND_SFCMD_ABORT            0x1
#define MSM_NAND_SFCMD_REGRD            0x2
#define MSM_NAND_SFCMD_REGWR            0x3
#define MSM_NAND_SFCMD_INTLO            0x4
#define MSM_NAND_SFCMD_INTHI            0x5
#define MSM_NAND_SFCMD_DATRD            0x6
#define MSM_NAND_SFCMD_DATWR            0x7

// ASYNC_READ_MASK = Mode = ASYNC, TRNSTYPE = DATA
// ASYNC_WRITE_MASK = Mode = ASYNC, TRNSTYPE = CMD

static uint16_t onld_addr[10];
static uint16_t onld_data[10];
static uint8_t onld_cmds;

#define SFLASH_CMD(num_words, offset_val, delta_val, transfer_type, mode, opcode) \
	((num_words << 20) | (offset_val << 12) | (delta_val << 6) | (transfer_type << 5) | (mode << 4) | opcode)

void inline SFLASHC_Execute(int WaitInterrupt) {
    WRITE_U32(REGS_START + MSM7200_REG_SFLASHC_EXEC_CMD, 1);
    do { wdog_reset(); } while (GET_BIT32(REGS_START + MSM7200_REG_SFLASHC_STATUS, MSM7200_SFLASHC_OPER_STATUS));
    if (WaitInterrupt) do { wdog_reset(); } while (!GET_BIT32(REGS_START + MSM7200_REG_SFLASHC_STATUS, MSM7200_SFLASHC_DEV_INTERRUPT));
}

int OneNAND_Ctrl_Wait_Ready(DCCMemory *mem, uint16_t flag) {
    // Busy assert routines
    do { wdog_reset(); } while (!GET_BIT32(REGS_START + MSM7200_REG_SFLASHC_STATUS, MSM7200_SFLASHC_DEV_INTERRUPT));
    return 1;
}

void OneNAND_Pre_Initialize(DCCMemory *mem, uint32_t offset) {
    // Initialize routines
    mem->base_offset = offset;
    mem->page_size = 0x800;

    WRITE_U32(REGS_START + MSM7200_REG_DEV0_CFG0, 0xaad4001a);
    WRITE_U32(REGS_START + MSM7200_REG_DEV0_CFG1, 0x2101bd);
    WRITE_U32(REGS_START + MSM7200_REG_DEV_CMD_VLD, 0xd);
    WRITE_U32(REGS_START + MSM7200_REG_SFLASHC_BURST_CFG, 0x20100327);
    WRITE_U32(REGS_START + MSM7200_REG_XFR_STEP1, 0x47804780);
    WRITE_U32(REGS_START + MSM7200_REG_XFR_STEP2, 0x39003a0);
    WRITE_U32(REGS_START + MSM7200_REG_XFR_STEP3, 0x3b008a8);
    WRITE_U32(REGS_START + MSM7200_REG_XFR_STEP4, 0x9b488a0);
    WRITE_U32(REGS_START + MSM7200_REG_XFR_STEP5, 0x89a2c420);
    WRITE_U32(REGS_START + MSM7200_REG_XFR_STEP6, 0xc420c020);
    WRITE_U32(REGS_START + MSM7200_REG_XFR_STEP7, 0xc020c020);

    OneNAND_Ctrl_Reg_Write(mem, O1N_REG_COMMAND, O1N_CMD_HOT_RESET, 0);
    OneNAND_Ctrl_Reg_Write(mem, O1N_REG_SYS_CFG1, 0x40e0, 1);
}

void OneNAND_Ctrl_Reg_Write(DCCMemory *mem, uint16_t reg, uint16_t data, uint8_t wait_interrupt) {
    // Write register routines
    WRITE_U32(REGS_START + MSM7200_REG_ADDR0, reg);
    WRITE_U32(REGS_START + MSM7200_REG_GENP_REG0, data);
    WRITE_U32(REGS_START + MSM7200_REG_SFLASHC_CMD, SFLASH_CMD(1, 0, 0, MSM_NAND_SFTRNSTYPE_CMDXS, MSM_NAND_SFMODE_BURST, MSM_NAND_SFCMD_REGWR));
    SFLASHC_Execute(wait_interrupt);
}

uint16_t OneNAND_Ctrl_Reg_Read(DCCMemory *mem, uint16_t reg) {
#if 0
    // Read register routines
    WRITE_U32(REGS_START + MSM7200_REG_ADDR0, reg);
    WRITE_U32(REGS_START + MSM7200_REG_SFLASHC_CMD, SFLASH_CMD(1, 0, 0, MSM_NAND_SFTRNSTYPE_CMDXS, MSM_NAND_SFMODE_BURST, MSM_NAND_SFCMD_REGRD));
    SFLASHC_Execute(1);

    return (uint16_t)READ_U32(REGS_START + MSM7200_REG_GENP_REG0);
#else
    // Alternate read register routines (RIFF)
    WRITE_U32(REGS_START + MSM7200_REG_MACRO1_REG, reg);
    WRITE_U32(REGS_START + MSM7200_REG_SFLASHC_CMD, SFLASH_CMD(1, 0, 0, MSM_NAND_SFTRNSTYPE_DATXS, MSM_NAND_SFMODE_BURST, MSM_NAND_SFCMD_DATRD));
    SFLASHC_Execute(1);

    return READ_U16(REGS_START + MSM7200_REG_FLASH_BUFFER);
#endif
}

void OneNAND_Ctrl_Reg_Write_Queue(DCCMemory *mem, uint16_t reg, uint16_t data) {
    onld_addr[onld_cmds] = reg;
    onld_data[onld_cmds++] = data;
}

const uint16_t address_tbls[4] = {MSM7200_REG_ADDR0, MSM7200_REG_ADDR1, MSM7200_REG_ADDR2, MSM7200_REG_ADDR3};
const uint16_t data_tbls[4] = {MSM7200_REG_GENP_REG0, MSM7200_REG_GENP_REG1, MSM7200_REG_GENP_REG2, MSM7200_REG_GENP_REG3};

int OneNAND_Ctrl_Execute_Queue(uint8_t wait_interrupt) {
    for (int t = 0; t < 4; t++) {
        WRITE_U32(REGS_START + address_tbls[t], onld_addr[t << 1] | (onld_addr[(t << 1) + 1] << 0x10));
        WRITE_U32(REGS_START + data_tbls[t], onld_data[t << 1] | (onld_data[(t << 1) + 1] << 0x10));
    }

    WRITE_U32(REGS_START + MSM7200_REG_SFLASHC_CMD, SFLASH_CMD(onld_cmds, 0, 0, MSM_NAND_SFTRNSTYPE_CMDXS, MSM_NAND_SFMODE_BURST, MSM_NAND_SFCMD_REGWR));
    SFLASHC_Execute(wait_interrupt);

    memset(onld_addr, 0x00, sizeof(onld_addr));
    memset(onld_data, 0x00, sizeof(onld_data));
    onld_cmds = 0;

    return 1;
}

void SFLASHC_Nand2Buf(uint16_t offset, uint8_t *data, uint16_t size) {
    uint16_t buf_offset = 0;
    
    while (size > 0) {
        uint32_t read_size = size > 512 ? 512 : size;

        WRITE_U32(REGS_START + MSM7200_REG_MACRO1_REG, offset);
        WRITE_U32(REGS_START + MSM7200_REG_SFLASHC_CMD, SFLASH_CMD(read_size >> 1, 0, 0, MSM_NAND_SFTRNSTYPE_DATXS, MSM_NAND_SFMODE_BURST, MSM_NAND_SFCMD_DATRD));

        SFLASHC_Execute(1);
        PLAT_MEMCPY(data + buf_offset, (uint8_t *)(REGS_START + MSM7200_REG_FLASH_BUFFER), read_size);
        
        size -= read_size;
        buf_offset += read_size;
        offset += read_size >> 1;
    }
}

void OneNAND_Ctrl_Get_Data(DCCMemory *mem, uint8_t *page_buf, uint8_t *spare_buf, uint32_t page_size, uint32_t spare_size) {
    SFLASHC_Nand2Buf(O1N_DATARAM, page_buf, page_size);
    SFLASHC_Nand2Buf(O1N_SPARERAM, spare_buf, spare_size);
}

