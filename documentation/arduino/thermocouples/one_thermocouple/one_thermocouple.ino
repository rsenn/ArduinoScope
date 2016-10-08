#include <SPI.h>
#include "Adafruit_MAX31855.h"

// Digital
#define CLK  3
#define DO_1 5
#define CS_1 4
Adafruit_MAX31855 thermocouple_1(CLK, CS_1, DO_1);

void setup() {
  Serial.begin(57600);
}

void loop() { 
  while (1)
  {
    Serial.println(thermocouple_1.readCelsius());
  }
}
