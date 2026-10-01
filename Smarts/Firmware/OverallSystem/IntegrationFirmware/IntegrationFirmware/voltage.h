/*
 * voltage.h
 *
 * Created: 1/10/2026 1:46:10 pm
 *  Author: byhz780
 */ 


#ifndef VOLTAGE_H_
#define VOLTAGE_H_

#include <stdint.h>

float voltage_adc_to_vvs(uint16_t adc_value);
float voltage_vvs_to_supercap(float vvs);

#endif