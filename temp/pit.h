#ifndef _PIT_H
#define _PIT_H

int32_t init_pit(int32_t channel_num, int32_t freq);
int32_t pit_handler();

#endif