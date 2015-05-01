#ifndef EP_STORE_H
#define EP_STORE_H

#include "ep_ds.h"

/* Access routines */
void ep_store_init(void);

#ifndef SD_SETTINGS_FILE
#define SD_SETTINGS_FILE      "settings.dat"
#endif

#endif
