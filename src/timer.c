#include "timer.h"

volatile uint32_t miliseconden = 0u;

void timerInit() {
    miliseconden = 0u;
    TCCR0A = (1u << WGM01);
    TCCR0B = (1u << CS01) | (1u << CS00);
    OCR0A = 249u;

    TCCR1A = 0u;
    TCCR1B = (1u << CS11) | (1u << CS10);
    // OCR1A = 249u;

    TIMSK0 |= 0x02;
    TIMSK1 |= 0x01;
    sei();
}

uint32_t millis(void) {
    return miliseconden;
}
