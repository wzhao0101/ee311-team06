/*
 * uart.h
 *
 * Created: 1/10/2026 1:45:30 pm
 *  Author: byhz780
 */ 


#ifndef UART_H_
#define UART_H_

#include <stdint.h>

void uart_init(void);

void uart_send_char(char data);
void uart_send_string(const char *str);

void uart_send_uint16(uint16_t value);
void uart_send_float(float value, uint8_t decimal_places);

#endif