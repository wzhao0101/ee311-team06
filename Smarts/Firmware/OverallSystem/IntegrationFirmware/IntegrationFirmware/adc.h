/*
 * adc.h
 *
 * Created: 1/10/2026 1:44:36 pm
 *  Author: byhz780
 */ 


#ifndef ADC_H_
#define ADC_H_

#include <stdint.h>

void adc_init(void);
uint16_t adc_read(uint8_t channel);

#endif