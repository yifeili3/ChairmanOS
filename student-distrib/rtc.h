#ifndef _RTC_H
#define _RTC_H

#include "types.h"

/* the function to init rtc */
int32_t init_rtc();

/* the rtc handler function */
void do_rtc_handler();

/* open rtc function */
int32_t open_rtc(const uint8_t* filename);

/* read rtc function */
int32_t read_rtc(int32_t fd, void* buf, int32_t count);

/* write rtc function */
int32_t write_rtc(int32_t fd, const void* buf, int32_t count);

/* close rtc function */
int32_t close_rtc(int32_t fd);

#endif /* _RTC_H */
