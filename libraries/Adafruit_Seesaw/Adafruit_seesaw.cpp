/*!
 * @file Adafruit_seesaw.cpp
 *
 * @mainpage Adafruit seesaw arduino driver
 *
 * @section intro_sec Introduction
 *
 * This is part of Adafruit's seesaw driver for the Arduino platform.  It is
 * designed specifically to work with the Adafruit products that use seesaw technology.
 *
 * These chips use I2C to communicate, 2 pins (SCL+SDA) are required
 * to interface with the board.
 *
 * Adafruit invests time and resources providing this open source code,
 * please support Adafruit and open-source hardware by purchasing
 * products from Adafruit!
 *
 * @section author Author
 *
 * Written by Dean Miller for Adafruit Industries.
 *
 * @section license License
 *
 * BSD license, all text here must be included in any redistribution.
 *
 */

#include "Adafruit_seesaw.h"

/**
 *****************************************************************************************
 *  @brief      Start the seesaw
 *
 *				This should be called when your sketch is connecting to the seesaw
 * 
 *  @param      addr the I2C address of the seesaw
 *
 *  @return     true if we could connect to the seesaw, false otherwise
 ****************************************************************************************/
bool Adafruit_seesaw::begin(uint8_t addr)
{
	_i2caddr = addr;
	
	_i2c_init();

	SWReset();
	delay(500);

	uint8_t c = this->read8(SEESAW_STATUS_BASE, SEESAW_STATUS_HW_ID);

	if(c != SEESAW_HW_ID_CODE) return false;
	
	return true;
}

/**
 *****************************************************************************************
 *  @brief      perform a software reset. This resets all seesaw registers to their default values.
 *
 *  			This is called automatically from Adafruit_seesaw.begin()
 * 
 *
 *  @return     none
 ****************************************************************************************/
void Adafruit_seesaw::SWReset()
{
	this->write8(SEESAW_STATUS_BASE, SEESAW_STATUS_SWRST, 0xFF);
}

/**
 *****************************************************************************************
 *  @brief      Returns the available options compiled into the seesaw firmware.
 * 
 *
 *  @return     the available options compiled into the seesaw firmware. If the option is included, the
 *				corresponding bit is set. For example, 
 *				if the ADC module is compiled in then (ss.getOptions() & (1UL << SEESAW_ADC_BASE)) > 0
 ****************************************************************************************/
uint32_t Adafruit_seesaw::getOptions()
{
	uint8_t buf[4];
	this->read(SEESAW_STATUS_BASE, SEESAW_STATUS_OPTIONS, buf, 4);
	uint32_t ret = ((uint32_t)buf[0] << 24) | ((uint32_t)buf[1] << 16) | ((uint32_t)buf[2] << 8) | (uint32_t)buf[3];
	return ret;
}

/**
 *****************************************************************************************
 *  @brief      Returns the version of the seesaw
 *
 *  @return     The version code. Bits [31:16] will be a date code, [15:0] will be the product id.
 ****************************************************************************************/
uint32_t Adafruit_seesaw::getVersion()
{
	uint8_t buf[4];
	this->read(SEESAW_STATUS_BASE, SEESAW_STATUS_VERSION, buf, 4);
	uint32_t ret = ((uint32_t)buf[0] << 24) | ((uint32_t)buf[1] << 16) | ((uint32_t)buf[2] << 8) | (uint32_t)buf[3];
	return ret;
}

/**
 *****************************************************************************************
 *  @brief      Set the mode of a GPIO pin.
 * 
 *  @param      pin the pin number. On the SAMD09 breakout, this corresponds to the number on the silkscreen.
 *  @param		mode the mode to set the pin. One of INPUT, OUTPUT, or INPUT_PULLUP.
 *
 *  @return     none
 ****************************************************************************************/
void Adafruit_seesaw::pinMode(uint8_t pin, uint8_t mode)
{
	pinModeBulk(1ul << pin, mode);
}

/**
 *****************************************************************************************
 *  @brief      Set the output of a GPIO pin
 * 
 *  @param      pin the pin number. On the SAMD09 breakout, this corresponds to the number on the silkscreen.
 *	@param		value the value to write to the GPIO pin. This should be HIGH or LOW.
 *
 *  @return     none
 ****************************************************************************************/
