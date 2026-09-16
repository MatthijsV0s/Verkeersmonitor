#ifndef DISPLAY_BINAIRY_H
#define DISPLAY_BINAIRY_H

#include <avr/io.h>
#include "gpio_lib.h"

int32_t displayBinairyShowValue(uint8_t value, uint8_t port, uint8_t firstBit, uint8_t mask);

#endif /* DISPLAY_BINAIRY_H */