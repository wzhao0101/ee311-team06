/*
 * touch.c
 *
 * Created: 1/10/2026 1:47:30 pm
 *  Author: byhz780
 */ 

#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#include "touch.h"


#define PRESS_THRESHOLD     110
#define RELEASE_THRESHOLD    80
#define TOUCH_TIMEOUT       1000


static uint8_t touch_state = 0;

static volatile uint16_t raw_count = 0;
static volatile uint16_t average_count = 0;


void touch_init(void)
{
	// PC5 starts as input
	DDRC &= ~(1 << PC5);

	// Disable internal pull-up
	PORTC &= ~(1 << PC5);

	// PB2 = LED output
	DDRB |= (1 << PB2);

	// LED initially OFF
	PORTB &= ~(1 << PB2);

	// Timer1 normal mode
	TCCR1A = 0;

	// Timer stopped initially
	TCCR1B = 0;

	TCNT1 = 0;
}


uint16_t touch_measure_raw(void)
{
	uint16_t result;


	// ---------------------------------
	// 1. Discharge touch capacitor
	// ---------------------------------

	DDRC |= (1 << PC5);

	PORTC &= ~(1 << PC5);

	_delay_us(20);


	// ---------------------------------
	// 2. Release touch node
	// ---------------------------------

	DDRC &= ~(1 << PC5);

	PORTC &= ~(1 << PC5);


	// ---------------------------------
	// 3. Start Timer1
	// ---------------------------------

	TCNT1 = 0;

	// Prescaler = 8
	// 16 MHz / 8 = 2 MHz
	// 0.5 us per count
	TCCR1B = (1 << CS11);


	// ---------------------------------
	// 4. Wait for PC5 HIGH
	// ---------------------------------

	while (!(PINC & (1 << PC5)))
	{
		if (TCNT1 >= TOUCH_TIMEOUT)
		{
			TCCR1B = 0;

			return TOUCH_TIMEOUT;
		}
	}


	result = TCNT1;


	// Stop timer
	TCCR1B = 0;


	return result;
}


uint16_t touch_measure_average(void)
{
	uint32_t total = 0;


	for (uint8_t i = 0; i < 8; i++)
	{
		raw_count = touch_measure_raw();


		if (raw_count >= TOUCH_TIMEOUT)
		{
			average_count = TOUCH_TIMEOUT;

			return TOUCH_TIMEOUT;
		}


		total += raw_count;

		_delay_ms(1);
	}


	average_count =
	(uint16_t)(total / 8);


	return average_count;
}


void touch_update(void)
{
	uint16_t count =
	touch_measure_average();


	// Invalid reading
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