void Adafruit_seesaw::digitalWrite(uint8_t pin, uint8_t value)
{
	digitalWriteBulk(1ul << pin, value);
}


/**
 *****************************************************************************************
 *  @brief      Read the current status of a GPIO pin
 * 
 *  @param      pin the pin number. On the SAMD09 breakout, this corresponds to the number on the silkscreen.
 *
 *  @return     the status of the pin. HIGH or LOW (1 or 0).
 ****************************************************************************************/
bool Adafruit_seesaw::digitalRead(uint8_t pin)
{
	return digitalReadBulk((1ul << pin)) != 0;
}

/**
 *****************************************************************************************
 *  @brief      read the status of multiple pins.
 * 
 *  @param      pins a bitmask of the pins to write. On the SAMD09 breakout, this corresponds to the number on the silkscreen.
 *				For example, passing 0b0110 will return the values of pins 2 and 3.
 *
 *  @return     the status of the passed pins. If 0b0110 was passed and pin 2 is high and pin 3 is low, 0b0010 (decimal number 2) will be returned.
 ****************************************************************************************/
uint32_t Adafruit_seesaw::digitalReadBulk(uint32_t pins)
{
	uint8_t buf[4];
	this->read(SEESAW_GPIO_BASE, SEESAW_GPIO_BULK, buf, 4);
	uint32_t ret = ((uint32_t)buf[0] << 24) | ((uint32_t)buf[1] << 16) | ((uint32_t)buf[2] << 8) | (uint32_t)buf[3];
	return ret & pins;
}

/**
 *****************************************************************************************
 *  @brief      Enable or disable GPIO interrupts on the passed pins
 * 
 *  @param      pins a bitmask of the pins to write. On the SAMD09 breakout, this corresponds to the number on the silkscreen.
 *				For example, passing 0b0110 will enable or disable interrups on pins 2 and 3.
 *	@param		enabled pass true to enable the interrupts on the passed pins, false to disable the interrupts on the passed pins.
 *
 *  @return     none
 ****************************************************************************************/
void Adafruit_seesaw::setGPIOInterrupts(uint32_t pins, bool enabled)
{
	uint8_t cmd[] = { (uint8_t)(pins >> 24) , (uint8_t)(pins >> 16), (uint8_t)(pins >> 8), (uint8_t)pins };
	if(enabled)
		this->write(SEESAW_GPIO_BASE, SEESAW_GPIO_INTENSET, cmd, 4);
	else
		this->write(SEESAW_GPIO_BASE, SEESAW_GPIO_INTENCLR, cmd, 4);
}

/**
 *****************************************************************************************
 *  @brief      read the analog value on an ADC-enabled pin.
 * 
 *  @param      pin the number of the pin to read. On the SAMD09 breakout, this corresponds to the number on the silkscreen.
 *				On the default seesaw firmware on the SAMD09 breakout, pins 2, 3, and 4 are ADC-enabled.
 *
 *  @return     the analog value. This is an integer between 0 and 1023
 ****************************************************************************************/
uint16_t Adafruit_seesaw::analogRead(uint8_t pin)
{
	uint8_t buf[2];
	uint8_t p;
	switch(pin){
		case ADC_INPUT_0_PIN: p = 0; break;
		case ADC_INPUT_1_PIN: p = 1; break;
		case ADC_INPUT_2_PIN: p = 2; break;
		case ADC_INPUT_3_PIN: p = 3; break;
		default:
			return 0;
			break;
	}

	this->read(SEESAW_ADC_BASE, SEESAW_ADC_CHANNEL_OFFSET + p, buf, 2, 500);
	uint16_t ret = ((uint16_t)buf[0] << 8) | buf[1];
  	delay(1);
	return ret;
}

//TODO: not sure if this is how this is gonna work yet
void Adafruit_seesaw::analogReadBulk(uint16_t *buf, uint8_t num)
{
	uint8_t rawbuf[num * 2];
	this->read(SEESAW_ADC_BASE, SEESAW_ADC_CHANNEL_OFFSET, rawbuf, num * 2);
	for(int i=0; i<num; i++){
		buf[i] = ((uint16_t)rawbuf[i * 2] << 8) | buf[i * 2 + 1];
	}
}

