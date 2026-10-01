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
 * ADC reference = 5.0 V
 */


/*
 * OVP:
 *
 * Vsupercap = 3.5 V
 * measured Vvs ? 4.39 V
 *
 * ADC = 4.39 / 5 * 1023
 *     ? 898
 */
#define OVP_ADC_THRESHOLD          898


/*
 * OTP firmware threshold = 65 C
 *
 * 65 C measured Vts ? 2.74 V
 *
 * ADC = 2.74 / 5 * 1023
 *     ? 561
 *
 * Higher temperature -> LOWER ADC.
 */
#define OTP_ADC_THRESHOLD          561


/*
 * Current:
 *
 * Vis = I + 2.5
 *
 * Boost FW OCP = +2.4 A
 *
 * Vis = 4.9 V
 *
 * ADC ? 1003
 */
#define BOOST_OCP_ADC_THRESHOLD    1003


/*
 * Charger FW OCP = -0.4 A
 *
 * Vis = 2.1 V
 *
 * ADC ? 430
 */
#define CHARGER_OCP_ADC_THRESHOLD  430


static uint8_t ovp_fault = 0;
static uint8_t otp_fault = 0;

static uint8_t boost_ocp_fault = 0;
static uint8_t charger_ocp_fault = 0;


static uint16_t latest_vvs_adc = 0;
static uint16_t latest_vis_adc = 0;
static uint16_t latest_vts_adc = 0;


void protection_init(void)
{
    // PC3 = CHARGER_FAULT_FW
    DDRC |= (1 << PC3);

    // PC4 = BOOST_FAULT_FW
    DDRC |= (1 << PC4);


    // Initially no firmware fault
    PORTC &= ~(1 << PC3);
    PORTC &= ~(1 << PC4);
}


void protection_update_fast(void)
{
    // -------------------------
    // FAST SENSOR READS
    // -------------------------

    latest_vvs_adc = adc_read(0);    // PC0
    latest_vis_adc = adc_read(1);    // PC1
    latest_vts_adc = adc_read(2);    // PC2


    // -------------------------
    // OVP
    // -------------------------

    if (latest_vvs_adc >= OVP_ADC_THRESHOLD)
    {
        ovp_fault = 1;
    }
    else
    {
        ovp_fault = 0;
    }


    // -------------------------
    // OTP
    // -------------------------

    if (latest_vts_adc <= OTP_ADC_THRESHOLD)
    {
        otp_fault = 1;
    }
    else
    {
        otp_fault = 0;
    }


    // -------------------------
    // BOOST OCP
    // -------------------------

    if (latest_vis_adc >= BOOST_OCP_ADC_THRESHOLD)
    {
        boost_ocp_fault = 1;
    }
    else
    {
        boost_ocp_fault = 0;
    }


    // -------------------------
    // CHARGER OCP
    // -------------------------

    if (latest_vis_adc <= CHARGER_OCP_ADC_THRESHOLD)
    {
        charger_ocp_fault = 1;
    }
    else
    {
        charger_ocp_fault = 0;
    }


    // -------------------------
    // BOOST FAULT OUTPUT
    // -------------------------

    if (ovp_fault ||
        otp_fault ||
        boost_ocp_fault)
    {
        PORTC |= (1 << PC4);
    }
    else
    {
        PORTC &= ~(1 << PC4);
    }


    // -------------------------
    // CHARGER FAULT OUTPUT
    // -------------------------

    if (ovp_fault ||
        otp_fault ||
        charger_ocp_fault)
    {
        PORTC |= (1 << PC3);
    }
    else
    {
        PORTC &= ~(1 << PC3);
    }
}


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