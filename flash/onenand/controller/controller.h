#pragma once
#include <stdint.h>

#define O1N_BOOTRAM 0x0000
#define O1N_DATARAM 0x0200
#define O1N_SPARERAM 0x8010
#define O1N_REG_MANUFACTURER_ID 0xF000
#define O1N_REG_DEVICE_ID 0xF001
#define O1N_REG_VERSION_ID 0xF002
#define O1N_REG_DATA_BUFFER_SIZE 0xF003
#define O1N_REG_BOOT_BUFFER_SIZE 0xF004
#define O1N_REG_NUM_BUFFERS 0xF005
#define O1N_REG_TECHNOLOGY 0xF006
#define O1N_REG_START_ADDRESS1 0xF100
#define O1N_REG_START_ADDRESS2 0xF101
#define O1N_REG_START_ADDRESS3 0xF102
#define O1N_REG_START_ADDRESS4 0xF103
#define O1N_REG_START_ADDRESS5 0xF104
#define O1N_REG_START_ADDRESS6 0xF105
#define O1N_REG_START_ADDRESS7 0xF106
#define O1N_REG_START_ADDRESS8 0xF107
#define O1N_REG_START_BUFFER 0xF200
#define O1N_REG_COMMAND 0xF220
#define O1N_REG_SYS_CFG1 0xF221
#define O1N_REG_SYS_CFG2 0xF222
#define O1N_REG_CTRL_STATUS 0xF240
#define O1N_REG_INTERRUPT 0xF241
#define O1N_REG_START_BLOCK_ADDRESS 0xF24C
#define O1N_REG_END_BLOCK_ADDRESS 0xF24D
#define O1N_REG_WP_STATUS 0xF24E
#define O1N_REG_ECC_STATUS 0xFF00
#define O1N_REG_ECC_M0 0xFF01
#define O1N_REG_ECC_S0 0xFF02
#define O1N_REG_ECC_M1 0xFF03
#define O1N_REG_ECC_S1 0xFF04
#define O1N_REG_ECC_M2 0xFF05
#define O1N_REG_ECC_S2 0xFF06
#define O1N_REG_ECC_M3 0xFF07
#define O1N_REG_ECC_S3 0xFF08

#define O1N_CMD_READ 0x00
#define O1N_CMD_READOOB 0x13
#define O1N_CMD_PROG 0x80
#define O1N_CMD_PROGOOB 0x1A
#define O1N_CMD_X2_PROG 0x7D
#define O1N_CMD_X2_CACHE_PROG 0x7F
#define O1N_CMD_UNLOCK 0x23
#define O1N_CMD_LOCK 0x2A
#define O1N_CMD_LOCK_TIGHT 0x2C
#define O1N_CMD_UNLOCK_ALL 0x27
#define O1N_CMD_ERASE 0x94
#define O1N_CMD_MULTIBLOCK_ERASE 0x95
#define O1N_CMD_ERASE_VERIFY 0x71
#define O1N_CMD_RESET 0xF0
#define O1N_CMD_HOT_RESET 0xF3
#define O1N_CMD_OTP_ACCESS 0x65
#define O1N_CMD_READID 0x90
#define O1N_CMD_PI_UPDATE 0x05
#define O1N_CMD_PI_ACCESS 0x66
#define O1N_CMD_RECOVER_LSB 0x05

void OneNAND_Pre_Initialize(DCCMemory *mem, uint32_t offset);
void OneNAND_Ctrl_Reg_Write(DCCMemory *mem, uint16_t reg, uint16_t data);
uint16_t OneNAND_Ctrl_Reg_Read(DCCMemory *mem, uint16_t reg);
void OneNAND_Ctrl_Get_Data(DCCMemory *mem, uint8_t *page_buf, uint8_t *spare_buf, uint32_t page_size, uint32_t spare_size);
