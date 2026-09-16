#include "carSpeed.h"
#include "displayBinairy.h"
#include "errors.h"

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
    speedSegmentBuffer[2] = 0u;
    speedSegmentBuffer[3] = 0u;
    return error;
}

int32_t carSpeedSaveSpeed(float speedMS) {
    int32_t error = SYSTEM_OK;
    uint16_t number = 0u;
    number = (uint16_t)(speedMS * 10.0);

    speedSegmentBuffer[0] = (number)        % 10;
    speedSegmentBuffer[1] = (number / 10)   % 10;
    speedSegmentBuffer[2] = (number / 100)  % 10;
    speedSegmentBuffer[3] = (number / 1000) % 10;
    return error;
}

int32_t carSpeedShowSpeed(uint8_t numberPort, uint8_t digitPort, uint8_t numberFirstBit, uint8_t digitFirstBit, uint8_t numberMask, uint8_t digitBits) {
    int32_t error = SYSTEM_OK;
    static uint8_t digit = 0u;

    if (SYSTEM_OK != gpioPinSetValue(digitPort, digitBits, HIGH)) {
        error = ERROR;
    }
    else if (SYSTEM_OK != displayBinairyShowValue(speedSegmentBuffer[digit], numberPort, numberFirstBit, numberMask)) {
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