/*
 * uart.c
 *
 * Created: 1/10/2026 1:45:16 pm
 *  Author: byhz780
 */ 

#define F_CPU 16000000UL

#include <avr/io.h>
#include <stdint.h>
#include <stdlib.h>

#include "uart.h"

#define BAUD 9600
#define UBRR_VALUE ((F_CPU / (16UL * BAUD)) - 1)


void uart_init(void)
{
	// Set baud rate
	UBRR0H = (uint8_t)(UBRR_VALUE >> 8);
	UBRR0L = (uint8_t)UBRR_VALUE;

	// Enable transmitter
	UCSR0B = (1 << TXEN0);

	// 8 data bits, 1 stop bit, no parity
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}


void uart_send_char(char data)
{
	while (!(UCSR0A & (1 << UDRE0)))
	{
	}

	UDR0 = data;
}


void uart_send_string(const char *str)
{
	while (*str)
	{
		uart_send_char(*str);
		str++;
	}
}


void uart_send_uint16(uint16_t value)
{
	char buffer[8];

	itoa(value, buffer, 10);

	uart_send_string(buffer);
}


void uart_send_float(float value, uint8_t decimal_places)
{
	char buffer[16];

	dtostrf(value, 1, decimal_places, buffer);

	uart_send_string(buffer);
}