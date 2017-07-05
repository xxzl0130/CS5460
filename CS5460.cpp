#include "CS5460.h"

/**
 * \brief constructor without arguments
 */
CS5460::CS5460(): resetPin(PIN_NDEFINED), edirPin(PIN_NDEFINED), eoutPin(PIN_NDEFINED), csPin(0)
{
}

/**
 * \brief constructor with arguments
 * \param _cs chip select pin
 * \param _reset rest pin
 * \param _edir EDIR pin
 * \param _eout EOUT pin
 */
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

/**
 * \brief Initialize the chip with sync signal and places the chip in the command 
 * mode where it waits until a valid command is received.
 */
void CS5460::init() const
{
	pinMode(12, INPUT_PULLUP);
	select();
	SPI.begin();
	SPI.beginTransaction(SPISettings(2000000L, MSBFIRST, SPI_MODE0));
	if(resetPin != PIN_NDEFINED)
	{
		digitalWrite(resetPin, HIGH);
	}
	SPI.transfer(SYNC1);
	SPI.transfer(SYNC1);
	SPI.transfer(SYNC1);
	SPI.transfer(SYNC0);
	unselect();
}

/**
 * \brief read a register and return the 24-bit data
 * \param reg register address
 * \return 24-bit raw data in MSB
 */
uint32_t CS5460::readRegister(uint8_t reg) const
{
	uint32_t data = 0;
	select();
	reg &= READ_REGISTER;
	SPI.transfer(reg);
	for(uint8_t i = 0;i < 3;++i)
	{
		data <<= 8;
		data |= SPI.transfer(SYNC0);
	}
	unselect();
	return data;
}

/**
 * \brief write a register for max 24-bit data
 * \param reg register address
 * \param cmd at most 24-bit data, more bits will be ignored
 */
void CS5460::writeRegister(uint8_t reg, uint32_t cmd) const
{
	int32Data data;
	select();
	reg |= WRITE_REGISTER;
	SPI.transfer(reg);
	data.data32 = cmd;
	for(int8_t i = 2;i >=0; --i)
	{// LSB in memory
		SPI.transfer(data.data8[i]);
	}
	unselect();
}

/**
 * \brief clear some bits in status register
 * \param cmd status command
 */
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
	return double(readRegister(LAST_CURRENT_REGISTER)) / UNSIGNED_OUTPUT_MAX;
}

uint32_t CS5460::getRawCurrent()
{
	return readRegister(LAST_CURRENT_REGISTER);
}

double CS5460::getVoltage()
{
	return double(readRegister(LAST_VOLTAGE_REGISTER)) / UNSIGNED_OUTPUT_MAX;
}

uint32_t CS5460::getRawVoltage()
{
	return readRegister(LAST_VOLTAGE_REGISTER);
}

double CS5460::getPower()
{
	int32_t rawData = readRegister(LAST_POWER_REGISTER);
	if(rawData & SIGN_BIT)
	{// signed
		// clear sign bit
		rawData ^= SIGN_BIT;
		// make it neg
		rawData = -rawData;
	}
	return double(rawData) / SIGNED_OUTPUT_MAX;
}

uint32_t CS5460::getRawPower()
{
	return readRegister(LAST_POWER_REGISTER);
}

double CS5460::getRMSCurrent()
{
	return double(readRegister(RMS_CURRENT_REGISTER)) / UNSIGNED_OUTPUT_MAX;
}

uint32_t CS5460::getRawRMSCurrent()
{
	return readRegister(RMS_CURRENT_REGISTER);
}

double CS5460::getRMSVoltage()
{
	return double(readRegister(RMS_VOLTAGE_REGISTER)) / UNSIGNED_OUTPUT_MAX;
}

uint32_t CS5460::getRawRMSVoltage()
{
	return readRegister(RMS_VOLTAGE_REGISTER);
}

double CS5460::getApparentPower()
{
	return getRMSCurrent() * getRMSVoltage();
}

double CS5460::getPowerFactor()
{
	return getPower() / getApparentPower();
}

uint32_t CS5460::getStatus()
{
	return readRegister(STATUS_REGISTER);
}

void CS5460::select() const
{
	digitalWrite(csPin, LOW);
}

void CS5460::unselect() const
{
	digitalWrite(csPin, HIGH);
}

