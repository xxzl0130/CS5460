#include "CS5460.h"

/**
 * \brief constructor without arguments
 */
CS5460::CS5460(): resetPin(PIN_NDEFINED), edirPin(PIN_NDEFINED), eoutPin(PIN_NDEFINED), csPin(PIN_NDEFINED),currentGain(1.0),voltageGain(1.0),powerGain(1.0)
{
}

/**
 * \brief constructor with arguments
 * \param _cs chip select pin
 * \param _reset rest pin
 * \param _edir EDIR pin
 * \param _eout EOUT pin
 */
CS5460::CS5460(uint8_t _cs, uint8_t _reset, uint8_t _edir, uint8_t _eout):currentGain(1.0), voltageGain(1.0),powerGain(1.0)
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
	SPI.beginTransaction(SETTING);
	if(resetPin != PIN_NDEFINED)
	{
		digitalWrite(resetPin, HIGH);
	}
	SPI.transfer(SYNC1);
	SPI.transfer(SYNC1);
	SPI.transfer(SYNC1);
	SPI.transfer(SYNC0);
	SPI.endTransaction();
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
	SPI.beginTransaction(SETTING);
	reg &= READ_REGISTER;
	SPI.transfer(reg);
	for(uint8_t i = 0;i < 3;++i)
	{
		data <<= 8;
		data |= SPI.transfer(SYNC1);
	}
	SPI.endTransaction();
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
	SPI.beginTransaction(SETTING);
	reg |= WRITE_REGISTER;
	SPI.transfer(reg);
	data.data32 = cmd;
	for(int8_t i = 2;i >=0; --i)
	{// LSB in memory
		SPI.transfer(data.data8[i]);
	}
	SPI.endTransaction();
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

void CS5460::startSingleConvert()
{
	send(START_SINGLE_CPNVERT);
}

void CS5460::startMultiConvert()
{
	send(START_MULTI_CONVERT);
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
	return signed2float(readRegister(LAST_CURRENT_REGISTER)) * currentGain;
}

uint32_t CS5460::getRawCurrent()
{
	return readRegister(LAST_CURRENT_REGISTER);
}

double CS5460::getVoltage()
{
	return signed2float(readRegister(LAST_VOLTAGE_REGISTER)) * voltageGain;
}

uint32_t CS5460::getRawVoltage()
{
	return readRegister(LAST_VOLTAGE_REGISTER);
}

double CS5460::getPower()
{
	return signed2float(readRegister(LAST_POWER_REGISTER)) * powerGain;
}

uint32_t CS5460::getRawPower()
{
	return readRegister(LAST_POWER_REGISTER);
}

double CS5460::getRMSCurrent()
{
	return unsigned2float(readRegister(RMS_CURRENT_REGISTER)) * currentGain;
}

uint32_t CS5460::getRawRMSCurrent()
{
	return readRegister(RMS_CURRENT_REGISTER);
}

double CS5460::getRMSVoltage()
{
	return unsigned2float(readRegister(RMS_VOLTAGE_REGISTER)) * voltageGain;
}

uint32_t CS5460::getRawRMSVoltage()
{
	return readRegister(RMS_VOLTAGE_REGISTER);
}

double CS5460::getApparentPower()
{
	return getRMSCurrent() * getRMSVoltage() * powerGain;
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

void CS5460::setCurrentGain(double gain)
{
	currentGain = gain;
	powerGain = currentGain * voltageGain;
}

void CS5460::setVoltageGain(double gain)
{
	voltageGain = gain;
	powerGain = currentGain * voltageGain;
}

/**
 * \brief send a 8-bit command with no return
 * \param cmd 8-bit command
 */
void CS5460::send(uint8_t cmd)
{
	select();
	SPI.beginTransaction(SETTING);
	SPI.transfer(cmd);
	SPI.endTransaction();
	unselect();
}

/**
 * \brief calibrate voltage/current gain/offset register
 * \param cmd calibrate command, must be commands for calibrate control
 */
void CS5460::calibrate(uint8_t cmd)
{
	cmd = CALIBRATE_CONTROL | (cmd & CALIBRATE_ALL);
	send(cmd);
	while(!(getStatus() & DATA_READY));
	// wait until data ready;
	clearStatus(DATA_READY);
}

uint32_t CS5460::calibrateVoltageOffset()
{
	calibrate(CALIBRATE_VOLTAGE | CALIBRATE_OFFSET);
	return readRegister(VOLTAGE_OFFSET_REGISTER);
}

uint32_t CS5460::calibrateVoltageGain()
{
	calibrate(CALIBRATE_VOLTAGE | CALIBRATE_GAIN);
	return readRegister(VOLTAGE_GAIN_REGISTER);
}

uint32_t CS5460::calibrateCurrentOffset()
{
	calibrate(CALIBRATE_CURRENT | CALIBRATE_OFFSET);
	return readRegister(CURRENT_OFFSET_REGISTER);
}

uint32_t CS5460::calibrateCurrentGain()
{
	calibrate(CALIBRATE_CURRENT | CALIBRATE_GAIN);
	return readRegister(CURRENT_GAIN_REGISTER);
}

double CS5460::signed2float(int32_t data)
{
	if(data & SIGN_BIT)
	{// signed
		// clear sign bit
		data ^= SIGN_BIT;
		// make it neg
		data = data - SIGNED_OUTPUT_MAX;
	}
	return double(data) / SIGNED_OUTPUT_MAX;
}

double CS5460::unsigned2float(uint32_t data)
{
	return double(data) / UNSIGNED_OUTPUT_MAX;
}