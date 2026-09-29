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

// Around Vvs = 4.40 V
// Corresponds to Vsupercap ? 3.5 V
#define OVP_ADC_THRESHOLD  900

static uint8_t ovp_fault = 0;


void voltage_init(void)
{
    // PC0 / ADC0 as input
    DDRC &= ~(1 << PC0);

    // Disable internal pull-up
    PORTC &= ~(1 << PC0);

    // AVcc reference, ADC0 selected
    ADMUX = (1 << REFS0);

    // Enable ADC
    // Prescaler = 128
    // 16 MHz / 128 = 125 kHz ADC clock
    ADCSRA = (1 << ADEN)
           | (1 << ADPS2)
           | (1 << ADPS1)
           | (1 << ADPS0);
}


uint16_t voltage_read_adc(void)
{
    // AVcc reference, select ADC0
    ADMUX = (1 << REFS0);

    // Dummy conversion after ADC channel switching
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


float voltage_convert_adc_to_vvs(uint16_t adc_value)
{
    return ((float)adc_value * ADC_REF_VOLTAGE) / 1023.0f;
}


float voltage_convert_vvs_to_supercap(float vvs)
{
    /*
     * Your conditioning equation:
     *
     * Vvs =
     * (1 + 47/12)
     * (47/(10+47))
     * Vsupercap
     * - 2.5(47/12)
     *
     * Approximately:
     *
     * Vvs = 4.0541 * Vsupercap - 9.7917
     *
     * Therefore:
     *
     * Vsupercap = (Vvs + 9.7917) / 4.0541
     */

    return (vvs + 9.7917f) / 4.0541f;
}


void ovp_update(void)
{
    uint16_t adc_value = voltage_read_average();

    /*
     * Higher supercap voltage -> higher Vvs.
     *
     * Around:
     *
     * Vsupercap = 3.5 V
     * Vvs        = 4.40 V
     * ADC        ? 900
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


uint8_t ovp_get_fault(void)
{
    return ovp_fault;
}

