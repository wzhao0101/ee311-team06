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

#define OVP_ADC_THRESHOLD 900

static uint8_t ovp_fault = 0;


void voltage_init(void)
{
	// PC0 = input
	DDRC &= ~(1 << PC0);

	// Disable digital pull-up
	PORTC &= ~(1 << PC0);

	// AVcc reference
	// ADC0 selected
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
	// Keep AVcc reference
	// Select ADC0
	ADMUX = (ADMUX & 0xF0);

	// Start conversion
	ADCSRA |= (1 << ADSC);

	// Wait until finished
	while (ADCSRA & (1 << ADSC));

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


void ovp_update(void)
{
	uint16_t adc_value = voltage_read_average();

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

