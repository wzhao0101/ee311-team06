/*
 * touch.h
 *
 * Created: 1/10/2026 1:47:48 pm
 *  Author: byhz780
 */ 


#ifndef TOUCH_H_
#define TOUCH_H_

#include <stdint.h>

void touch_init(void);

uint16_t touch_measure_raw(void);
uint16_t touch_measure_average(void);

void touch_update(void);

uint8_t touch_get_state(void);
uint16_t touch_get_count(void);

#endif