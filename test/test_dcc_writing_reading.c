#include "dcc/dn_dcc_proto.h"
#include <stdio.h>

int main(void) {
    printf("DCC READ OUT: 0x%08X\n", DN_Packet_DCC_Read());
    DN_Packet_DCC_Send(0x12345678);
    return 0;
}