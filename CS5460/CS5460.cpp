#include "CS5460.h"
#include <Arduino.h>
#include <SPI.h>

CS5460::CS5460():resetPin(PIN_NDEFINED),edirPin(PIN_NDEFINED),eoutPin(PIN_NDEFINED)
{
	
}

CS5460::CS5460(uint8_t _cs, uint8_t _reset, uint8_t _edir, uint8_t _eout)
{
	csPin = _cs;
	pinMode(csPin, OUTPUT);
	resetPin = _reset;
	if (_reset != PIN_NDEFINED)
	{
		pinMode(resetPin, OUTPUT);
	}
	edirPin = _edir;
	if (_edir != PIN_NDEFINED)
	{
		pinMode(edirPin, INPUT);
	}
	eoutPin = _eout;
	if (_eout != PIN_NDEFINED)
	{
		pinMode(eoutPin, INPUT);
	}
}

void CS5460::init() const
{
	digitalWrite(csPin, LOW);
	SPI.setBitOrder(MSBFIRST);
	if(resetPin != PIN_NDEFINED)
	{
		digitalWrite(resetPin, HIGH);
	}
	digitalWrite(csPin, LOW);
	SPI.transfer(SYNC1);
	SPI.transfer(SYNC1);
	SPI.transfer(SYNC1);
	SPI.transfer(SYNC0);
	digitalWrite(csPin, HIGH);
}

uint32_t CS5460::readRegister(uint8_t reg)
{
	uint32_t data = 0;
	reg &= READ_REGISTER;
	SPI.transfer(reg);
	for(uint8_t i = 0;i < 3;++i)
	{
		data <<= 8;
		data |= SPI.transfer(0x0);
	}
	return data;
}

void CS5460::writeRegister(uint8_t reg, uint32_t cmd)
{
	int32Data data;
	reg |= WRITE_REGISTER;
	SPI.transfer(reg);
	data.data32 = cmd;
	for(uint8_t i = 2;i >=0; --i)
	{// LSB in memory
		SPI.transfer(data.data8[i]);
	}
}

void CS5460::clearStatus(uint32_t cmd)
{
	writeRegister(STATUS_REGISTER, cmd);
}

void CS5460::startSigleConvert()
{
	SPI.transfer(START_SIGLE_CPNVERT);
}

void CS5460::startMultiConvert()
{
	SPI.transfer(START_MULTI_CONVERT);
}

void CS5460::resetChip() const
{
	if(resetPin != PIN_NDEFINED)
	{
		digitalWrite(resetPin, LOW);
		delayMicroseconds(50);
		digitalWrite(resetPin, HIGH);
		delayMicroseconds(50);
	}
	else
	{
		writeRegister(CONFIG_REGISTER, CHIP_RESET);
		delayMicroseconds(50);
	}
}

double CS5460::getCurrent()
{
	return double(readRegister(LAST_CURRENT_REGISTER)) / SIGNED_OUTPUT_MAX;
}

double CS5460::getVoltage()
{
	return double(readRegister(LAST_VOLTAGE_REGISTER)) / SIGNED_OUTPUT_MAX;
}

double CS5460::getPower()
{
	return double(readRegister(LAST_POWER_REGISTER)) / SIGNED_OUTPUT_MAX;
}

double CS5460::getRMSCurrent()
{
	return double(readRegister(RMS_CURRENT_REGISTER)) / UNSIGNED_OUTPUT_MAX;
}

double CS5460::getRMSVoltage()
{
	return double(readRegister(RMS_VOLTAGE_REGISTER)) / SIGNED_OUTPUT_MAX;
}

double CS5460::getApparentPower()
{
	return getRMSCurrent() * getRMSVoltage();
}

double CS5460::getPowerFactor()
{
	return getPower() / getApparentPower();
}

