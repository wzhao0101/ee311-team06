/*
 * current.c
 *
 * Created: 1/10/2026 1:46:24 pm
 *  Author: byhz780
 */ 

#include <stdint.h>

#include "current.h"


#define ADC_REF_VOLTAGE       5.0f

#define CURRENT_ZERO_VOLTAGE  2.5f

#define SHUNT_RESISTANCE      0.01f
#define CURRENT_GAIN          100.0f


float current_adc_to_vis(uint16_t adc_value)
{
    return ((float)adc_value * ADC_REF_VOLTAGE) / 1023.0f;
}


float current_vis_to_current(float vis)
{
    /*
     * Vis = Gain * Rshunt * I + 2.5
     *
     * I = (Vis - 2.5) / (Gain * Rshunt)
     *
     * Gain * Rshunt = 100 * 0.01 = 1
     */

    return
        (vis - CURRENT_ZERO_VOLTAGE)
        /
        (CURRENT_GAIN * SHUNT_RESISTANCE);
}