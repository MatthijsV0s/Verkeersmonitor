#include "gpio_lib.h"
#include "usart.h"
#include "errors.h"

#define OUTPUT_COUNTER_LEDS         (BIT0 | BIT1 | BIT2 | BIT3)
#define OUTPUT_COUNTER_LEDS_MASK    (0xFF & ~OUTPUT_COUNTER_LEDS)

void buttonPushISR(void);
int32_t showBinairy(uint8_t value);

uint8_t teller = 0u;

int main(void) {
    int32_t error = SYSTEM_OK;

    USART_Init();
    if (gpioPinSetDirection(PB, OUTPUT_COUNTER_LEDS, OUTPUT) != SYSTEM_OK) {
        error = ERROR;
    }
    else if (gpioExternalInterruptEnable(PD, BIT2, FALLING_EDGE, &buttonPushISR) != SYSTEM_OK) {
        error = ERROR;
    }
    else if (globalInterruptEnable() != SYSTEM_OK) {
        error = ERROR;
    }
    else {
        while (1) {
            (void)showBinairy((teller / 2) % 16);
        }
    }
    return error;
}

void buttonPushISR(void) {
    teller++;
}

int32_t showBinairy(uint8_t value) {
    int32_t error = SYSTEM_OK;
    uint8_t copyOutput = 0u;

    copyOutput = PORTB & OUTPUT_COUNTER_LEDS_MASK;
    PORTB = copyOutput | value;

    return error;
}