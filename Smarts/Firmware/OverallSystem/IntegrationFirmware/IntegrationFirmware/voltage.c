/*
 * voltage.c
 *
 * Created: 1/10/2026 1:45:59 pm
 *  Author: byhz780
 */ 

#include <stdint.h>

#include "voltage.h"


#define ADC_REF_VOLTAGE 5.0f


/*
 * =========================================================
 * ADC VALUE -> Vvs
 * =========================================================
 *
 * ADC is 10-bit:
 *
 *      Vvs = ADC / 1023 * Vref
 *
 * Currently assuming AVcc = 5.0 V.
 *
 * Later, replace 5.0 V with the actual measured AVcc
 * if necessary for better calibration.
 */

float voltage_adc_to_vvs(uint16_t adc_value)
{
    return ((float)adc_value * ADC_REF_VOLTAGE) / 1023.0f;
}


/*
 * =========================================================
 * Vvs -> SUPERCAPACITOR VOLTAGE
 * =========================================================
 *
 * DMM calibration results:
 *
 * Vsupercap      Measured Vvs
 *
 * 2.50 V    ->   0.39 V
 * 3.00 V    ->   2.40 V
 * 3.30 V    ->   3.59 V
 * 3.50 V    ->   4.39 V
 *
 *
 * Best-fit relationship:
 *
 *      Vvs = 3.9987 * Vsupercap - 9.6034
 *
 *
 * Rearranging:
 *
 *      Vsupercap =
 *      (Vvs + 9.6034) / 3.9987
 */

float voltage_vvs_to_supercap(float vvs)
{
    return (vvs + 9.6034f) / 3.9987f;
}