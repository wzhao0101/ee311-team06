/*
 * current.h
 *
 * Created: 1/10/2026 1:46:37 pm
 *  Author: byhz780
 */ 


#ifndef CURRENT_H_
#define CURRENT_H_

#include <stdint.h>

float current_adc_to_vis(uint16_t adc_value);
float current_vis_to_current(float vis);

#endif