/**
 *****************************************************************************************
 *  @brief      set the mode of multiple GPIO pins at once.
 * 
 *  @param      pins a bitmask of the pins to write. On the SAMD09 breakout, this corresponds to the number on the silkscreen.
 *				For example, passing 0b0110 will set the mode of pins 2 and 3.
 *	@param		mode the mode to set the pins to. One of INPUT, OUTPUT, or INPUT_PULLUP.
 *
 *  @return     none
 ****************************************************************************************/
void Adafruit_seesaw::pinModeBulk(uint32_t pins, uint8_t mode)
{
	uint8_t cmd[] = { (uint8_t)(pins >> 24) , (uint8_t)(pins >> 16), (uint8_t)(pins >> 8), (uint8_t)pins };
	switch (mode){
		case OUTPUT:
			this->write(SEESAW_GPIO_BASE, SEESAW_GPIO_DIRSET_BULK, cmd, 4);
			break;
		case INPUT:
			this->write(SEESAW_GPIO_BASE, SEESAW_GPIO_DIRCLR_BULK, cmd, 4);
			break;
		case INPUT_PULLUP:
			this->write(SEESAW_GPIO_BASE, SEESAW_GPIO_DIRCLR_BULK, cmd, 4);
			this->write(SEESAW_GPIO_BASE, SEESAW_GPIO_PULLENSET, cmd, 4);
			this->write(SEESAW_GPIO_BASE, SEESAW_GPIO_BULK_SET, cmd, 4);
			break;
	}
		
}

/**
 *****************************************************************************************
 *  @brief      write a value to multiple GPIO pins at once.
 * 
 *  @param      pins a bitmask of the pins to write. On the SAMD09 breakout, this corresponds to the number on the silkscreen.
 *				For example, passing 0b0110 will write the passed value to pins 2 and 3.
 *	@param		value pass HIGH to set the output on the passed pins to HIGH, low to set the output on the passed pins to LOW.
 *
 *  @return     none
 ****************************************************************************************/
void Adafruit_seesaw::digitalWriteBulk(uint32_t pins, uint8_t value)
{
	uint8_t cmd[] = { (uint8_t)(pins >> 24) , (uint8_t)(pins >> 16), (uint8_t)(pins >> 8), (uint8_t)pins };
	if(value)
		this->write(SEESAW_GPIO_BASE, SEESAW_GPIO_BULK_SET, cmd, 4);
	else
		this->write(SEESAW_GPIO_BASE, SEESAW_GPIO_BULK_CLR, cmd, 4);
}

/**
 *****************************************************************************************
 *  @brief      write a PWM value to a PWM-enabled pin
 * 
 *  @param      pin the number of the pin to write. On the SAMD09 breakout, this corresponds to the number on the silkscreen.
 *				on the default seesaw firmware on the SAMD09 breakout, pins 5, 6, and 7 are PWM enabled.
 *	@param		value the value to write to the pin
 *	@param		width the width of the value to write. Defaults to 8. If 16 is passed a 16 bit value will be written.
 *
 *  @return     none
 ****************************************************************************************/
void Adafruit_seesaw::analogWrite(uint8_t pin, uint16_t value, uint8_t width)
{
	int8_t p = -1;
	switch(pin){
		case PWM_0_PIN: p = 0; break;
		case PWM_1_PIN: p = 1; break;
		case PWM_2_PIN: p = 2; break;
		case PWM_3_PIN: p = 3; break;
		default:
			break;
	}
	if(p > -1){
		if(width == 16){
			uint8_t cmd[] = {(uint8_t)p, (uint8_t)(value >> 8), (uint8_t)value};
			this->write(SEESAW_TIMER_BASE, SEESAW_TIMER_PWM, cmd, 3);
		}
		else 
		{
			uint16_t mappedVal = map(value, 0, 255, 0, 65535);
			uint8_t cmd[] = {(uint8_t)p, (uint8_t)(mappedVal >> 8), (uint8_t)mappedVal};
			this->write(SEESAW_TIMER_BASE, SEESAW_TIMER_PWM, cmd, 3);
		}
	}
}

