#ifndef PIT_H
#define PIT_H

#include "types.h"

/* function to handle pit interrupt */
extern void do_pit_handler();

/* function to initialize pit */
extern void init_pit(uint32_t channelnum, uint32_t freq);

#endif
