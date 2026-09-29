/*
 * temperature.h
 *
 * Created: 29/09/2026 6:08:40 pm
 *  Author: byhz780
 */ 


#ifndef TEMPERATURE_H_
#define TEMPERATURE_H_

#include <stdint.h>

void temperature_init(void);

uint16_t temperature_read_adc(void);
uint16_t temperature_read_average(void);

float temperature_convert_adc_to_vts(uint16_t adc_value);
float temperature_convert_vts_to_celsius(float vts);

void otp_update(void);
uint8_t otp_get_fault(void);


#endif /* TEMPERATURE_H_ */