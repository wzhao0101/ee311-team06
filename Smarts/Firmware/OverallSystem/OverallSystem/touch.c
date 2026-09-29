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


#define PRESS_THRESHOLD    110
#define RELEASE_THRESHOLD   80
#define TOUCH_TIMEOUT      1000


static uint8_t touch_state = 0;

static volatile uint16_t raw_count = 0;
static volatile uint16_t average_count = 0;


void touch_init(void)
{
	// PC5 starts as input
	DDRC &= ~(1 << PC5);

	// Disable internal pull-up on PC5
	PORTC &= ~(1 << PC5);

	// PB2 = LED output
	DDRB |= (1 << PB2);

	// LED initially OFF
	PORTB &= ~(1 << PB2);

	// Timer1 normal mode
	TCCR1A = 0;

	// Timer initially stopped
	TCCR1B = 0;

	// Clear timer
	TCNT1 = 0;
}


uint16_t touch_measure_raw(void)
{
	uint16_t result;


	// -----------------------------
	// 1. DISCHARGE TOUCH CAPACITOR
	// -----------------------------

	// PC5 becomes output
	DDRC |= (1 << PC5);

	// Drive PC5 LOW
	PORTC &= ~(1 << PC5);

	// Allow capacitor to discharge through R36
	_delay_us(20);


	// -----------------------------
	// 2. RELEASE TOUCH NODE
	// -----------------------------

	// PC5 becomes input
	DDRC &= ~(1 << PC5);

	// Internal pull-up stays OFF
	PORTC &= ~(1 << PC5);


	// -----------------------------
	// 3. START TIMER
	// -----------------------------

	TCNT1 = 0;

	// Timer1 prescaler = 8
	// 16 MHz / 8 = 2 MHz
	// 1 timer count = 0.5 us
	TCCR1B = (1 << CS11);


	// -----------------------------
	// 4. WAIT FOR PC5 TO GO HIGH
	// -----------------------------

	while (!(PINC & (1 << PC5)))
	{
		if (TCNT1 >= TOUCH_TIMEOUT)
		{
			TCCR1B = 0;

			return TOUCH_TIMEOUT;
		}
	}


	// -----------------------------
	// 5. STORE RESULT
	// -----------------------------

	result = TCNT1;


	// -----------------------------
	// 6. STOP TIMER
	// -----------------------------

	TCCR1B = 0;

	return result;
}


uint16_t touch_measure_average(void)
{
	uint32_t total = 0;

	for (uint8_t i = 0; i < 8; i++)
	{
		raw_count = touch_measure_raw();

		// Invalid measurement
		if (raw_count >= TOUCH_TIMEOUT)
		{
			average_count = TOUCH_TIMEOUT;

			return TOUCH_TIMEOUT;
		}

		total += raw_count;

		_delay_ms(1);
	}

	average_count = (uint16_t)(total / 8);

	return average_count;
}


void touch_update(void)
{
	uint16_t count = touch_measure_average();


	// Invalid measurement -> don't change state
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


uint16_t touch_get_count(void)
{
	return average_count;
}

