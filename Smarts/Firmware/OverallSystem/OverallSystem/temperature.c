/*
 * temperature.c
 *
 * Created: 29/09/2026 7:12:49 pm
 *  Author: byhz780
 */ 

#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#include "temperature.h"

#define ADC_REF_VOLTAGE   5.0f

// 70 C corresponds to approximately 2.51 V
#define OTP_ADC_THRESHOLD 514

// Thermistor parameter
#define BETA              4300.0f

static uint8_t otp_fault = 0;


void temperature_init(void)
{
    // PC2 / ADC2 as input
    DDRC &= ~(1 << PC2);

    // Disable pull-up
    PORTC &= ~(1 << PC2);
}


uint16_t temperature_read_adc(void)
{
    // AVcc reference, select ADC2
    ADMUX = (1 << REFS0) | 2;

    // Dummy conversion after switching ADC channel
    ADCSRA |= (1 << ADSC);

    while (ADCSRA & (1 << ADSC))
    {
    }

    // Actual conversion
    ADCSRA |= (1 << ADSC);

    while (ADCSRA & (1 << ADSC))
    {
    }

    return ADC;
}


uint16_t temperature_read_average(void)
{
    uint32_t total = 0;

    for (uint8_t i = 0; i < 8; i++)
    {
        total += temperature_read_adc();
        _delay_ms(1);
    }

    return (uint16_t)(total / 8);
}


float temperature_convert_adc_to_vts(uint16_t adc_value)
{
    return ((float)adc_value * ADC_REF_VOLTAGE) / 1023.0f;
}


float temperature_convert_vts_to_celsius(float vts)
{
    float temperature;

    /*
     * Measured calibration points:
     *
     * 25 C -> 4.37 V
     * 65 C -> 2.74 V
     * 70 C -> 2.51 V
     * 85 C -> 1.78 V
     *
     * Linear interpolation is used between points.
     */

    if (vts >= 2.74f)
    {
        // 25 C to 65 C
        temperature =
            25.0f +
            (4.37f - vts)
            * (65.0f - 25.0f)
            / (4.37f - 2.74f);
    }
    else if (vts >= 2.51f)
    {
        // 65 C to 70 C
        temperature =
            65.0f +
            (2.74f - vts)
            * (70.0f - 65.0f)
            / (2.74f - 2.51f);
    }
    else
    {
        // 70 C to 85 C
        temperature =
            70.0f +
            (2.51f - vts)
            * (85.0f - 70.0f)
            / (2.51f - 1.78f);
    }

    return temperature;
}


void otp_update(void)
{
    uint16_t adc_value = temperature_read_average();

    /*
     * Higher temperature -> lower Vts.
     *
     * 70 C ? 2.51 V
     * ADC ? 514
     */

    if (adc_value <= OTP_ADC_THRESHOLD)
    {
        otp_fault = 1;
    }
    else
    {
        otp_fault = 0;
    }
}


uint8_t otp_get_fault(void)
{
    return otp_fault;
}