/**
 *****************************************************************************************
 *  @brief      set the PWM frequency of a PWM-enabled pin. Note that on SAMD09, SAMD11 boards
 *				the frequency will be mapped to closest match fixed frequencies.
 *				Also note that PWM pins 4 and 5 share a timer, and PWM pins 6 and 7 share a timer.
 *				Changing the frequency for one pin will change the frequency for the other pin that
 *				is on the timer.
 * 
 *  @param      pin the number of the pin to change frequency of. On the SAMD09 breakout, this corresponds to the number on the silkscreen.
 *				on the default seesaw firmware on the SAMD09 breakout, pins 5, 6, and 7 are PWM enabled.
 *	@param		freq the frequency to set.
 *
 *  @return     none
 ****************************************************************************************/
void Adafruit_seesaw::setPWMFreq(uint8_t pin, uint16_t freq)
{
	int8_t p = -1;
	switch(pin){
		case PWM_0_PIN: p = 0; break;
		case PWM_1_PIN: p = 1; break;
		case PWM_2_PIN: p = 2; break;
		case PWM_3_PIN: p = 3; break;
		default:
			break;
	}
	if(p > -1){
		uint8_t cmd[] = {(uint8_t)p, (uint8_t)(freq >> 8), (uint8_t)freq};
		this->write(SEESAW_TIMER_BASE, SEESAW_TIMER_FREQ, cmd, 3);
	}
}

/**
 *****************************************************************************************
 *  @brief      Enable the data ready interrupt on the passed sercom. Note that both the interrupt module and
 *				the passed sercom must be compiled into the seesaw firmware for this to function.
 *				If both of these things are true, the interrupt pin on the seesaw will fire when
 *				there is data to be read from the passed sercom. On the default seesaw firmeare
 *				on the SAMD09 breakout, no sercoms are enabled.
 * 
 *  @param      sercom the sercom to enable the interrupt on. 
 *
 *  @return     none
 ****************************************************************************************/
void Adafruit_seesaw::enableSercomDataRdyInterrupt(uint8_t sercom)
{
	_sercom_inten.DATA_RDY = 1;
	this->write8(SEESAW_SERCOM0_BASE + sercom, SEESAW_SERCOM_INTEN, _sercom_inten.get());
}

/**
 *****************************************************************************************
 *  @brief      Disable the data ready interrupt on the passed sercom.
 * 
 *  @param      sercom the sercom to disable the interrupt on. 
 *
 *  @return     none
 ****************************************************************************************/
void Adafruit_seesaw::disableSercomDataRdyInterrupt(uint8_t sercom)
{
	_sercom_inten.DATA_RDY = 0;
	this->write8(SEESAW_SERCOM0_BASE + sercom, SEESAW_SERCOM_INTEN, _sercom_inten.get());
}

/**
 *****************************************************************************************
 *  @brief      Reads a character from the passed sercom if one is available. Note that on
 *				the default seesaw firmware on the SAMD09 breakout no sercoms are enabled.
 * 
 *  @param      sercom the sercom to read data from.
 *
 *  @return     a character read from the sercom.
 ****************************************************************************************/
char Adafruit_seesaw::readSercomData(uint8_t sercom)
{
	return this->read8(SEESAW_SERCOM0_BASE + sercom, SEESAW_SERCOM_DATA);
}

/**
 *****************************************************************************************
 *  @brief      Set the seesaw I2C address. This will automatically call Adafruit_seesaw.begin()
 *				with the new address.
 * 
 *  @param      addr the new address for the seesaw. This must be a valid 7 bit I2C address.
 *
 *  @return     none
 ****************************************************************************************/
void Adafruit_seesaw::setI2CAddr(uint8_t addr)
{
  this->EEPROMWrite8(SEESAW_EEPROM_I2C_ADDR, addr);
  delay(250);
  this->begin(addr); //restart w/ the new addr
}


/**
 *****************************************************************************************
 *  @brief      Read the I2C address of the seesaw
 *
 *  @return     the 7 bit I2C address of the seesaw... which you probably already know because you
 *				just read data from it.
 ****************************************************************************************/
