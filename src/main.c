#include "gpio_lib.h"
#include "usart.h"

int main(void) {
    gpioPinSetDirection(PB, BIT0, OUTPUT);      // LED output

    USART_Init();

    // Configure INT0 on falling edge
    EICRA |= (1 << ISC01);
    EIMSK |= (1 << INT0);

    sei();                   // Enable global interrupts

    while (1)
    {
        
    }
}

ISR(INT0_vect)
{
    gpioPinToggle(PB, BIT0);            // Toggle LED
    USART_Transmit('x');
}