#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>


/* ---------- Hardware settings: match actual circuit ---------- */

#define ADC_REF_MV       5000UL
#define R22_OHM          1000UL
#define R24_OHM          10000UL

#define VOUT_FULL_SCALE_MV \
    ((ADC_REF_MV * (R22_OHM + R24_OHM)) / R24_OHM)

/* Default project target: 5 V */
#define DEFAULT_REF_MV   5000U

/*
 * Implementation choice, not a lecture requirement.
 * UART accepts targets in this range.
 * This does NOT establish a validated hardware operating range.
 */
#define REF_MIN_MV       1000U
#define REF_MAX_MV       5200U

/* [L2] Filtered MCU output range */
#define PI_MIN_MV        1000
#define PI_MAX_MV        4000

/* Implementation choice: retain original 2.5 V bias */
#define PI_BIAS_MV       2500

/*
 * Retain original trial gains:
 *
 * Kp = 0.10 = 1/10
 * Ki = 2.0 per second
 *
 * [F3] Ts = 0.0001 s
 * Therefore Ki * Ts = 1/5000.
 *
 * These two divisors encode the gains.
 * If sample period changes, recompute INTEGRAL_DIVISOR.
 */
#define KP_DIVISOR        10
#define INTEGRAL_DIVISOR  5000

/* [F5] 16 MHz / (159 + 1) = 100 kHz */
#define PWM_TOP           159U

/* [F2] Implementation choice: 9600 baud, 8N1 */
#define UART_BAUD         9600UL
#define UART_UBRR \
    (F_CPU / (16UL * UART_BAUD) - 1UL)

/* ---------- State ---------- */

static volatile uint16_t reference_mv = DEFAULT_REF_MV;

/* Integral output, in integer mV */
static int16_t integral_mv = 0;

/* Fractional integral remainder: units of 1/5000 mV */
static int16_t integral_remainder = 0;

/* Optional debugger observations */
volatile uint16_t measured_vout_mv = 0;
volatile uint16_t pi_output_mv = PI_BIAS_MV;

/* ---------- PWM: PB1 / OC1A ---------- */

static void pwm_write_mv(uint16_t output_mv)
{
    /*
     * [F5, L1]
     * For a 5 V PWM high level:
     *
     * duty = output_mv / 5000
     * counts = duty * 160
     *        = output_mv * 4 / 125
     *
     * Non-inverting Fast PWM:
     * duty = (OCR1A + 1) / (ICR1 + 1).
     *
     * Valid here because output is limited to 1000-4000 mV.
     */
    uint16_t counts =
        (uint16_t)((output_mv * 4U + 62U) / 125U);

    OCR1A = counts - 1U;
}

static void pwm_init(void)
{
    /* [F1, F5] PB1 = OC1A */
    DDRB |= (1 << DDB1);

    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;

    ICR1 = PWM_TOP;
    pwm_write_mv(PI_BIAS_MV);  // Start at 50%

    /* Fast PWM mode 14: TOP = ICR1 */
    TCCR1A = (1 << COM1A1) |
             (1 << WGM11);

    /* Prescaler = 1 */
    TCCR1B = (1 << WGM13) |
             (1 << WGM12) |
             (1 << CS10);
}

/* ---------- UART ---------- */

static void uart_init(void)
{
    /* [F1, F2] PD0/RXD input, no pull-up */
    DDRD &= ~(1 << DDD0);
    PORTD &= ~(1 << PORTD0);

    UBRR0H = (uint8_t)(UART_UBRR >> 8);
    UBRR0L = (uint8_t)UART_UBRR;

    UCSR0A = 0;

    /* Receive enabled; receive-complete interrupt */
    UCSR0B = (1 << RXEN0) |
             (1 << RXCIE0);

    /* 8 data bits, no parity, 1 stop bit */
    UCSR0C = (1 << UCSZ01) |
             (1 << UCSZ00);
}

/* Device headers use different USART vector names */
#if defined(USART0_RX_vect)
ISR(USART0_RX_vect)
#elif defined(USART_RX_vect)
ISR(USART_RX_vect)
#else
#error "Unsupported USART receive interrupt vector"
#endif
{
    /* [F2] Four ASCII digits, terminated by CR and/or LF */
    static uint16_t value = 0;
    static uint8_t digits = 0;
    static uint8_t invalid = 0;

    uint8_t status = UCSR0A;
    uint8_t c = UDR0;

    if (status & ((1 << FE0) |
                  (1 << DOR0) |
                  (1 << UPE0))) {
        invalid = 1;
    }

    if (c == '\r' || c == '\n') {
        if (!invalid &&
            digits == 4 &&
            value >= REF_MIN_MV &&
            value <= REF_MAX_MV) {
            reference_mv = value;
        }

        value = 0;
        digits = 0;
        invalid = 0;
        return;
    }

    if (!invalid &&
        c >= '0' && c <= '9' &&
        digits < 4) {
        value = (uint16_t)(value * 10U + (c - '0'));
        digits++;
    } else {
        invalid = 1;
    }
}

