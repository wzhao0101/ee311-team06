/*
 * uart.h
 *
 * Created: 25/09/2026 5:31:33 pm
 *  Author: byhz780
 */ 


#ifndef UART_H_
#define UART_H_

#include <stdint.h>

void uart_init(void);
void uart_send_char(char data);
void uart_send_string(const char *str);
void uart_send_uint16(uint16_t value);

#endif /* UART_H_ */