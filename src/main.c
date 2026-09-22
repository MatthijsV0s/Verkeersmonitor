/**
 * Matthijs Vos
 * s1200598
 * Datapunt DP-MC1-6
 * Dit project bevat de firmware voor een verkeersmonitor,
 * deze monitor telt het aantal voertuigen dat passeert over de sensoren, 
 * en meet de snelheid van het passerende voertuig.
 * Copyright
 */
#include "gpio_lib.h"
#include "usart.h"
#include "errors.h"
#include "timer.h"
#include "carSpeed.h"
#include "carCount.h"
#include "displayBinairy.h"

#define INPUT_BUTTON_1_REGISTER         (PC)
#define INPUT_BUTTON_1_BIT              (BIT4)
#define INPUT_BUTTON_2_REGISTER         (PC)
#define INPUT_BUTTON_2_BIT              (BIT5)

#define DEBOUNCE_TIME_MS                (10u)

#define CAR_COUNTER_OVERFLOW            (16u)
#define MAX_TIME_BETWEEN_AXLES_MS       (1000u)
#define SPEED_MEASURE_TIMEOUT_MS        (10811u) /* 10.811 seconds at a 0.6 meter distance = 0.2 km/h */
#define DEFAULT_DISTANCE_M              (0.6f)
#define SENSOR_COUNTER                  (0u)
#define SENSOR_SPEED                    (1u)
#define NUMBER_OF_SENSORS               (2u)
#define CONVERTION_TO_KMH               (3600.0f)
#define CONVERTION_TO_MS                (1000.0f)
#define CONVERTION_FACTOR               CONVERTION_TO_KMH
#define MAX_SPEED_KMH                   (10.0f)
#define MAX_SPEED_MS                    (2.8f)
#define MAX_SPEED                       MAX_SPEED_KMH

void init(void);
void initializeIO(void);
bool vehiclePassed(uint8_t sensor);
bool axleDetected(uint8_t sensor);
void displayCounter(uint8_t value);
void determineAndShowSpeed(void);
void buttonPushCounterISR(void);
void buttonPushSpeedISR(void);

volatile uint8_t    carCounter                              = 0u;
volatile uint32_t   lastTimeTeller1ISR                      = 0u;
volatile uint32_t   lastTimeTeller2ISR                      = 0u;
volatile bool       isButton1Down                           = false;
volatile bool       isButton2Down                           = false;
volatile bool       isFirstButtonPressedFlag                = false;
volatile bool       isSecondButtonPressedFlag               = false;
volatile uint32_t   timeAxlePast[NUMBER_OF_SENSORS]         = {0u};
volatile uint32_t   timeStartMeasureSpeed                   = 0u;
bool                isCounterIncreased                      = false;

int main(void) {
    init();
    initializeIO();

    while (1) {
        if (vehiclePassed(SENSOR_COUNTER)) {
            carCounter++;
            if (carCounter == CAR_COUNTER_OVERFLOW) {
                carCounter = 0u;
            }
            isCounterIncreased = true;
        }
        displayCounter(carCounter);
        determineAndShowSpeed();
    }
}

void init(void) {
    timerMillisInit();
    timerSpeedInit();
}

void initializeIO(void) {
    gpioPinSetDirection(INPUT_BUTTON_1_REGISTER, INPUT_BUTTON_1_BIT, INPUT);
    gpioPinSetDirection(INPUT_BUTTON_2_REGISTER, INPUT_BUTTON_2_BIT, INPUT);
    gpioPinSetPullUp(INPUT_BUTTON_1_REGISTER, INPUT_BUTTON_1_BIT, PULLUP);
    gpioPinSetPullUp(INPUT_BUTTON_2_REGISTER, INPUT_BUTTON_2_BIT, PULLUP);
    gpioPinChangeInterruptEnable(INPUT_BUTTON_1_REGISTER, INPUT_BUTTON_1_BIT, FALLING_EDGE, &buttonPushCounterISR);
    gpioPinChangeInterruptEnable(INPUT_BUTTON_1_REGISTER, INPUT_BUTTON_1_BIT, RISING_EDGE, &buttonPushCounterISR);
    gpioPinChangeInterruptEnable(INPUT_BUTTON_2_REGISTER, INPUT_BUTTON_2_BIT, FALLING_EDGE, &buttonPushSpeedISR);
    gpioPinChangeInterruptEnable(INPUT_BUTTON_2_REGISTER, INPUT_BUTTON_2_BIT, RISING_EDGE, &buttonPushSpeedISR);
    carCountInit();
    carSpeedInit();
    globalInterruptEnable();
}

bool vehiclePassed(uint8_t sensor) {
    bool        hasVehiclePassed        = false;
    static bool isPreviousAxleDetected[NUMBER_OF_SENSORS] = {false, false};
    bool        isCurrentAxleDetected   = axleDetected(sensor);

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

bool axleDetected(uint8_t sensor) {
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

void displayCounter(uint8_t value) {
    (void)carCountDisplay(value);   /* Call previously written function to display the amount of cars that have been passed */
}

void determineAndShowSpeed(void) {
    float32_t speed = 0.0f;
    static bool isSpeedBeingMeasured = false;

    if (isCounterIncreased) {
        carSpeedSaveSpeed(CLEAR_DISPLAY);
        timeStartMeasureSpeed = timeAxlePast[SENSOR_COUNTER];
        isCounterIncreased = false;
        isSpeedBeingMeasured = true;
    }

    if (vehiclePassed(SENSOR_SPEED) && isSpeedBeingMeasured) {
        speed = DEFAULT_DISTANCE_M * CONVERTION_FACTOR / (float32_t)(timeAxlePast[SENSOR_SPEED] - timeStartMeasureSpeed);
        if (speed >= MAX_SPEED) {
            speed = MAX_SPEED;
        }
        carSpeedSaveSpeed(speed);
        isSpeedBeingMeasured = false;
    }
    else {
        if (((millis() - timeStartMeasureSpeed) > SPEED_MEASURE_TIMEOUT_MS) && isSpeedBeingMeasured) {
            // return error;
            isSpeedBeingMeasured = false;
        }
    }
}

/** 
 * Check if the button wan't already reported pressed, and if time between presses is more than DEBOUCE_TIME_MS.
 * Also depend on a button release, before marking a new press.
 */
void buttonPushCounterISR(void) {
    uint32_t    nowTimeTellerISR = millis();
    bool        isButtonLow = ((PINC & INPUT_BUTTON_1_BIT) == 0u); /* Make this less hardcoded */

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
