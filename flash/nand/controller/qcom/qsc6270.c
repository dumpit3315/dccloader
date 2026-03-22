#define REGS_START 0x60008000
#define REGS_INIT1 0xaad4001a
#define REGS_INIT2 0x44747e
// OneNAND is handled by default onenand controller at 0x38000000 (Must not set NAND_BUS_ENA at EBI2_CFG; 0x80028000, Corresponds to EBI2_CS0_N)
// for NAND, EBI2_CS1_N is always connected and NAND_BUS_ENA is set (by default, this is on)

#include "msm7200.c"