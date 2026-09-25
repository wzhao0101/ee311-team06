/*
 * touch.h
 *
 * Created: 25/09/2026 5:31:57 pm
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

#endif /* TOUCH_H_ */