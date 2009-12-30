#ifndef CALENDAR_H
#define CALENDAR_H

#include <stdint.h>

#define IS_LEAP_YEAR(y) (((y) % 4 == 0) && (((y) % 100 != 0) || ((y) % 400 == 0)))

uint8_t days_in_month(uint8_t m, uint16_t y);
uint8_t day_of_week(uint16_t y, uint8_t m, uint8_t d);


#endif /* CALENDAR_H */

