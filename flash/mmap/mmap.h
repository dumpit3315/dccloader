#pragma once
#include "dcc/dn_dcc_proto.h"

#define MEMDUMP_MFR 0x80
#define MEMDUMP_DEVID 0xd4
#define MEMDUMP_MAX_SIZE 0x02000000

extern Driver memdump;