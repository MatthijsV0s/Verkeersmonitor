#include "gpio_lib.h"
#include "usart.h"
#include "errors.h"

#define OUTPUT_MASK 0xF0

void buttonPushISR(void);
int32_t showBinairy(uint8_t value);

uint8_t teller = 0u;

int main(void) {
    int32_t error = SYSTEM_OK;

    USART_Init();
    if (gpioPinSetDirection(PB, BIT0 | BIT1 | BIT2 | BIT3, OUTPUT) != SYSTEM_OK) {
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

    copyOutput = PORTB & OUTPUT_MASK;
    PORTB = copyOutput | value;

    return error;
}