/*
 * protection.c
 *
 * Created: 29/09/2026 7:13:09 pm
 *  Author: byhz780
 */ 

#include <avr/io.h>

#include "voltage.h"
#include "temperature.h"
#include "protection.h"


void protection_init(void)
{
    // PC3 = CHARGER_FAULT_FW
    DDRC |= (1 << PC3);

    // PC4 = BOOST_FAULT_FW
    DDRC |= (1 << PC4);

    // Initially no fault
    PORTC &= ~(1 << PC3);
    PORTC &= ~(1 << PC4);
}


void protection_update(void)
{
    // Update individual protection states
    ovp_update();
    otp_update();

    /*
     * If either OVP OR OTP is active:
     *
     * CHARGER_FAULT_FW = HIGH
     * BOOST_FAULT_FW   = HIGH
     */
    if (ovp_get_fault() || otp_get_fault())
    {
        PORTC |= (1 << PC3);   // CHARGER_FAULT_FW HIGH
        PORTC |= (1 << PC4);   // BOOST_FAULT_FW HIGH
    }
    else
    {
        PORTC &= ~(1 << PC3);  // CHARGER_FAULT_FW LOW
        PORTC &= ~(1 << PC4);  // BOOST_FAULT_FW LOW
    }
}

