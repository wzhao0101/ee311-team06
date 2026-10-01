/*
 * temperature.h
 *
 * Created: 1/10/2026 1:47:07 pm
 *  Author: byhz780
 */ 


#ifndef TEMPERATURE_H_
#define TEMPERATURE_H_

#include <stdint.h>

float temperature_adc_to_vts(uint16_t adc_value);
float temperature_vts_to_celsius(float vts);

#endif