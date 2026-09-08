#include "gpio_lib.h"
#include "usart.h"
#include "errors.h"

void buttonPD2ISR(void);

int main(void) {
    int32_t error = SYSTEM_OK;

    USART_Init();
    USART_Transmit('B');
    if (gpioPinSetDirection(PB, BIT0, OUTPUT) != SYSTEM_OK) {
        error = ERROR;
        USART_Transmit('1');
    }
    // else if (gpioExternalInterruptEnable(PD, BIT2, FALLING_EDGE, &buttonPD2ISR) != SYSTEM_OK) {
    //     error = ERROR;
    //     USART_Transmit('2');
    // }
    else if (gpioPinSetDirection(PB, BIT1, INPUT) != SYSTEM_OK) {
        error = ERROR;
        USART_Transmit('I');
    }
    else if (gpioPinChangeInterruptEnable(PB, BIT1, FALLING_EDGE, &buttonPD2ISR) != SYSTEM_OK) {
        error = ERROR;
        USART_Transmit('2');
    }
    else if (globalInterruptEnable() != SYSTEM_OK) {
        error = ERROR;
        USART_Transmit('3');
    }
    else {
        while (1) {

        }
    }
    USART_Transmit('e');
    return error;
}

void buttonPD2ISR(void) {
    (void)gpioPinToggle(PB, BIT0);            // Toggle LED
    USART_Transmit('x');
}