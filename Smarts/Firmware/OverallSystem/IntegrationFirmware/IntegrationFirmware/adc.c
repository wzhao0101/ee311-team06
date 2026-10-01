/*
 * adc.c
 *
 * Created: 1/10/2026 1:44:18 pm
 *  Author: byhz780
 */ 

#define F_CPU 16000000UL

#include <avr/io.h>
#include <stdint.h>

#include "adc.h"


void adc_init(void)
{
    // AVcc used as ADC reference
    ADMUX = (1 << REFS0);

    /*
     * Enable ADC
     *
     * Prescaler = 128
     *
     * 16 MHz / 128 = 125 kHz ADC clock
     */
    ADCSRA = (1 << ADEN)
           | (1 << ADPS2)
           | (1 << ADPS1)
           | (1 << ADPS0);
}


uint16_t adc_read(uint8_t channel)
{
    /*
     * Keep AVcc as reference
     * and select ADC0 - ADC7.
     */
    ADMUX = (1 << REFS0)
          | (channel & 0x07);


    /*
     * Dummy conversion.
     *
     * Because we switch between:
     *
     * ADC0 = Vvs
     * ADC1 = Vis
     * ADC2 = Vts
     *
     * discard the first conversion after changing channel.
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


uint16_t adc_read_fast_average(uint8_t channel)
{
    uint32_t total = 0;


    /*
     * Take 8 samples.
     *
     * No _delay_ms() here because this function
     * is used by the fast protection path.
     */

    for (uint8_t i = 0; i < 8; i++)
    {
        total += adc_read(channel);
    }


    return (uint16_t)(total / 8);
}