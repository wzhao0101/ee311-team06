/*
 * IntegrationFirmware.c
 *
 * Created: 1/10/2026 1:43:44 pm
 * Author : byhz780
 */ 

#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

#include "adc.h"
#include "uart.h"
#include "touch.h"
#include "voltage.h"
#include "current.h"
#include "temperature.h"
#include "soe.h"
#include "protection.h"
#include "reset.h"


volatile uint32_t system_ms = 0;


/* =========================================================
   TIMER0 1 ms SYSTEM TICK
   ========================================================= */

ISR(TIMER0_COMPA_vect)
{
    system_ms++;
}


void timer0_init(void)
{
    // Timer0 CTC mode
    TCCR0A = (1 << WGM01);

    /*
     * 16 MHz / 64 = 250 kHz
     *
     * 250 counts = 1 ms
     *
     * OCR0A = 249
     */
    OCR0A = 249;

    // Prescaler = 64
    TCCR0B = (1 << CS01) | (1 << CS00);

    // Enable Compare Match A interrupt
    TIMSK0 = (1 << OCIE0A);
}

/* =========================================================
   PRINT TIMER1 COUNTS AS MICROSECONDS
   ========================================================= */

static void uart_send_timer_us(uint16_t count)
{
    /*
     * Timer1 runs at 2 MHz
     * 1 count = 0.5 us
     */

    uart_send_uint16(count / 2);

    if (count & 1)
    {
        uart_send_string(".5");
    }
    else
    {
        uart_send_string(".0");
    }

    uart_send_string("us");
}

/* =========================================================
   MAIN
   ========================================================= */

int main(void)
{
    uart_init();

    adc_init();

    touch_init();

    protection_init();
	
	reset_init();

    timer0_init();

    sei();


    uint32_t last_touch_time = 0;
    uint32_t last_uart_time = 0;


    while (1)
    {
        /*
         * ========================================
         * FAST PROTECTION PATH
         * ========================================
         *
         * No delay.
         * No UART.
         * No floating point.
         */

        protection_update_fast();


        /*
         * ========================================
         * TOUCH
         * every approximately 20 ms
         * ========================================
         */

        if ((system_ms - last_touch_time) >= 20)
        {
            last_touch_time = system_ms;

            touch_update();
        }
		

        reset_update(system_ms);


        /*
         * ========================================
         * UART DIAGNOSTICS
         * every approximately 250 ms
         * ========================================
         */

        if ((system_ms - last_uart_time) >= 250)
        {
            last_uart_time = system_ms;


            // -------------------------
            // Latest ADC readings
            // -------------------------

            uint16_t vvs_adc =
                protection_get_vvs_adc();

            uint16_t vis_adc =
                protection_get_vis_adc();

            uint16_t vts_adc =
                protection_get_vts_adc();


            // -------------------------
            // Voltage
            // -------------------------

            float vvs =
                voltage_adc_to_vvs(vvs_adc);

            float v_supercap =
                voltage_vvs_to_supercap(vvs);


            // -------------------------
            // SoE
            // -------------------------

            float soe =
                soe_calculate(v_supercap);


            // -------------------------
            // Current
            // -------------------------

            float vis =
                current_adc_to_vis(vis_adc);

            float current =
                current_vis_to_current(vis);


            // -------------------------
            // Temperature
            // -------------------------

            float vts =
                temperature_adc_to_vts(vts_adc);

            float temperature =
                temperature_vts_to_celsius(vts);


            // -------------------------
            // Touch count
            // -------------------------

            uint16_t touch_count =
                touch_get_count();
				
			// -------------------------
			// Protection timing
			// -------------------------

			uint16_t protection_time =
			protection_get_total_time_count();

			uint16_t ovp_time =
			protection_get_ovp_time_count();

			uint16_t ocp_time =
			protection_get_ocp_time_count();

			uint16_t otp_time =
			protection_get_otp_time_count();


            // =================================================
            // UART
            // =================================================

            uart_send_string("Vsc=");
            uart_send_float(v_supercap, 3);
            uart_send_string("V");


            uart_send_string(" | SoE=");
            uart_send_float(soe, 1);
            uart_send_string("%");


            uart_send_string(" | Vvs=");
            uart_send_float(vvs, 3);
            uart_send_string("V");


            uart_send_string(" | I=");
            uart_send_float(current, 2);
            uart_send_string("A");


            uart_send_string(" | Vis=");
            uart_send_float(vis, 3);
            uart_send_string("V");


            uart_send_string(" | Temp=");
            uart_send_float(temperature, 1);
            uart_send_string("C");


            uart_send_string(" | Vts=");
            uart_send_float(vts, 3);
            uart_send_string("V");


            uart_send_string(" | Touch=");
            uart_send_uint16(touch_count);


            // -------------------------
            // Fault diagnostics
            // -------------------------

            uart_send_string(" | OVP=");
            uart_send_uint16(
                protection_get_ovp()
            );


            uart_send_string(" | BOCP=");
            uart_send_uint16(
                protection_get_boost_ocp()
            );


            uart_send_string(" | COCP=");
            uart_send_uint16(
                protection_get_charger_ocp()
            );


            uart_send_string(" | OTP=");
            uart_send_uint16(
                protection_get_otp()
            );
			
			uart_send_string(" | Tprot=");
			uart_send_timer_us(protection_time);

			uart_send_string(" | Tovp=");
			uart_send_timer_us(ovp_time);

			uart_send_string(" | Tocp=");
			uart_send_timer_us(ocp_time);

			uart_send_string(" | Totp=");
			uart_send_timer_us(otp_time);


            uart_send_string("\r\n");
        }
    }
}
