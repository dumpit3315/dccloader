#include "../dcc/plat.h"

#ifdef DCC_TESTING
void wdog_reset(void) {}
#else
void wdog_reset(void) {
    // Reset watchdog (or else, system restarts)
}
#endif