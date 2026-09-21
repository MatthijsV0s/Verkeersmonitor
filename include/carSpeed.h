#ifndef CAR_SPEED_H
#define CAR_SPEED_H

#include <avr/io.h>
#include "gpio_lib.h"

typedef float float32_t;
typedef double float64_t;

#define OUTPUT_SPEED_LEDS_REGISTER      (PD)
#define OUTPUT_SPEED_LEDS_BITS          (BIT0 | BIT1 | BIT2 | BIT3)
#define OUTPUT_SPEED_LEDS_MASK          (0xFF & ~OUTPUT_SPEED_LEDS_BITS)
#define OUTPUT_SPEED_LEDS_FIRST_BIT     (BIT0)
#define OUTPUT_DIGITS_LEDS_REGISTER     (PD)
#define OUTPUT_DIGITS_LEDS_BITS         (BIT4 | BIT5 | BIT6 | BIT7)
#define OUTPUT_DIGITS_LEDS_MASK         (0xFF & ~OUTPUT_DIGITS_LEDS_BITS)
#define OUTPUT_DIGITS_LEDS_FIRST_BIT    (BIT4)
#define OUTPUT_DP_LED_PORT              (PB)
#define OUTPUT_DP_LED_BIT               (BIT3)

int32_t carSpeedInit(void);
int32_t carSpeedSaveSpeed(float32_t speed);
int32_t carSpeedShowSpeed(uint8_t numberPort, uint8_t digitPort, uint8_t numberFirstBit, uint8_t digitFirstBit, uint8_t numberMask, uint8_t digitBits);

#endif /* CAR_SPEED_H */