uint8_t Adafruit_seesaw::getI2CAddr()
{
  return this->read8(SEESAW_EEPROM_BASE, SEESAW_EEPROM_I2C_ADDR);
}

/**
 *****************************************************************************************
 *  @brief      Write a 1 byte to an EEPROM address
 * 
 *  @param      addr the address to write to. On the default seesaw firmware on the SAMD09
 *				breakout this is between 0 and 63.
 *	@param		val to write between 0 and 255
 *
 *  @return     none
 ****************************************************************************************/
void Adafruit_seesaw::EEPROMWrite8(uint8_t addr, uint8_t val)
{
  this->EEPROMWrite(addr, &val, 1);
}

/**
 *****************************************************************************************
 *  @brief      write a string of bytes to EEPROM starting at the passed address
 * 
 *  @param      addr the starting address to write the first byte. This will be automatically
 *				incremented with each byte written.
 *	@param		buf the buffer of bytes to be written.
 *	@param		size the number of bytes to write. Writing past the end of available EEPROM
 *				may result in undefined behavior.
 *
 *  @return     none
 ****************************************************************************************/
void Adafruit_seesaw::EEPROMWrite(uint8_t addr, uint8_t *buf, uint8_t size)
{
  this->write(SEESAW_EEPROM_BASE, addr, buf, size);
}

/**
 *****************************************************************************************
 *  @brief      Read 1 byte from the specified EEPROM address.
 * 
 *  @param      addr the address to read from. One the default seesaw firmware on the SAMD09
 *				breakout this is between 0 and 63.
 *
 *  @return     the value between 0 and 255 that was read from the passed address.
 ****************************************************************************************/
uint8_t Adafruit_seesaw::EEPROMRead8(uint8_t addr)
{
  return this->read8(SEESAW_EEPROM_BASE, addr);
}

/**
 *****************************************************************************************
 *  @brief      Set the baud rate on SERCOM0.
 * 
 *  @param      baud the baud rate to set. This is an integer value. Baud rates up to 115200 are supported.
 *
 *  @return     none
 ****************************************************************************************/
void Adafruit_seesaw::UARTSetBaud(uint32_t baud)
{
	uint8_t cmd[] = { (uint8_t)(baud >> 24), (uint8_t)(baud >> 16), (uint8_t)(baud >> 8), (uint8_t)baud };
	this->write(SEESAW_SERCOM0_BASE, SEESAW_SERCOM_BAUD, cmd, 4);
}

/**
 *****************************************************************************************
 *  @brief      Write 1 byte to the specified seesaw register.
 * 
 *  @param      regHigh the module address register (ex. SEESAW_NEOPIXEL_BASE)
 *	@param		regLow the function address register (ex. SEESAW_NEOPIXEL_PIN)
 *	@param		value the value between 0 and 255 to write
 *
 *  @return     none
 ****************************************************************************************/
void Adafruit_seesaw::write8(byte regHigh, byte regLow, byte value)
{
	this->write(regHigh, regLow, &value, 1);
}

/**
 *****************************************************************************************
 *  @brief      read 1 byte from the specified seesaw register.
 * 
 *  @param      regHigh the module address register (ex. SEESAW_STATUS_BASE)
 *	@param		regLow the function address register (ex. SEESAW_STATUS_VERSION)
 *
 *  @return     the value between 0 and 255 read from the passed register
 ****************************************************************************************/
uint8_t Adafruit_seesaw::read8(byte regHigh, byte regLow)
{
	uint8_t ret;
	this->read(regHigh, regLow, &ret, 1);
	
	return ret;
}

/**
 *****************************************************************************************
 *  @brief      Initialize I2C. On arduino this just calls Wire.begin()
 * 
 *
 *  @return     none
 ****************************************************************************************/
void Adafruit_seesaw::_i2c_init()
{
  Wire.begin();
}

/**
 *****************************************************************************************
 *  @brief      Read a specified number of bytes into a buffer from the seesaw.
 * 
 *  @param      regHigh the module address register (ex. SEESAW_STATUS_BASE)
 *	@param		regLow the function address register (ex. SEESAW_STATUS_VERSION)
 *	@param		buf the buffer to read the bytes into
 *	@param		num the number of bytes to read.
 *	@param		delay an optional delay in between setting the read register and reading
 *				out the data. This is required for some seesaw functions (ex. reading ADC data)
 *
 *  @return     none
 ****************************************************************************************/
