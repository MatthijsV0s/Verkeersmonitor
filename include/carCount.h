#ifndef CAR_COUNT_H
#define CAR_COUNT_H

#include <avr/io.h>
#include "gpio_lib.h"

#define OUTPUT_COUNTER_LEDS_REGISTER    (PC)
#define OUTPUT_COUNTER_LEDS_BITS        (BIT0 | BIT1 | BIT2 | BIT3)
#define OUTPUT_COUNTER_LEDS_MASK        (0xFF & ~OUTPUT_COUNTER_LEDS_BITS)
#define OUTPUT_COUNTER_LEDS_FIRST_BIT   (BIT0)

int32_t carCountInit(void);
int32_t carCountDisplay(uint8_t count);

#endif /* CAR_COUNT_H */