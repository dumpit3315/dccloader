#include "dcc/dn_dcc_proto.h"
#include "flash/onenand/onenand.h"

Device devices[] = {
    {&onenand_controller, 0x38000000},
    // {&nand_controller, 0x0},
    {0x0, 0x0}
};