
/*
 * voltage.h
 *
 * Created: 25/09/2026 5:32:16 pm
 *  Author: byhz780
 */ 


#ifndef VOLTAGE_H_
#define VOLTAGE_H_

#include <stdint.h>

void voltage_init(void);

uint16_t voltage_read_adc(void);
uint16_t voltage_read_average(void);

void ovp_update(void);
uint8_t ovp_get_fault(void);

#endif /* VOLTAGE_H_ */