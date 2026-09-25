#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdlib.h>

#define BAUD 9600
#define UBRR_VALUE ((F_CPU / (16UL * BAUD)) - 1)

/* =========================
   UART
   ========================= */

void uart_init(void)
{
    UBRR0H = (uint8_t)(UBRR_VALUE >> 8);
    UBRR0L = (uint8_t)UBRR_VALUE;

    // Enable transmitter
    UCSR0B = (1 << TXEN0);

    // 8 data bits, 1 stop bit, no parity
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void uart_send_char(char data)
{
    while (!(UCSR0A & (1 << UDRE0)));

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

void uart_send_voltage(uint16_t value){
	uint32_t millivolts;
	
	millivolts = ((uint32_t)value *5000UL)/1023;
	
	uart_send_uint16(millivolts/1000);
	uart_send_char('.');
	
	uint16_t remainder = millivolts % 1000;
	
	if (remainder < 100)
	    uart_send_char('0');
	
	if (remainder < 10)
	    uart_send_char('0');
		
	uart_send_uint16(remainder);
	
	uart_send_string(" V");
}


/* =========================
   ADC
   ========================= */

void adc_init(void)
{
    // AVcc = ADC reference voltage
    // ADC0 selected initially
    ADMUX = (1 << REFS0);

    // Enable ADC
    // Prescaler = 128
    //
    // 16 MHz / 128 = 125 kHz ADC clock
    ADCSRA = (1 << ADEN)
           | (1 << ADPS2)
           | (1 << ADPS1)
           | (1 << ADPS0);
}


/* =========================
   READ ADC0
   ========================= */

uint16_t adc_read(void)
{
    // Make sure ADC0 is selected
    ADMUX &= 0xF0;

    // Start conversion
    ADCSRA |= (1 << ADSC);

    // Wait until conversion finishes
    while (ADCSRA & (1 << ADSC));

    // Return 10-bit ADC result
    return ADC;
}


/* =========================
   MAIN
   ========================= */

int main(void)
{
    uart_init();
    adc_init();

    uint16_t adc_value;

    while (1)
    {
        adc_value = adc_read();

        uart_send_string("ADC = ");
        uart_send_uint16(adc_value);
		
		uart_send_string("Voltage = ");
		uart_send_voltage(adc_value);

        uart_send_string("\r\n");

        _delay_ms(200);
    }
}