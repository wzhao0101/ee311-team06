/*
 * reset.c
 *
 * Created: 14/10/2026 5:56:04 pm
 *  Author: byhz780
 */ 

#include <avr/io.h>
#include <stdint.h>

#include "reset.h"
#include "protection.h"


#define RESET_PULSE_MS 10


static uint8_t fault_seen = 0;
static uint8_t reset_active = 0;

static uint32_t reset_start_time = 0;


/* =========================================================
   INITIALISE RESET OUTPUT
   ========================================================= */

void reset_init(void)
{
    /*
     * PD0 = Switch_Reset
     *
     * Normally leave PD0 as an input / high impedance.
     * External pull-down keeps Switch_Reset LOW.
     */

    DDRD &= ~(1 << PD0);

    // Disable internal pull-up
    PORTD &= ~(1 << PD0);

    fault_seen = 0;
    reset_active = 0;
    reset_start_time = 0;
}


/* =========================================================
   START RESET PULSE
   ========================================================= */

static void reset_start_pulse(uint32_t current_ms)
{
    /*
     * Prepare HIGH first.
     */
    PORTD |= (1 << PD0);

    /*
     * Now make PD0 an output.
     * Switch_Reset becomes HIGH.
     */
    DDRD |= (1 << PD0);

    reset_start_time = current_ms;
    reset_active = 1;
}


/* =========================================================
   END RESET PULSE
   ========================================================= */

static void reset_end_pulse(void)
{
    /*
     * Return PD0 to input / high impedance.
     */
    DDRD &= ~(1 << PD0);

    /*
     * Disable internal pull-up.
     *
     * External pull-down resistor now brings
     * Switch_Reset back LOW.
     */
    PORTD &= ~(1 << PD0);

    reset_active = 0;
}


/* =========================================================
   RESET STATE MACHINE
   ========================================================= */

void reset_update(uint32_t current_ms)
{
    /*
     * First deal with an active reset pulse.
     */
    if (reset_active)
    {
        if ((current_ms - reset_start_time) >= RESET_PULSE_MS)
        {
            reset_end_pulse();
        }

        return;
    }


    /*
     * Read current protection states.
     */

    uint8_t ovp =
        protection_get_ovp();

    uint8_t otp =
        protection_get_otp();

    uint8_t boost_ocp =
        protection_get_boost_ocp();

    uint8_t charger_ocp =
        protection_get_charger_ocp();


    /*
     * If ANY fault is active,
     * remember that a fault occurred.
     */

    if (ovp ||
        otp ||
        boost_ocp ||
        charger_ocp)
    {
        fault_seen = 1;

        return;
    }


    /*
     * At this point:
     *
     * OVP         = 0
     * OTP         = 0
     * BOOST OCP   = 0
     * CHARGER OCP = 0
     *
     * If a fault had previously occurred,
     * generate one reset pulse.
     */

    if (fault_seen)
    {
        reset_start_pulse(current_ms);

        /*
         * Clear fault_seen so another pulse
         * is not generated every loop.
         */
        fault_seen = 0;
    }
}