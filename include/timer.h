#ifndef TIMER_H
#define TIMER_H

#include <avr/io.h>
#include <avr/interrupt.h>

extern volatile uint32_t miliseconden;

extern void timerInit();
extern uint32_t millis(void);

#endif /* TIMER_H */