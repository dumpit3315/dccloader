#pragma once

#include "dcc/dn_dcc_proto.h"
#include "flash/cfi/cfi.h"
#include "flash/nand/nand.h"
#include "flash/onenand/onenand.h"
#include "flash/superand/superand.h"

static Device devices[] = {
    {&nor_cfi_controller, 0x0},
    {&nor_cfi_controller, 0x12000000},
    // {&nand_controller, 0x0},
    {0x0, 0x0}
};