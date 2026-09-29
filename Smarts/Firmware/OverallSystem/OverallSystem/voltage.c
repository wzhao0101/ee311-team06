/*
 * voltage.c
 *
 * Created: 25/09/2026 5:31:08 pm
 *  Author: byhz780
 */ 

#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#include "voltage.h"


#define ADC_REF_VOLTAGE    5.0f

/*
 * Your measured calibration:
 *
 * Vsupercap     Measured Vvs
 * 2.50 V   ->   0.39 V
 * 3.00 V   ->   2.40 V
 * 3.30 V   ->   3.59 V
 * 3.50 V   ->   4.39 V
 *
 * Best-fit equation:
 *
 * Vvs = 3.9987 * Vsupercap - 9.6034
 *
 * Therefore:
 *
 * Vsupercap = (Vvs + 9.6034) / 3.9987
 */


// 3.5 V supercap gives approximately 4.39 V at Vvs
// 4.39 / 5 * 1023 ? 898
#define OVP_ADC_THRESHOLD  898


static uint8_t ovp_fault = 0;


/* =========================================================
   INITIALISE ADC
   ========================================================= */

void voltage_init(void)
{
    // PC0 / ADC0 as input
    DDRC &= ~(1 << PC0);

    // Disable internal pull-up on PC0
    PORTC &= ~(1 << PC0);

    // AVcc reference, initially ADC0
    ADMUX = (1 << REFS0);

    // Enable ADC
    // Prescaler = 128
    // 16 MHz / 128 = 125 kHz ADC clock
    ADCSRA = (1 << ADEN)
           | (1 << ADPS2)
           | (1 << ADPS1)
           | (1 << ADPS0);
}


/* =========================================================
   READ ADC0
   ========================================================= */

uint16_t voltage_read_adc(void)
{
    // AVcc reference, ADC0 selected
    ADMUX = (1 << REFS0);

    /*
     * Dummy conversion because ADC channels are being switched
     * between Vvs and Vts elsewhere in the program.
     */
    ADCSRA |= (1 << ADSC);

    while (ADCSRA & (1 << ADSC))
    {
    }


    // Actual conversion
    ADCSRA |= (1 << ADSC);

    while (ADCSRA & (1 << ADSC))
    {
    }

    return ADC;
}


/* =========================================================
   AVERAGE ADC0
   ========================================================= */

uint16_t voltage_read_average(void)
{
    uint32_t total = 0;

    for (uint8_t i = 0; i < 8; i++)
    {
        total += voltage_read_adc();

        _delay_ms(1);
    }

    return (uint16_t)(total / 8);
}


/* =========================================================
   ADC COUNT -> Vvs
   ========================================================= */

float voltage_convert_adc_to_vvs(uint16_t adc_value)
{
    return ((float)adc_value * ADC_REF_VOLTAGE) / 1023.0f;
}


/* =========================================================
   Vvs -> ACTUAL SUPERCAP VOLTAGE
   ========================================================= */

float voltage_convert_vvs_to_supercap(float vvs)
{
    /*
     * Calibrated using DMM measurements:
     *
     * Vvs = 3.9987 * Vsupercap - 9.6034
     *
     * Rearranged:
     *
     * Vsupercap = (Vvs + 9.6034) / 3.9987
     */

    return (vvs + 9.6034f) / 3.9987f;
}


/* =========================================================
   OVER-VOLTAGE PROTECTION
   ========================================================= */

void ovp_update(void)
{
    uint16_t adc_value = voltage_read_average();

    /*
     * Measured:
     *
     * Vsupercap = 3.50 V
     * Vvs        = 4.39 V
     *
     * ADC approximately 898.
     *
     * Therefore trip when Vvs reaches approximately
     * the level corresponding to 3.5 V supercap voltage.
     */

    if (adc_value >= OVP_ADC_THRESHOLD)
    {
        ovp_fault = 1;
    }
    else
    {
        ovp_fault = 0;
    }
}


/* =========================================================
   GET OVP STATE
   ========================================================= */

uint8_t ovp_get_fault(void)
{
    return ovp_fault;
}


