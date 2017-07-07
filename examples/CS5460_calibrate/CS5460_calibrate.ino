#include <CS5460.h>

/*
 * for UNO
 * MOSI 11 --- 23
 * MISO 12 --- 6
 * SCLK 13 --- 5
 * SS   10 --- 7
 */
CS5460 powerMeter(10);
uint32_t sum,data;
uint8_t cnt;
void setup()
{
    Serial.begin(9600);
    powerMeter.init();
    powerMeter.startMultiConvert();
}

void loop()
{
    data = powerMeter.calibrateVoltageOffset();
    sum += data;
    ++cnt;
    Serial.print(data,HEX);
    Serial.print("\t");
    Serial.print(sum / cnt,HEX);
    Serial.print("\n");
}
