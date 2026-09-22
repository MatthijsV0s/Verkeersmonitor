#include "carSpeed.h"
#include "displayBinairy.h"
#include "errors.h"

static uint8_t bitIndex(uint8_t value);

const uint8_t segments[10] = {  /* Binairy values follow this layout: 0b0GFEDCBA, with A-G representing the segments */
    0b00111111,                 /* 0 */
    0b00000110,                 /* 1 */
    0b01011011,                 /* 2 */
    0b01001111,                 /* 3 */
    0b01100110,                 /* 4 */
    0b01101101,                 /* 5 */
    0b01111101,                 /* 6 */
    0b00000111,                 /* 7 */
    0b01111111,                 /* 8 */
    0b01101111                  /* 9 */
};

volatile uint8_t speedSegmentBuffer[4] = {0u};

int32_t carSpeedInit(void) {
    int32_t error = SYSTEM_OK;

    if (SYSTEM_OK != gpioPinSetDirection(OUTPUT_SPEED_LEDS_REGISTER, OUTPUT_SPEED_LEDS_BITS, OUTPUT)) {
        error = ERROR;
    }
    else if (SYSTEM_OK != gpioPinSetDirection(OUTPUT_DIGITS_LEDS_REGISTER, OUTPUT_DIGITS_LEDS_BITS, OUTPUT)) {
        error = ERROR;
    }
    else if (SYSTEM_OK != gpioPinSetDirection(OUTPUT_DP_LED_PORT, OUTPUT_DP_LED_BIT, OUTPUT)) {
        error = ERROR;
    }
    speedSegmentBuffer[0] = 0u;
    speedSegmentBuffer[1] = 0u;
    speedSegmentBuffer[2] = EMPTY_SEGMENT;
    speedSegmentBuffer[3] = EMPTY_SEGMENT;
    return error;
}

int32_t carSpeedSaveSpeed(float32_t speed) {
    int32_t error = SYSTEM_OK;
    uint16_t number = 0u;

    if (speed == CLEAR_DISPLAY) {
        speedSegmentBuffer[0] = EMPTY_SEGMENT;
        speedSegmentBuffer[1] = EMPTY_SEGMENT;
        speedSegmentBuffer[2] = EMPTY_SEGMENT;
        speedSegmentBuffer[3] = EMPTY_SEGMENT;
    }
    else {
        number = (uint16_t)(speed * 10.0f);

        speedSegmentBuffer[0] = (number)         % 10u;
        speedSegmentBuffer[1] = (number / 10u)   % 10u;
        speedSegmentBuffer[2] = (number / 100u)  % 10u;
        speedSegmentBuffer[3] = (number / 1000u) % 10u;

        if ((0u == speedSegmentBuffer[2]) && (0u == speedSegmentBuffer[3])) {
            speedSegmentBuffer[2] = EMPTY_SEGMENT;
            speedSegmentBuffer[3] = EMPTY_SEGMENT;
        }
        else if (0u == speedSegmentBuffer[3]) {
            speedSegmentBuffer[3] = EMPTY_SEGMENT;
        }
    }
    return error;
}

int32_t carSpeedShowSpeed(uint8_t numberPort, uint8_t digitPort, uint8_t numberFirstBit, uint8_t digitFirstBit, uint8_t numberMask, uint8_t digitBits) {
    int32_t error = SYSTEM_OK;
    static uint8_t digit = 0u;

    if (SYSTEM_OK != gpioPinSetValue(digitPort, digitBits, HIGH)) {
        error = ERROR;
    }
    else if (SYSTEM_OK != displaySegments(speedSegmentBuffer[digit], numberPort, numberFirstBit, numberMask)) {
        error = ERROR;
    }
    else if (SYSTEM_OK != gpioPinSetValue(digitPort, (digitFirstBit << digit), LOW)) {
        error = ERROR;
    }
    else {
        if (digit == 1) {
            (void)gpioPinSetValue(OUTPUT_DP_LED_PORT, OUTPUT_DP_LED_BIT, HIGH);
        }
        else {
            (void)gpioPinSetValue(OUTPUT_DP_LED_PORT, OUTPUT_DP_LED_BIT, LOW);
        }
        digit = (digit + 1) & 0x03;
    }
    return error;
}

int32_t displaySegments(uint8_t value, uint8_t port, uint8_t firstBit, uint8_t mask) {
    int32_t error = SYSTEM_OK;
    uint8_t copyOutput = 0u;
    uint8_t output = 0u;

    if (value != EMPTY_SEGMENT) {
        output = segments[value];
    }
    output = (output << bitIndex(firstBit));
    port *= 3u;
    copyOutput = (*(&PORTB + port)) & mask;
    (*(&PORTB + port)) = copyOutput | output;

    return error;
}

static uint8_t bitIndex(uint8_t value) {
    uint8_t index = 0u;
    while (value >>= 1) {
        index++;
    }
    return index;
}
