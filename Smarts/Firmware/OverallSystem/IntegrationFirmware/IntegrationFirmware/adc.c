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
	// AVcc reference
	ADMUX = (1 << REFS0);

	// Enable ADC
	// Prescaler = 128
	// 16 MHz / 128 = 125 kHz
	ADCSRA = (1 << ADEN)
	| (1 << ADPS2)
	| (1 << ADPS1)
	| (1 << ADPS0);
}


uint16_t adc_read(uint8_t channel)
{
	// Keep AVcc reference, select ADC channel 0-7
	ADMUX = (1 << REFS0) | (channel & 0x07);

	// Dummy conversion after switching channel
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