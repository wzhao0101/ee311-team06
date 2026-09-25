#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdlib.h>

#define BAUD 9600
#define UBRR_VALUE ((F_CPU/(16UL*BAUD))-1)

#define PRESS_THRESHOLD    110
#define RELEASE_THRESHOLD  80
#define TOUCH_TIMEOUT      1000

uint8_t touch_state = 0;   // 0 = released, 1 = pressed
volatile uint16_t raw_count = 0;
volatile uint16_t average_count = 0;


void uart_init(void){
	//Set baud rate
	UBRR0H = (uint8_t)(UBRR_VALUE>>8);
	UBRR0L = (uint8_t)(UBRR_VALUE);
	
	//Enable transmitter
	UCSR0B = (1 << TXEN0);
	
	//8 data bits, 1 stop bit, no parity
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void uart_send_char(char data)
{
	// Wait until transmit buffer is empty
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

	// Convert number to text
	itoa(value, buffer, 10);

	uart_send_string(buffer);
}

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

	// Clear timer count
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

	// Make sure internal pull-up stays OFF
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

uint16_t touch_measure_average(void){
	uint32_t total = 0;
	
	for (uint8_t i = 0; i < 8; i++){
		raw_count = touch_measure_raw();
		
		//If measurement failed
		if(raw_count >= TOUCH_TIMEOUT){
			return TOUCH_TIMEOUT;
		}
		total += raw_count;
		_delay_ms(1);
	}
	
	average_count = (uint16_t)(total/8);
	return average_count;
}


void touch_update(void)
{
	uint16_t count = touch_measure_average();
	
	uart_send_string("count = ");
	uart_send_uint16(count);
	uart_send_string("\r\n");


    //Invalid measurement -> don't change state
	if(count >= TOUCH_TIMEOUT){
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


int main(void)
{
	touch_init();
	uart_init();

	while (1)
	{
		touch_update();

		_delay_ms(20);
	}
}