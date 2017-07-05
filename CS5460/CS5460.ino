#include "cs5460.h"
#include <arduino.h>

/*
 * MOSI 11 --- 23
 * MISO 12 --- 6
 * SCLK 13 --- 5
 * SS    10 --- 7
 */

CS5460 powerMeter(10);

void setup()
{
    Serial.begin(9600);
    powerMeter.init();
    powerMeter.startMultiConvert();
}

void loop()
{
    powerMeter.select();
    Serial.print(powerMeter.getCurrent());
    Serial.print("\t");
    Serial.print(powerMeter.getVoltage());
    Serial.print("\t");
    Serial.print(powerMeter.getPower());
    Serial.print("\n");
    Serial.print(powerMeter.getRawCurrent(),HEX);
    Serial.print("\t");
    Serial.print(powerMeter.getRawVoltage(),HEX);
    Serial.print("\t");
    Serial.print(powerMeter.getRawPower(),HEX);
    Serial.print("\n");
    delay(500);
    
    //powerMeter.getRawVoltage();
    
}

