#ifndef DRVINIT_H
#define DRVINIT_H

#include "keyboard.h"

static inline void init_drivers() {
    init_keyboard_drv();
}

#endif
