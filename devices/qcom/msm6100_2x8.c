#include "dcc/dn_dcc_proto.h"
#include "flash/cfi/cfi.h"

const Device devices[] = {
    {&nor_cfi_controller, 0x0},
    {&nor_cfi_controller, 0x08000000},
    // {&nand_controller, 0x0},
    {0x0, 0x0}
};