void Adafruit_seesaw::read(uint8_t regHigh, uint8_t regLow, uint8_t *buf, uint8_t num, uint16_t delay)
{
	uint8_t value;
	uint8_t pos = 0;
	
	//on arduino we need to read in 32 byte chunks
	while(pos < num){
		
		uint8_t read_now = min(32, num - pos);
		Wire.beginTransmission((uint8_t)_i2caddr);
		Wire.write((uint8_t)regHigh);
		Wire.write((uint8_t)regLow);
		Wire.endTransmission();

		//TODO: tune this
		delayMicroseconds(delay);

		Wire.requestFrom((uint8_t)_i2caddr, read_now);
		
		for(int i=0; i<read_now; i++){
			buf[pos] = Wire.read();
			pos++;
		}
	}
}

/**
 *****************************************************************************************
 *  @brief      Write a specified number of bytes to the seesaw from the passed buffer.
 * 
 *  @param      regHigh the module address register (ex. SEESAW_GPIO_BASE)
 *	@param		regLow the function address register (ex. SEESAW_GPIO_BULK_SET)
 *	@param		buf the buffer the the bytes from
 *	@param		num the number of bytes to write.
 *
 *  @return     none
 ****************************************************************************************/
void Adafruit_seesaw::write(uint8_t regHigh, uint8_t regLow, uint8_t *buf, uint8_t num)
{ 
	Wire.beginTransmission((uint8_t)_i2caddr);
	Wire.write((uint8_t)regHigh);
	Wire.write((uint8_t)regLow);
	Wire.write((uint8_t *)buf, num);
	Wire.endTransmission();
}

/**
 *****************************************************************************************
 *  @brief      The print wrapper for the seesaw class. Calling this allows you to use
 *				ss.print() or ss.println() and write to the UART on SERCOM0 of the seesaw.
 *				Note that this functionality is only available when the UART (sercom) module
 *				is compiled into the seesaw firmware. On the default seesaw firmware on the
 *				SAMD09 breakout this functionality is not available.
 * 
 *  @param      character the character to write.
 *
 *  @return     none
 ****************************************************************************************/
size_t Adafruit_seesaw::write(uint8_t character) {
	//TODO: add support for multiple sercoms
	this->write8(SEESAW_SERCOM0_BASE, SEESAW_SERCOM_DATA, character);
	delay(1); //TODO: this can be optimized... it's only needed for longer writes
}

/**
 *****************************************************************************************
 *  @brief      The print wrapper for the seesaw class allowing the user to print a string. 
 *				Calling this allows you to use
 *				ss.print() or ss.println() and write to the UART on SERCOM0 of the seesaw.
 *				Note that this functionality is only available when the UART (sercom) module
 *				is compiled into the seesaw firmware. On the default seesaw firmware on the
 *				SAMD09 breakout this functionality is not available.
 * 
 *  @param      str the string to write
 *
 *  @return     none
 ****************************************************************************************/
size_t Adafruit_seesaw::write(const char *str) {
	uint8_t buf[32];
	uint8_t len = 0;
	while(*str){
		buf[len] = *str;
		str++;
		len++;
	}
	this->write(SEESAW_SERCOM0_BASE, SEESAW_SERCOM_DATA, buf, len);
	return len;
}

/**
 *****************************************************************************************
 *  @brief      Write only the module base address register and the function address register.
 * 
 *  @param      regHigh the module address register (ex. SEESAW_STATUS_BASE)
 *	@param		regLow the function address register (ex. SEESAW_STATUS_SWRST)
 *
 *  @return     none
 ****************************************************************************************/
void Adafruit_seesaw::writeEmpty(uint8_t regHigh, uint8_t regLow)
{
    Wire.beginTransmission((uint8_t)_i2caddr);
    Wire.write((uint8_t)regHigh);
    Wire.write((uint8_t)regLow);
    Wire.endTransmission();
}
