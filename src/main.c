#include "gpio_lib.h"
#include "usart.h"
#include "errors.h"
#include "timer.h"
#include "carSpeed.h"
#include "carCount.h"
#include "displayBinairy.h"

#define INPUT_BUTTON_1_REGISTER         (PB)
#define INPUT_BUTTON_1_BIT              (BIT0)
#define INPUT_BUTTON_2_REGISTER         (PC)
#define INPUT_BUTTON_2_BIT              (BIT5)

#define DEBOUNCE_TIME_MS                (10u)

#define CAR_COUNTER_OVERFLOW            (16u)
#define MAX_TIME_BETWEEN_AXLES_MS       (1000u)
#define SENSOR_COUNTER                  (0u)
#define SENSOR_SPEED                    (1u)
#define NUMBER_OF_SENSORS               (2u)

void buttonPushCounterISR(void);
void buttonPushSpeedISR(void);
bool vehicle_passed(uint8_t sensor);
bool axle_detected(uint8_t sensor);
void display_counter(uint8_t value);

volatile uint8_t    carCounter                              = 0u;
volatile uint32_t   lastTimeTeller1ISR                      = 0u;
volatile uint32_t   lastTimeTeller2ISR                      = 0u;
volatile bool       isButton1Down                           = false;
volatile bool       isButton2Down                           = false;
volatile bool       isFirstButtonPressedFlag                = false;
volatile bool       isSecondButtonPressedFlag               = false;
bool                isFirstCallFunction[NUMBER_OF_SENSORS]  = {true, true};
volatile uint32_t   timeAxlePast[NUMBER_OF_SENSORS]         = {0u};

int main(void) {
    int32_t error = SYSTEM_OK;
    timerMillisInit();
    timerSpeedInit();

    if (gpioPinSetDirection(INPUT_BUTTON_1_REGISTER, INPUT_BUTTON_1_BIT, INPUT) != SYSTEM_OK) {
        error = ERROR;
    }
    else if (gpioPinSetPullUp(INPUT_BUTTON_1_REGISTER, INPUT_BUTTON_1_BIT, PULLUP) != SYSTEM_OK) {
        error = ERROR;
    }
    else if (gpioPinChangeInterruptEnable(INPUT_BUTTON_1_REGISTER, INPUT_BUTTON_1_BIT, FALLING_EDGE, &buttonPushCounterISR) != SYSTEM_OK) {
        error = ERROR;
    }
    else if (gpioPinChangeInterruptEnable(INPUT_BUTTON_1_REGISTER, INPUT_BUTTON_1_BIT, RISING_EDGE, &buttonPushCounterISR) != SYSTEM_OK) {
        error = ERROR;
    }
    else if (gpioPinSetDirection(INPUT_BUTTON_2_REGISTER, INPUT_BUTTON_2_BIT, INPUT) != SYSTEM_OK) {
        error = ERROR;
    }
    else if (gpioPinSetPullUp(INPUT_BUTTON_2_REGISTER, INPUT_BUTTON_2_BIT, PULLUP) != SYSTEM_OK) {
        error = ERROR;
    }
    else if (gpioPinChangeInterruptEnable(INPUT_BUTTON_2_REGISTER, INPUT_BUTTON_2_BIT, FALLING_EDGE, &buttonPushSpeedISR) != SYSTEM_OK) {
        error = ERROR;
    }
    else if (gpioPinChangeInterruptEnable(INPUT_BUTTON_2_REGISTER, INPUT_BUTTON_2_BIT, RISING_EDGE, &buttonPushSpeedISR) != SYSTEM_OK) {
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
        while (1) {
            if (vehicle_passed(SENSOR_COUNTER)) {
                carCounter++;
                if (carCounter == CAR_COUNTER_OVERFLOW) {
                    carCounter = 0u;
                }
            }
            display_counter(carCounter);
            //determine_and_show_speed();
        }
    }
    return error;
}

/** 
 * Check if the button wan't already reported pressed, and if time between presses is more than DEBOUCE_TIME_MS.
 * Also depend on a button release, before marking a new press.
 */
void buttonPushCounterISR(void) {
    uint32_t    nowTimeTellerISR = millis();
    bool        isButtonLow = ((PINB & INPUT_BUTTON_1_BIT) == 0u); /* Make this less hardcoded */

    if (isButtonLow) {
        if (!isButton1Down && ((nowTimeTellerISR - lastTimeTeller1ISR) >= DEBOUNCE_TIME_MS)) {
            lastTimeTeller1ISR = nowTimeTellerISR;
            isButton1Down = true;
            isFirstButtonPressedFlag = true;
        }
    }
    else if (isButton1Down) {
        isButton1Down = false;
    }
}

/** 
 * Check if the button wan't already reported pressed, and if time between presses is more than DEBOUCE_TIME_MS.
 * Also depend on a button release, before marking a new press.
 */
void buttonPushSpeedISR(void) {
    uint32_t    nowTimeTellerISR = millis();
    bool        isButtonLow = ((PINC & INPUT_BUTTON_2_BIT) == 0u); /* Make this less hardcoded */

    if (isButtonLow) {
        if (!isButton2Down && ((nowTimeTellerISR - lastTimeTeller2ISR) >= DEBOUNCE_TIME_MS)) {
            lastTimeTeller2ISR = nowTimeTellerISR;
            isButton2Down = true;
            isSecondButtonPressedFlag = true;
        }
    }
    else if (isButton2Down) {
        isButton2Down = false;
    }
}

bool vehicle_passed(uint8_t sensor) {
    bool        hasVehiclePassed        = false;
    static bool isPreviousAxleDetected[NUMBER_OF_SENSORS];
    bool        isCurrentAxleDetected   = axle_detected(sensor);

    if (isFirstCallFunction[sensor]) { /* Initialize static variable isPreviousAxleDetected */
        isPreviousAxleDetected[sensor] = false;
        isFirstCallFunction[sensor] = false;
    }

    if (isCurrentAxleDetected && isPreviousAxleDetected[sensor]) {
        if ((millis() - timeAxlePast[sensor]) < MAX_TIME_BETWEEN_AXLES_MS) {
            hasVehiclePassed = true;
            isPreviousAxleDetected[sensor] = false;
        }
        else {
            timeAxlePast[sensor] = millis();
        }
    }
    else if (isCurrentAxleDetected && !isPreviousAxleDetected[sensor]) {
        isPreviousAxleDetected[sensor] = true;
        timeAxlePast[sensor] = millis();
    }
    else {
        /* No axle detected, do nothing */
    }
    return hasVehiclePassed;
}

bool axle_detected(uint8_t sensor) {
    bool isAxleDeteted = false;

    if (SENSOR_COUNTER == sensor) {
        isAxleDeteted = isFirstButtonPressedFlag;
        isFirstButtonPressedFlag = false;   /* Flag needs to be cleared, after handling new axle. */
    }
    else if (SENSOR_SPEED == sensor) {
        isAxleDeteted = isSecondButtonPressedFlag;
        isSecondButtonPressedFlag = false;  /* Flag needs to be cleared, after handling new axle. */
    }
    return isAxleDeteted;
}

void display_counter(uint8_t value) {
    (void)carCountDisplay(value);   /* Call previously written function to display the amount of cars that have been passed */
}
