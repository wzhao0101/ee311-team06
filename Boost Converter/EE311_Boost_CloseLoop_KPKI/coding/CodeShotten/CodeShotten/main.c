#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

/* ---------- Hardware settings ---------- */

#define ADC_REF_MV       5000UL
#define R22_OHM          1000UL
#define R24_OHM          10000UL

#define VOUT_FULL_SCALE_MV \
    ((ADC_REF_MV * (R22_OHM + R24_OHM)) / R24_OHM)

/* Output voltage reference */
#define DEFAULT_REF_MV   5000U

/* UART reference command range */
#define REF_MIN_MV       1000U
#define REF_MAX_MV       5200U


#define PI_MIN_MV        2500  //set minimum duty cycle as 50%
#define PI_MAX_MV        4000
#define PI_BIAS_MV       3325


 // Kp = 0.10
 // Ki = 2.0 / second
 //Sample period = 100 us
 // Ki * Ts = 1 / 5000
#define KP_DIVISOR        10
#define INTEGRAL_DIVISOR  5000

//16 MHz / 160 = 100 kHz 
#define PWM_TOP           159U

//set UART for further display
// UART: 9600 baud, 8N1 
#define UART_BAUD         9600UL
#define UART_UBRR \
    (F_CPU / (16UL * UART_BAUD) - 1UL)



static volatile uint16_t reference_mv = DEFAULT_REF_MV;

static int16_t integral_mv = 0;
static int16_t integral_remainder = 0;

volatile uint16_t measured_vout_mv = 0;
volatile uint16_t pi_output_mv = PI_MIN_MV;

/* ---------- PWM: PB1 / OC1A / IO9 ---------- */

static void pwm_write_mv(uint16_t output_mv)
{
    // to make sure the duty cycle is in range
    if (output_mv < PI_MIN_MV) {
        output_mv = PI_MIN_MV;
    }

    if (output_mv > PI_MAX_MV) {
        output_mv = PI_MAX_MV;
    }

    //duty = (OCR1A + 1) / (ICR1 + 1)
    //counts = output_mv / 5000 * 160
    
	uint16_t counts =
        (uint16_t)((output_mv * 4U + 62U) / 125U);

    OCR1A = counts - 1U;
}

static void pwm_init(void)
{
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;

    PORTB &= ~(1 << PORTB1);
    DDRB |= (1 << DDB1);

    ICR1 = PWM_TOP;

    //start with minimum set duty cycle
    pwm_write_mv(PI_MIN_MV);

    //Non-inverting Fast PWM, mode 14, TOP = ICR1 
    TCCR1A = (1 << COM1A1) | (1 << WGM11);

    //set prescaler as 1，and start Timer1 
    TCCR1B = (1 << WGM13) |(1 << WGM12) |(1 << CS10);
}

/* ---------- UART ---------- */

static void uart_init(void)
{
    //Set PD0 / RXD input, no pull-up 
    DDRD &= ~(1 << DDD0);
    PORTD &= ~(1 << PORTD0);

    UBRR0H = (uint8_t)(UART_UBRR >> 8);
    UBRR0L = (uint8_t)UART_UBRR;

    UCSR0A = 0;

    //接收使能 + 接收中断使能 
    UCSR0B = (1 << RXEN0) | (1 << RXCIE0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

#if defined(USART0_RX_vect)
ISR(USART0_RX_vect)
#elif defined(USART_RX_vect)
ISR(USART_RX_vect)
#else
#error "Unsupported USART receive interrupt vector"
#endif
{
    
     //输入四位数字后按Enter。
     //例如：5000代表目标输出电压5 V
	 
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

/* ---------- ADC: PC0 / ADC0 ---------- */

static void adc_init(void)
{
    DDRC &= ~(1 << DDC0);
    PORTC &= ~(1 << PORTC0);
    DIDR0 |= (1 << ADC0D);

    /* AVCC reference, ADC0, right-adjusted result */
    ADMUX = (1 << REFS0);
    ADCSRB = 0;

    
     //ADC clock = 16 MHz / 64 = 250 kHz。
     // 保留原设置；该频率高于最大10-bit精度的推荐范围。

    ADCSRA = (1 << ADEN) |
             (1 << ADPS2) |
             (1 << ADPS1);

    // 第一次转换用于初始化，结果丢弃 
    ADCSRA |= (1 << ADSC);

    while (ADCSRA & (1 << ADSC)) {
    }

    (void)ADC;

    //清除完成标志，再开启ADC中断 
    ADCSRA |= (1 << ADIF);
    ADCSRA |= (1 << ADIE);
}

/* ---------- Timer2: 100 us sampling ---------- */

static void sampling_timer_init(void)
{
    /* CTC mode */
    TCCR2A = (1 << WGM21);
    TCCR2B = 0;
    TCNT2 = 0;

    /* 16 MHz / 8 / 200 = 10 kHz，即100 us */
    OCR2A = 199;

    TIFR2 = (1 << OCF2A);
    TIMSK2 = (1 << OCIE2A);

    // Prescaler = 8，启动Timer2 
    TCCR2B = (1 << CS21);
}

ISR(TIMER2_COMPA_vect)
{
    // ADC空闲时，启动一次转换 
    if (!(ADCSRA & (1 << ADSC))) {
        ADCSRA |= (1 << ADSC);
    }
}

/* ---------- PI calculation ---------- */

static uint16_t pi_update(uint16_t vout_mv,
                          uint16_t target_mv)
{
    // keep proportional relationship
    int16_t error_mv =
        (int16_t)vout_mv - (int16_t)target_mv;

    // P项：Kp = 0.10 
    int16_t proportional_mv = error_mv / KP_DIVISOR;

  
     // 先计算候选积分值。
     // remainder保存不足1 mV的累积量，
     //避免整数除法把小误差的积分截成0。
	 
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

    //用候选积分计算输出，判断是否继续向饱和方向推动 
    int16_t trial_mv =
        PI_BIAS_MV + proportional_mv + candidate_integral;

    uint8_t pushes_lower =
        (trial_mv <= PI_MIN_MV && error_mv < 0);

    uint8_t pushes_upper =
        (trial_mv >= PI_MAX_MV && error_mv > 0);

    //Anti-windup：只接受不会进一步推动饱和的积分 
    if (!pushes_lower && !pushes_upper) {
        integral_mv = candidate_integral;
        integral_remainder = candidate_remainder;
    }

    // 最终输出 = 偏置 + P项 + I项 
    int16_t output_mv =
        PI_BIAS_MV + proportional_mv + integral_mv;

    if (output_mv < PI_MIN_MV) {
        output_mv = PI_MIN_MV;
    }

    if (output_mv > PI_MAX_MV) {
        output_mv = PI_MAX_MV;
    }

    return (uint16_t)output_mv;
}

/* ---------- ADC conversion complete ---------- */

ISR(ADC_vect)
{
    // 1. Convert ADC value to actual Vout
    uint16_t vout_mv = (uint16_t)(((uint32_t)ADC * VOUT_FULL_SCALE_MV + 512UL) >> 10);

    measured_vout_mv = vout_mv;
	
    // 2.start PI calculation
    pi_output_mv = pi_update(vout_mv, reference_mv);

    // 3. update output PWM
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
        // 采样、PI控制和串口接收均由中断执行 
    }
}