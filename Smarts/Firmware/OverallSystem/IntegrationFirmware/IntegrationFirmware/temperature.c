/*
 * temperature.c
 *
 * Created: 1/10/2026 1:46:53 pm
 *  Author: byhz780
 */ 

#include <stdint.h>

#include "temperature.h"


#define ADC_REF_VOLTAGE 5.0f


float temperature_adc_to_vts(uint16_t adc_value)
{
    return ((float)adc_value * ADC_REF_VOLTAGE) / 1023.0f;
}


float temperature_vts_to_celsius(float vts)
{
    float temperature;

    /*
     * Board calibration:
     *
     * 25 C -> 4.37 V
     * 65 C -> 2.74 V
     * 70 C -> 2.51 V
     * 85 C -> 1.78 V
     */

    if (vts >= 2.74f)
    {
        temperature =
            25.0f +
            (4.37f - vts)
            * 40.0f
            / (4.37f - 2.74f);
    }
    else if (vts >= 2.51f)
    {
        temperature =
            65.0f +
            (2.74f - vts)
            * 5.0f
            / (2.74f - 2.51f);
    }
    else
    {
        temperature =
            70.0f +
            (2.51f - vts)
            * 15.0f
            / (2.51f - 1.78f);
    }

    return temperature;
}