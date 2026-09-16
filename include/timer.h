#ifndef TIMER_H
#define TIMER_H

#include <avr/io.h>
#include <avr/interrupt.h>

void timerMillisInit(void);
void timerSpeedInit(void);
uint32_t millis(void);

#endif /* TIMER_H */