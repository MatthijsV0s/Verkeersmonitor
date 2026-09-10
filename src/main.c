#include "gpio_lib.h"
#include "usart.h"
#include "errors.h"
#include <util/delay.h>

#define OUTPUT_COUNTER_LEDS_REGISTER    (PC)
#define OUTPUT_COUNTER_LEDS_BITS        (BIT0 | BIT1 | BIT2 | BIT3)
#define OUTPUT_COUNTER_LEDS_MASK        (0xFF & ~OUTPUT_COUNTER_LEDS_BITS)
#define OUTPUT_SPEED_LEDS_REGISTER      (PD)
#define OUTPUT_SPEED_LEDS_BITS          (BIT0 | BIT1 | BIT2 | BIT3)
#define OUTPUT_SPEED_LEDS_MASK          (0xFF & ~OUTPUT_SPEED_LEDS_BITS)
#define OUTPUT_DIGITS_LEDS_REGISTER     (PD)
#define OUTPUT_DIGITS_LEDS_BITS         (BIT4 | BIT5 | BIT6 | BIT7)
#define OUTPUT_DIGITS_LEDS_MASK         (0xFF & ~OUTPUT_DIGITS_LEDS_BITS)
#define OUTPUT_DIGITS_LEDS_FIRST_BIT    (BIT4)
#define INPUT_BUTTON_1_REGISTER         (PB)
#define INPUT_BUTTON_1_BIT              (BIT0)

void buttonPushISR(void);
int32_t showBinairy(uint8_t value, uint8_t port, uint8_t mask);
void saveSpeed(float speedMS);
void showSpeed(void);

uint8_t teller = 0u;
uint8_t speedSegmentBuffer[4] = {0u};

int main(void) {
    int32_t error = SYSTEM_OK;

    if (gpioPinSetDirection(OUTPUT_COUNTER_LEDS_REGISTER, OUTPUT_COUNTER_LEDS_BITS, OUTPUT) != SYSTEM_OK) {
        error = ERROR;
    }
    else if (gpioPinSetDirection(OUTPUT_SPEED_LEDS_REGISTER, OUTPUT_SPEED_LEDS_BITS, OUTPUT) != SYSTEM_OK) {
        error = ERROR;
    }
    else if (gpioPinSetDirection(OUTPUT_DIGITS_LEDS_REGISTER, OUTPUT_DIGITS_LEDS_BITS, OUTPUT) != SYSTEM_OK) {
        error = ERROR;
    }
    else if (gpioPinSetDirection(INPUT_BUTTON_1_REGISTER, INPUT_BUTTON_1_BIT, INPUT) != SYSTEM_OK) {
        error = ERROR;
    }
    else if (gpioPinSetPullUp(INPUT_BUTTON_1_REGISTER, INPUT_BUTTON_1_BIT, NOPULLUP) != SYSTEM_OK) {
        error = ERROR;
    }
    else if (gpioPinChangeInterruptEnable(INPUT_BUTTON_1_REGISTER, INPUT_BUTTON_1_BIT, FALLING_EDGE, &buttonPushISR) != SYSTEM_OK) {
        error = ERROR;
    }
    else if (globalInterruptEnable() != SYSTEM_OK) {
        error = ERROR;
    }
    else {
        saveSpeed(123.4);
        while (1) {
            (void)showBinairy((teller / 2) % 16, OUTPUT_COUNTER_LEDS_REGISTER, OUTPUT_COUNTER_LEDS_MASK);
            showSpeed();
            _delay_ms(2);
        }
    }
    return error;
}

void buttonPushISR(void) {
    teller++;
}

int32_t showBinairy(uint8_t value, uint8_t port, uint8_t mask) {
    int32_t error = SYSTEM_OK;
    uint8_t copyOutput = 0u;

    port *= 3u;
    copyOutput = (*(&PORTB + port)) & mask;
    (*(&PORTB + port)) = copyOutput | value;

    return error;
}

void saveSpeed(float speedMS) {
    uint16_t number = 0u;
    number = (uint16_t)(speedMS * 10.0);

    speedSegmentBuffer[0] = (number)        % 10;
    speedSegmentBuffer[1] = (number / 10)   % 10;
    speedSegmentBuffer[2] = (number / 100)  % 10;
    speedSegmentBuffer[3] = (number / 1000) % 10;
}

void showSpeed(void) {
    static uint8_t digit = 0u;

    gpioPinSetValue(OUTPUT_DIGITS_LEDS_REGISTER, OUTPUT_DIGITS_LEDS_BITS, HIGH); // Set high (off) because of Common Cathode
    showBinairy(speedSegmentBuffer[digit], OUTPUT_SPEED_LEDS_REGISTER, OUTPUT_SPEED_LEDS_MASK);
    gpioPinSetValue(OUTPUT_DIGITS_LEDS_REGISTER, (OUTPUT_DIGITS_LEDS_FIRST_BIT << digit), LOW);

    digit = (digit + 1) & 0x03;
}
