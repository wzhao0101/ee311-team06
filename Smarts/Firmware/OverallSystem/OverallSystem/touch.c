/*
 * touch.c
 *
 * Created: 25/09/2026 5:30:43 pm
 *  Author: byhz780
 */ 

#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#include "touch.h"

#define PRESS_THRESHOLD    120
#define RELEASE_THRESHOLD   90
#define TOUCH_TIMEOUT      1000

static uint8_t touch_state = 0;


void touch_init(void)
{
	// PC5 initially input
	DDRC &= ~(1 << PC5);

	// Disable internal pull-up
	PORTC &= ~(1 << PC5);

	// PB2 = debug LED output
	DDRB |= (1 << PB2);

	// LED initially OFF
	PORTB &= ~(1 << PB2);

	// Timer1 normal mode
	TCCR1A = 0;

	// Timer initially stopped
	TCCR1B = 0;

	TCNT1 = 0;
}


uint16_t touch_measure_raw(void)
{
	uint16_t result;

	// -------------------------
	// 1. Discharge touch node
	// -------------------------

	// PC5 = output
	DDRC |= (1 << PC5);

	// Drive LOW
	PORTC &= ~(1 << PC5);

	_delay_us(50);


	// -------------------------
	// 2. Release node
	// -------------------------

	// PC5 = input
	DDRC &= ~(1 << PC5);

	// Keep pull-up disabled
	PORTC &= ~(1 << PC5);


	// -------------------------
	// 3. Start Timer1
	// -------------------------

	TCNT1 = 0;

	// Prescaler = 8
	// 16 MHz / 8 = 2 MHz
	// 1 count = 0.5 us
	TCCR1B = (1 << CS11);


	// -------------------------
	// 4. Wait for PC5 HIGH
	// -------------------------

	while (!(PINC & (1 << PC5)))
	{
		if (TCNT1 >= TOUCH_TIMEOUT)
		{
			TCCR1B = 0;
			return TOUCH_TIMEOUT;
		}
	}


	// -------------------------
	// 5. Save result
	// -------------------------

	result = TCNT1;


	// -------------------------
	// 6. Stop Timer1
	// -------------------------

	TCCR1B = 0;

	return result;
}


uint16_t touch_measure_average(void)
{
	uint32_t total = 0;

	for (uint8_t i = 0; i < 8; i++)
	{
		uint16_t reading = touch_measure_raw();

		if (reading >= TOUCH_TIMEOUT)
		{
			return TOUCH_TIMEOUT;
		}

		total += reading;

		_delay_ms(1);
	}

	return (uint16_t)(total / 8);
}


void touch_update(void)
{
	uint16_t count = touch_measure_average();

	// Ignore invalid measurement
	if (count >= TOUCH_TIMEOUT)
	{
		return;
	}

	// Currently released
	if (touch_state == 0)
	{
		if (count > PRESS_THRESHOLD)
		{
			touch_state = 1;

			// LED ON
			PORTB |= (1 << PB2);
		}
	}

	// Currently pressed
	else
	{
		if (count < RELEASE_THRESHOLD)
		{
			touch_state = 0;

			// LED OFF
			PORTB &= ~(1 << PB2);
		}
	}
}


uint8_t touch_get_state(void)
{
	return touch_state;
}
