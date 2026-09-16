#include "timer.h"
#include "carSpeed.h"

volatile uint32_t miliseconden = 0u;

void timerMillisInit(void) {
    miliseconden = 0u;

    /* Timer0: CTC, clock /64, compare 249 -> 1 kHz (1 ms) at F_CPU = 16 MHz. */
    TCCR0A = (1u << WGM01);
    TCCR0B = (1u << CS01) | (1u << CS00);
    OCR0A = 249u;

    TIMSK0 |= 0x02;
    sei();
}

void timerSpeedInit(void) {
    /* Timer1: normal mode, clock /64 -> overflow every 262.144 ms at F_CPU = 16 MHz. */
    TCCR1A = 0u;
    TCCR1B = (1u << WGM12) | (1u << CS11) | (1u << CS10);
    OCR1A = 1041u;

    TIMSK1 |= 0x02;
    sei();
}

uint32_t millis(void) {
    return miliseconden;
}

ISR(TIMER0_COMPA_vect) {
    miliseconden++;
}

ISR(TIMER1_COMPA_vect) {
    (void)carSpeedShowSpeed(OUTPUT_SPEED_LEDS_REGISTER, OUTPUT_DIGITS_LEDS_REGISTER, OUTPUT_SPEED_LEDS_FIRST_BIT, OUTPUT_DIGITS_LEDS_FIRST_BIT, OUTPUT_SPEED_LEDS_MASK, OUTPUT_DIGITS_LEDS_BITS);
}
