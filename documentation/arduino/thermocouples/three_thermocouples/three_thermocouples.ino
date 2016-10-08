#include <SPI.h>
#include "Adafruit_MAX31855.h"

// Digital
#define CLK  8

#define DO_1 2
#define CS_1 3
Adafruit_MAX31855 thermocouple_1(CLK, CS_1, DO_1);

#define DO_2 4
#define CS_2 5
Adafruit_MAX31855 thermocouple_2(CLK, CS_2, DO_2);

#define DO_3 6
#define CS_3 7
Adafruit_MAX31855 thermocouple_3(CLK, CS_3, DO_3);

void setup() {
  Serial.begin(57600);
}

void loop() { 
  while (1)
  {
    Serial.print  (thermocouple_1.readCelsius());
    Serial.print("; ");
    Serial.print  (thermocouple_2.readCelsius());
    Serial.print("; ");
    Serial.println(thermocouple_3.readCelsius());
  }
}
