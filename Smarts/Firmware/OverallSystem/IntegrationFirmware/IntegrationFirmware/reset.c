/*
 * reset.c
 *
 * Touch-controlled SR latch reset
 */

#include <avr/io.h>
#include <stdint.h>

#include "reset.h"
#include "touch.h"


void reset_init(void)
{
    /*
     * PD0 = Switch_Reset / SR latch R
     *
     * Drive it directly as an output.
     */

    DDRD |= (1 << PD0);

    // Normally LOW
    PORTD &= ~(1 << PD0);
}


void reset_update(uint32_t current_ms)
{
    // current_ms is no longer needed
    (void)current_ms;


    /*
     * Touch detected:
     * R = 1
     */

    if (touch_get_state())
    {
        PORTD |= (1 << PD0);
    }

    /*
     * No touch:
     * R = 0
     */

    else
    {
        PORTD &= ~(1 << PD0);
    }
}