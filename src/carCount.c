#include "carCount.h"
#include "displayBinairy.h"
#include "errors.h"

int32_t carCountInit(void) {
    int32_t error = SYSTEM_OK;

    if (SYSTEM_OK != gpioPinSetDirection(OUTPUT_COUNTER_LEDS_REGISTER, OUTPUT_COUNTER_LEDS_BITS, OUTPUT)) {
        error = ERROR;
    }
    return error;
}

int32_t carCountDisplay(uint8_t count) {
    int32_t error = SYSTEM_OK;

    if (SYSTEM_OK != displayBinairyShowValue(count, OUTPUT_COUNTER_LEDS_REGISTER, OUTPUT_COUNTER_LEDS_FIRST_BIT, OUTPUT_COUNTER_LEDS_MASK)) {
        error = ERROR;
    }
    return error;
}