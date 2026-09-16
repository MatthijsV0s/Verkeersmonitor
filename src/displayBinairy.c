#include "displayBinairy.h"
#include "errors.h"

static uint8_t bitIndex(uint8_t value);

int32_t displayBinairyShowValue(uint8_t value, uint8_t port, uint8_t firstBit, uint8_t mask) {
    int32_t error = SYSTEM_OK;
    uint8_t copyOutput = 0u;

    value = (value << bitIndex(firstBit));
    port *= 3u;
    copyOutput = (*(&PORTB + port)) & mask;
    (*(&PORTB + port)) = copyOutput | value;

    return error;
}

static uint8_t bitIndex(uint8_t value) {
    uint8_t index = 0u;
    while (value >>= 1) {
        index++;
    }
    return index;
}
