#include "gpio_lib.h"
#include "usart.h"
#include "errors.h"
#include <util/delay.h>
#include "timer.h"

#define OUTPUT_COUNTER_LEDS_REGISTER    (PC)
#define OUTPUT_COUNTER_LEDS_BITS        (BIT0 | BIT1 | BIT2 | BIT3)
#define OUTPUT_COUNTER_LEDS_MASK        (0xFF & ~OUTPUT_COUNTER_LEDS_BITS)
#define OUTPUT_COUNTER_LEDS_FIRST_BIT   (BIT0)
#define OUTPUT_SPEED_LEDS_REGISTER      (PD)
#define OUTPUT_SPEED_LEDS_BITS          (BIT0 | BIT1 | BIT2 | BIT3)
#define OUTPUT_SPEED_LEDS_MASK          (0xFF & ~OUTPUT_SPEED_LEDS_BITS)
#define OUTPUT_SPEED_LEDS_FIRST_BIT     (BIT0)
#define OUTPUT_DIGITS_LEDS_REGISTER     (PD)
#define OUTPUT_DIGITS_LEDS_BITS         (BIT4 | BIT5 | BIT6 | BIT7)
#define OUTPUT_DIGITS_LEDS_MASK         (0xFF & ~OUTPUT_DIGITS_LEDS_BITS)
#define OUTPUT_DIGITS_LEDS_FIRST_BIT    (BIT4)
#define INPUT_BUTTON_1_REGISTER         (PB)
#define INPUT_BUTTON_1_BIT              (BIT0)
#define OUTPUT_DP_LED_PORT              (PB)
#define OUTPUT_DP_LED_BIT               (BIT3)

void buttonPushISR(void);
int32_t showBinairy(uint8_t value, uint8_t port, uint8_t firstBit, uint8_t mask);
int32_t saveSpeed(float speedMS);
int32_t showSpeed(uint8_t numberPort, uint8_t digitPort, uint8_t numberFirstBit, uint8_t digitFirstBit, uint8_t numberMask, uint8_t digitBits);
uint8_t bitIndex(uint8_t value);

void tempSaveSeconds(uint32_t seconds);

uint8_t teller = 0u;
volatile uint8_t speedSegmentBuffer[4] = {0u};

int main(void) {
    int32_t error = SYSTEM_OK;
    uint32_t seconden = 0u;
    uint32_t lastMillis = 0u;

    timerInit();

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
    else if (gpioPinSetDirection(OUTPUT_DP_LED_PORT, OUTPUT_DP_LED_BIT, OUTPUT) != SYSTEM_OK) {
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
        // (void)saveSpeed(0.0f);
        while (1) {
            uint32_t currentMillis = millis();
            (void)showBinairy((teller / 2) % 16, OUTPUT_COUNTER_LEDS_REGISTER, OUTPUT_COUNTER_LEDS_FIRST_BIT, OUTPUT_COUNTER_LEDS_MASK);
            // _delay_ms(2);
            // (void)showSpeed(OUTPUT_SPEED_LEDS_REGISTER, OUTPUT_DIGITS_LEDS_REGISTER, OUTPUT_SPEED_LEDS_FIRST_BIT, OUTPUT_DIGITS_LEDS_FIRST_BIT, OUTPUT_SPEED_LEDS_MASK, OUTPUT_DIGITS_LEDS_BITS);
            if ((uint32_t)(currentMillis - lastMillis) >= 1000u) {
                lastMillis += 1000u;
                seconden++;
                tempSaveSeconds(seconden);
            }
        }
    }
    return error;
}

void buttonPushISR(void) {
    teller++;
}

int32_t showBinairy(uint8_t value, uint8_t port, uint8_t firstBit, uint8_t mask) {
    int32_t error = SYSTEM_OK;
    uint8_t copyOutput = 0u;

    value = (value << bitIndex(firstBit));
    port *= 3u;
    copyOutput = (*(&PORTB + port)) & mask;
    (*(&PORTB + port)) = copyOutput | value;

    return error;
}

int32_t saveSpeed(float speedMS) {
    int32_t error = SYSTEM_OK;
    uint16_t number = 0u;
    number = (uint16_t)(speedMS * 10.0);

    speedSegmentBuffer[0] = (number)        % 10;
    speedSegmentBuffer[1] = (number / 10)   % 10;
    speedSegmentBuffer[2] = (number / 100)  % 10;
    speedSegmentBuffer[3] = (number / 1000) % 10;
    return error;
}

void tempSaveSeconds(uint32_t seconds) {
    uint8_t digits[4];

    digits[0] = seconds             % 10u;
    digits[1] = (seconds / 10u)     % 10u;
    digits[2] = (seconds / 100u)    % 10u;
    digits[3] = (seconds / 1000u)   % 10u;

    cli();
    speedSegmentBuffer[0] = digits[0];
    speedSegmentBuffer[1] = digits[1];
    speedSegmentBuffer[2] = digits[2];
    speedSegmentBuffer[3] = digits[3];
    sei();
}

int32_t showSpeed(uint8_t numberPort, uint8_t digitPort, uint8_t numberFirstBit, uint8_t digitFirstBit, uint8_t numberMask, uint8_t digitBits) {
    int32_t error = SYSTEM_OK;
    static uint8_t digit = 0u;

    if (gpioPinSetValue(digitPort, digitBits, HIGH) != SYSTEM_OK) {
        error = ERROR;
    }
    else if (showBinairy(speedSegmentBuffer[digit], numberPort, numberFirstBit, numberMask) != SYSTEM_OK) {
        error = ERROR;
    }
    else if (gpioPinSetValue(digitPort, (digitFirstBit << digit), LOW) != SYSTEM_OK) {
        error = ERROR;
    }
    else {
        // if (digit == 1) {
        //     (void)gpioPinSetValue(OUTPUT_DP_LED_PORT, OUTPUT_DP_LED_BIT, HIGH);
        // }
        // else {
        //     (void)gpioPinSetValue(OUTPUT_DP_LED_PORT, OUTPUT_DP_LED_BIT, LOW);
        // }
        digit = (digit + 1) & 0x03;
    }
    return error;
}

uint8_t bitIndex(uint8_t value) {
    uint8_t index = 0u;
    while (value >>= 1) {
        index++;
    }
    return index;
}

ISR(TIMER0_COMPA_vect) {
    miliseconden++;
    (void)showSpeed(OUTPUT_SPEED_LEDS_REGISTER, OUTPUT_DIGITS_LEDS_REGISTER, OUTPUT_SPEED_LEDS_FIRST_BIT, OUTPUT_DIGITS_LEDS_FIRST_BIT, OUTPUT_SPEED_LEDS_MASK, OUTPUT_DIGITS_LEDS_BITS);
}

ISR(TIMER1_OVF_vect) {
    // miliseconden++;
    // (void)showSpeed(OUTPUT_SPEED_LEDS_REGISTER, OUTPUT_DIGITS_LEDS_REGISTER, OUTPUT_SPEED_LEDS_FIRST_BIT, OUTPUT_DIGITS_LEDS_FIRST_BIT, OUTPUT_SPEED_LEDS_MASK, OUTPUT_DIGITS_LEDS_BITS);
}
