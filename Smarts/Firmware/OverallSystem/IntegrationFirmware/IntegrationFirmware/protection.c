/*
 * protection.c
 *
 * Created: 1/10/2026 1:48:31 pm
 *  Author: byhz780
 */ 

#include <avr/io.h>
#include <stdint.h>

#include "adc.h"
#include "protection.h"


/*
 * =========================================================
 * FIRMWARE PROTECTION THRESHOLDS
 * =========================================================
 */


/*
 * OVP
 *
 * Target firmware trip:
 * Vsupercap = 3.50 V
 *
 * Measured Vvs at 3.50 V:
 * approximately 4.39 V
 *
 * ADC count:
 * 4.39 / 5.0 * 1023 ? 898
 *
 * Use hysteresis so OVP does not chatter around 3.5 V.
 */

#define OVP_TRIP_ADC     893
#define OVP_RESET_ADC    885


/*
 * OTP
 *
 * Firmware trip target:
 * 65 C
 *
 * Measured:
 * 65 C -> Vts ? 2.74 V
 *
 * ADC:
 * 2.74 / 5.0 * 1023 ? 561
 *
 * Higher temperature -> lower Vts -> lower ADC count.
 */

#define OTP_TRIP_ADC     561

/*
 * Reset threshold is slightly higher,
 * meaning the system must cool down before OTP clears.
 *
 * This value can be calibrated later.
 */

#define OTP_RESET_ADC    575


/*
 * BOOST OCP
 *
 * Temporary threshold based on:
 *
 * Vis = 2.5 + I
 *
 * Boost FW threshold = +2.4 A
 *
 * Vis = 4.9 V
 *
 * ADC ? 1003
 *
 * We can update this later when you provide
 * the final current-sensing information.
 */

#define BOOST_OCP_TRIP_ADC    1003


/*
 * CHARGER OCP
 *
 * Temporary threshold based on:
 *
 * Charger FW threshold = -0.4 A
 *
 * Vis = 2.5 - 0.4
 *     = 2.1 V
 *
 * ADC ? 430
 */

#define CHARGER_OCP_TRIP_ADC  430



/*
 * =========================================================
 * FAULT STATES
 * =========================================================
 */

static uint8_t ovp_fault = 0;
static uint8_t otp_fault = 0;

static uint8_t boost_ocp_fault = 0;
static uint8_t charger_ocp_fault = 0;


/*
 * Store the latest averaged ADC values.
 *
 * These values are also used by main.c
 * for UART diagnostics.
 */

static uint16_t latest_vvs_adc = 0;
static uint16_t latest_vis_adc = 0;
static uint16_t latest_vts_adc = 0;



/*
 * =========================================================
 * INITIALISATION
 * =========================================================
 */

void protection_init(void)
{
    /*
     * PC3 = CHARGER_FAULT_FW
     * PC4 = BOOST_FAULT_FW
     */

    DDRC |= (1 << PC3);
    DDRC |= (1 << PC4);


    // Start with both fault outputs LOW
    PORTC &= ~(1 << PC3);
    PORTC &= ~(1 << PC4);
}



/*
 * =========================================================
 * FAST PROTECTION UPDATE
 * =========================================================
 */

void protection_update_fast(void)
{
    /*
     * -----------------------------------------------------
     * READ SENSOR CHANNELS
     * -----------------------------------------------------
     *
     * ADC0 = Vvs
     * ADC1 = Vis
     * ADC2 = Vts
     *
     * Each uses the 4-sample fast average.
     *
     * No intentional delay is used here.
     */

    latest_vvs_adc =
        adc_read_fast_average(0);

    latest_vis_adc =
        adc_read_fast_average(1);

    latest_vts_adc =
        adc_read_fast_average(2);



    /*
     * -----------------------------------------------------
     * OVP
     * -----------------------------------------------------
     *
     * Hysteresis:
     *
     * If currently SAFE:
     *     trip when ADC >= 898
     *
     * If currently in FAULT:
     *     remain faulted until ADC <= 890
     */

    if (ovp_fault == 0)
    {
        if (latest_vvs_adc >= OVP_TRIP_ADC)
        {
            ovp_fault = 1;
        }
    }
    else
    {
        if (latest_vvs_adc <= OVP_RESET_ADC)
        {
            ovp_fault = 0;
        }
    }



    /*
     * -----------------------------------------------------
     * OTP
     * -----------------------------------------------------
     *
     * Higher temperature gives lower Vts.
     *
     * Trip:
     *     ADC <= 561
     *
     * Reset:
     *     ADC >= 575
     */

    if (otp_fault == 0)
    {
        if (latest_vts_adc <= OTP_TRIP_ADC)
        {
            otp_fault = 1;
        }
    }
    else
    {
        if (latest_vts_adc >= OTP_RESET_ADC)
        {
            otp_fault = 0;
        }
    }



    /*
     * -----------------------------------------------------
     * BOOST OCP
     * -----------------------------------------------------
     *
     * For now:
     *
     * +2.4 A -> Vis ? 4.9 V
     */

    if (latest_vis_adc >= BOOST_OCP_TRIP_ADC)
    {
        boost_ocp_fault = 1;
    }
    else
    {
        boost_ocp_fault = 0;
    }



    /*
     * -----------------------------------------------------
     * CHARGER OCP
     * -----------------------------------------------------
     *
     * For now:
     *
     * -0.4 A -> Vis ? 2.1 V
     */

    if (latest_vis_adc <= CHARGER_OCP_TRIP_ADC)
    {
        charger_ocp_fault = 1;
    }
    else
    {
        charger_ocp_fault = 0;
    }



    /*
     * -----------------------------------------------------
     * BOOST FAULT OUTPUT
     * -----------------------------------------------------
     *
     * BOOST_FAULT_FW goes HIGH if:
     *
     * OVP
     * OR
     * OTP
     * OR
     * BOOST OCP
     */

    if (
        ovp_fault
        ||
        otp_fault
        ||
        boost_ocp_fault
       )
    {
        PORTC |= (1 << PC4);
    }
    else
    {
        PORTC &= ~(1 << PC4);
    }



    /*
     * -----------------------------------------------------
     * CHARGER FAULT OUTPUT
     * -----------------------------------------------------
     *
     * CHARGER_FAULT_FW goes HIGH if:
     *
     * OVP
     * OR
     * OTP
     * OR
     * CHARGER OCP
     */

    if (
        ovp_fault
        ||
        otp_fault
        ||
        charger_ocp_fault
       )
    {
        PORTC |= (1 << PC3);
    }
    else
    {
        PORTC &= ~(1 << PC3);
    }
}



/*
 * =========================================================
 * FAULT GETTERS
 * =========================================================
 */

uint8_t protection_get_ovp(void)
{
    return ovp_fault;
}


uint8_t protection_get_otp(void)
{
    return otp_fault;
}


uint8_t protection_get_boost_ocp(void)
{
    return boost_ocp_fault;
}


uint8_t protection_get_charger_ocp(void)
{
    return charger_ocp_fault;
}



/*
 * =========================================================
 * ADC GETTERS
 * =========================================================
 */

uint16_t protection_get_vvs_adc(void)
{
    return latest_vvs_adc;
}


uint16_t protection_get_vis_adc(void)
{
    return latest_vis_adc;
}


uint16_t protection_get_vts_adc(void)
{
    return latest_vts_adc;
}