#pragma once

/* Standard type*/
#define BCM_NAND_CMD    0x08000000
#define BCM_NAND_ADDR   0x08000004
#define BCM_NAND_DATA   0x08000008
#define BCM_NAND_STATUS 0x08090000

/* AXI type */
#define BCM_AXI_CMD_RESET   0x020007f8  
#define BCM_AXI_CMD_READID  0x02200480
#define BCM_AXI_READ_ID     0x02080000
#define BCM_AXI_READ_BUF    0x02298000
#define BCM_AXI_LOCK        0x0809001c
#define BCM_AXI_ADDR1       0x02800000
#define BCM_AXI_ADDR2       0x02b18000

/* Unknown class */
#define BCM_AXI_BIT         0x08880008
#define BCM_AXI_FLAGS       0x08880010