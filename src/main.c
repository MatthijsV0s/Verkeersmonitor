#include "gpio_lib.h"
#include "usart.h"
#include "errors.h"
#include "timer.h"
#include "carSpeed.h"
#include "carCount.h"
#include "displayBinairy.h"

#define INPUT_BUTTON_1_REGISTER         (PB)
#define INPUT_BUTTON_1_BIT              (BIT0)

#define DEBOUNCE_TIME_MS                (10u)

void buttonPushISR(void);

volatile uint8_t    teller                      = 0u;
volatile uint32_t   lastTimeTellerISR           = 0u;
volatile bool       isButtonDown                = false;
volatile bool       isFirstButtonPressedFlag    = false;
volatile bool       isSecondButtonPressedFlag   = false;
bool                isFirstCallFunction         = true;

int main(void) {
    int32_t error = SYSTEM_OK;
    timerMillisInit();
    timerSpeedInit();

    if (gpioPinSetDirection(INPUT_BUTTON_1_REGISTER, INPUT_BUTTON_1_BIT, INPUT) != SYSTEM_OK) {
        error = ERROR;
    }
    else if (gpioPinSetPullUp(INPUT_BUTTON_1_REGISTER, INPUT_BUTTON_1_BIT, NOPULLUP) != SYSTEM_OK) {
        error = ERROR;
    }
    else if (gpioPinChangeInterruptEnable(INPUT_BUTTON_1_REGISTER, INPUT_BUTTON_1_BIT, FALLING_EDGE, &buttonPushISR) != SYSTEM_OK) {
        error = ERROR;
    }
    else if (gpioPinChangeInterruptEnable(INPUT_BUTTON_1_REGISTER, INPUT_BUTTON_1_BIT, RISING_EDGE, &buttonPushISR) != SYSTEM_OK) {
        error = ERROR;
    }
    else if (globalInterruptEnable() != SYSTEM_OK) {
        error = ERROR;
    }
    else if (SYSTEM_OK != carCountInit()) {
        error = ERROR;
    }
    else if (SYSTEM_OK != carSpeedInit()) {
        error = ERROR;
    }
    else {
        (void)carSpeedSaveSpeed(0.0f);
        while (1) {
            (void)carCountDisplay((teller >> 1) % 16);
        }
    }
    return error;
}

/** 
 * Check if the button wan't already reported pressed, and if time between presses is more than DEBOUCE_TIME_MS.
 * Also depend on a button release, before marking a new press.
 */
void buttonPushISR(void) {
    uint32_t nowTimeTellerISR = millis();
    bool isButtonLow = ((PINB & INPUT_BUTTON_1_BIT) == 0u);

    if (isButtonLow) {
        if (!isButtonDown && ((nowTimeTellerISR - lastTimeTellerISR) >= DEBOUNCE_TIME_MS)) {
            lastTimeTellerISR = nowTimeTellerISR;
            isButtonDown = true;
            isFirstButtonPressedFlag = true;
        }
    }
    else if (isButtonDown) {
        isButtonDown = false;
    }
}
    }
}
