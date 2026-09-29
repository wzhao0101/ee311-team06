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
#include "temperature.h"
#include "protection.h"


int main(void)
{
	uart_init();

	voltage_init();
	temperature_init();
	touch_init();
	protection_init();

	while (1)
	{
		// Update OVP / OTP and fault outputs
		protection_update();


		// -------------------------
		// TOUCH
		// -------------------------

		uint16_t touch_count =
		touch_measure_average();


		// -------------------------
		// VOLTAGE
		// -------------------------

		uint16_t voltage_adc =
		voltage_read_average();

		float vvs =
		voltage_convert_adc_to_vvs(voltage_adc);

		float v_supercap =
		voltage_convert_vvs_to_supercap(vvs);


		// -------------------------
		// TEMPERATURE
		// -------------------------

		uint16_t temp_adc =
		temperature_read_average();

		float vts =
		temperature_convert_adc_to_vts(temp_adc);

		float temperature =
		temperature_convert_vts_to_celsius(vts);


		// -------------------------
		// UART
		// -------------------------

		uart_send_string("Vvs = ");
		uart_send_float(vvs, 3);
		uart_send_string(" V");

		uart_send_string("  |  Vts = ");
		uart_send_float(vts, 3);
		uart_send_string(" V");

		uart_send_string("  |  Vsupercap = ");
		uart_send_float(v_supercap, 3);
		uart_send_string(" V");

		uart_send_string("  |  Temp = ");
		uart_send_float(temperature, 1);
		uart_send_string(" C");

		uart_send_string("  |  Touch = ");
		uart_send_uint16(touch_count);

		uart_send_string("\r\n");


		_delay_ms(250);
	}
}