/* ---------- ADC ---------- */

static void adc_init(void)
{
    /* [F1, F3] PC0 / ADC0 input */
    DDRC &= ~(1 << DDC0);
    PORTC &= ~(1 << PORTC0);
    DIDR0 |= (1 << ADC0D);

    /* AVCC reference; right-adjusted ADC0 result */
    ADMUX = (1 << REFS0);
    ADCSRB = 0;

    /*
     * ADC clock = 16 MHz / 64 = 250 kHz.
     * Normal conversion takes about 52 us.
     *
     * This meets the 100 us timing target, but 250 kHz
     * exceeds the manufacturer's maximum-resolution
     * recommended ADC clock range. See explanation below.
     */
    ADCSRA = (1 << ADEN) |
             (1 << ADPS2) |
             (1 << ADPS1);

    /* Discard the longer first conversion before sampling */
    ADCSRA |= (1 << ADSC);

    while (ADCSRA & (1 << ADSC)) {
    }

    (void)ADC;

    /* Clear completion flag; enable ADC interrupt */
    ADCSRA |= (1 << ADIF);
    ADCSRA |= (1 << ADIE);
}

/* ---------- 100 us sample scheduling ---------- */

static void sampling_timer_init(void)
{
    /* [F3] Timer2 CTC */
    TCCR2A = (1 << WGM21);
    TCCR2B = 0;
    TCNT2 = 0;

    /* 16 MHz / 8 / 200 = 10 kHz */
    OCR2A = 199;

    TIFR2 = (1 << OCF2A);
    TIMSK2 = (1 << OCIE2A);

    /* Prescaler = 8 */
    TCCR2B = (1 << CS21);
}

ISR(TIMER2_COMPA_vect)
{
    /* [F3] Start a conversion every 100 us */
    if (!(ADCSRA & (1 << ADSC))) {
        ADCSRA |= (1 << ADSC);
    }
}

/* ---------- PI after each measurement ---------- */

ISR(ADC_vect)
{
    uint16_t adc_count = ADC;

    /*
     * [F3] Recover Vout from the divider.
     *
     * ADC has 1024 quantization steps.
     * Use multiplication and a shift instead of float.
     */
    uint16_t vout_mv =
        (uint16_t)(((uint32_t)adc_count *
                    VOUT_FULL_SCALE_MV + 512UL) >> 10);

    measured_vout_mv = vout_mv;

    /* [F4, L3] Lecture error convention */
    int16_t error_mv =
        (int16_t)vout_mv - (int16_t)reference_mv;

    /* Kp = 0.10 */
    int16_t proportional_mv = error_mv / KP_DIVISOR;

    /*
     * [F4] Integral update:
     *
     * delta I = error_mv / 5000 mV per sample.
     *
     * Preserve fractional increments in the remainder.
     * Bounded error means these loops execute only a
     * small number of times per sample.
     */
    int16_t candidate_integral = integral_mv;
    int16_t candidate_remainder =
        integral_remainder + error_mv;

    while (candidate_remainder >= INTEGRAL_DIVISOR) {
        candidate_integral++;
        candidate_remainder -= INTEGRAL_DIVISOR;
    }

    while (candidate_remainder <= -INTEGRAL_DIVISOR) {
        candidate_integral--;
        candidate_remainder += INTEGRAL_DIVISOR;
    }

    int16_t trial_mv =
        PI_BIAS_MV + proportional_mv + candidate_integral;

    /*
     * [F4] Conditional integration anti-windup.
     *
     * Reject an integral update if it pushes the output
     * farther beyond either limit.
     */
    uint8_t pushes_lower =
        (trial_mv <= PI_MIN_MV && error_mv < 0);

    uint8_t pushes_upper =
        (trial_mv >= PI_MAX_MV && error_mv > 0);

    if (!pushes_lower && !pushes_upper) {
        integral_mv = candidate_integral;
        integral_remainder = candidate_remainder;
    }

    int16_t output_mv =
        PI_BIAS_MV + proportional_mv + integral_mv;

    /* [L2] 1-4 V operating range */
    if (output_mv < PI_MIN_MV) {
        output_mv = PI_MIN_MV;
    }

    if (output_mv > PI_MAX_MV) {
        output_mv = PI_MAX_MV;
    }

    pi_output_mv = (uint16_t)output_mv;

    /* [F5] Scale PI output to PWM duty */
    pwm_write_mv(pi_output_mv);
}

/* ---------- Main ---------- */

int main(void)
{
    cli();

    pwm_init();
    uart_init();
    adc_init();
    sampling_timer_init();

    sei();

    while (1) {
        /* Timer, ADC/PI and UART run through interrupts */
    }
}