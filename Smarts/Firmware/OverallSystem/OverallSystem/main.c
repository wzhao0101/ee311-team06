/*
 * OverallSystem.c
 *
 * Created: 25/09/2026 5:26:05 pm
 * Author : byhz780
 */ 

#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#include "uart.h"
#include "touch.h"
#include "voltage.h"


int main(void)
{
	uart_init();
	touch_init();
	voltage_init();

	while (1)
	{
		// Update touch state
		touch_update();

		// Update voltage protection state
		ovp_update();


		// -------------------------
		// Debug output
		// -------------------------

		uint16_t touch_count = touch_measure_average();
		uint16_t adc_value = voltage_read_average();

		uart_send_string("Touch = ");
		uart_send_uint16(touch_count);

		uart_send_string("    ADC = ");
		uart_send_uint16(adc_value);

		uart_send_string("    OVP = ");

		if (ovp_get_fault())
		{
			uart_send_string("FAULT");
		}
		else
		{
			uart_send_string("SAFE");
		}

		uart_send_string("\r\n");


		_delay_ms(100);
	}
}



