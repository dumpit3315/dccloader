#include "dcc/dn_dcc_proto.h"
#include "flash/cfi/cfi.h"
#include "flash/nand/nand.h"

const Device devices[] = {
    {&nor_cfi_controller, 0x0},
    {&nand_controller, 0x0},
    // {&nand_controller, 0x0},
    {0x0, 0x0}
};