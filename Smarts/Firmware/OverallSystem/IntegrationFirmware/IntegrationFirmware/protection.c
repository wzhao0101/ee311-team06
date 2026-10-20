/*
 * protection.c
 *
 * Created: 1/10/2026 1:48:31 pm
 * Author: byhz780
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


/* OVP */

#define OVP_TRIP_ADC     893
#define OVP_RESET_ADC    885


/* OTP */

#define OTP_TRIP_ADC     561
#define OTP_RESET_ADC    575


/* BOOST OCP */

#define BOOST_OCP_TRIP_ADC    1003


/* CHARGER OCP */

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
 * =========================================================
 * LATEST ADC VALUES
 * =========================================================
 */

static uint16_t latest_vvs_adc = 0;
static uint16_t latest_vis_adc = 0;
static uint16_t latest_vts_adc = 0;



/*
 * =========================================================
 * TIMING RESULTS
 * =========================================================
 *
 * Timer1:
 *
 * 16 MHz / 8 = 2 MHz
 *
 * 1 count = 0.5 us
 */

static uint16_t total_time_count = 0;

static uint16_t ovp_time_count = 0;
static uint16_t ocp_time_count = 0;
static uint16_t otp_time_count = 0;



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


    // Clear timing results
    total_time_count = 0;

    ovp_time_count = 0;
    ocp_time_count = 0;
    otp_time_count = 0;
}



/*
 * =========================================================
 * FAST PROTECTION UPDATE
 * =========================================================
 */

void protection_update_fast(void)
{
    /*
     * Individual timing markers
     */

    uint16_t start_count;
    uint16_t vvs_adc_time;
    uint16_t vis_adc_time;
    uint16_t vts_adc_time;

    uint16_t ovp_logic_time;
    uint16_t ocp_logic_time;
    uint16_t otp_logic_time;



    /*
     * =====================================================
     * START OVERALL TIMER
     * =====================================================
     */

    // Stop Timer1 first
    TCCR1B = 0;

    // Normal mode
    TCCR1A = 0;

    // Start measurement from zero
    TCNT1 = 0;

    // Prescaler = 8
    // 16 MHz / 8 = 2 MHz
    // 1 count = 0.5 us
    TCCR1B = (1 << CS11);



    /*
     * =====================================================
     * ADC0 - Vvs
     * =====================================================
     */

    start_count = TCNT1;

    latest_vvs_adc =
        adc_read_fast_average(0);

    vvs_adc_time =
        (uint16_t)(TCNT1 - start_count);



    /*
     * =====================================================
     * ADC1 - Vis
     * =====================================================
     */

    start_count = TCNT1;

    latest_vis_adc =
        adc_read_fast_average(1);

    vis_adc_time =
        (uint16_t)(TCNT1 - start_count);



    /*
     * =====================================================
     * ADC2 - Vts
     * =====================================================
     */

    start_count = TCNT1;

    latest_vts_adc =
        adc_read_fast_average(2);

    vts_adc_time =
        (uint16_t)(TCNT1 - start_count);



    /*
     * =====================================================
     * OVP
     * =====================================================
     */

    start_count = TCNT1;


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


    ovp_logic_time =
        (uint16_t)(TCNT1 - start_count);



    /*
     * =====================================================
     * OTP
     * =====================================================
     */

    start_count = TCNT1;


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


    otp_logic_time =
        (uint16_t)(TCNT1 - start_count);



    /*
     * =====================================================
     * OCP
     * =====================================================
     */

    start_count = TCNT1;


    /*
     * BOOST OCP
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
     * CHARGER OCP
     */

    if (latest_vis_adc <= CHARGER_OCP_TRIP_ADC)
    {
        charger_ocp_fault = 1;
    }
    else
    {
        charger_ocp_fault = 0;
    }


    ocp_logic_time =
        (uint16_t)(TCNT1 - start_count);



    /*
     * =====================================================
     * BOOST FAULT OUTPUT
     * =====================================================
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
     * =====================================================
     * CHARGER FAULT OUTPUT
     * =====================================================
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



    /*
     * =====================================================
     * FINISH OVERALL MEASUREMENT
     * =====================================================
     */

    total_time_count = TCNT1;


    // Stop Timer1
    TCCR1B = 0;



    /*
     * =====================================================
     * INDIVIDUAL PROTECTION TIMES
     * =====================================================
     *
     * These are:
     *
     * sensor acquisition
     * +
     * associated protection decision
     */

    ovp_time_count =
        vvs_adc_time + ovp_logic_time;


    ocp_time_count =
        vis_adc_time + ocp_logic_time;


    otp_time_count =
        vts_adc_time + otp_logic_time;
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



/*
 * =========================================================
 * TIMING GETTERS
 * =========================================================
 */

uint16_t protection_get_total_time_count(void)
{
    return total_time_count;
}


uint16_t protection_get_ovp_time_count(void)
{
    return ovp_time_count;
}


uint16_t protection_get_ocp_time_count(void)
{
    return ocp_time_count;
}


uint16_t protection_get_otp_time_count(void)
{
    return otp_time_count;
}