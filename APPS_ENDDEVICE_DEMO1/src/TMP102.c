/*
 * TMP102.c
 *
 * Created: 5/9/2023 4:51:12 AM
 *  Author: 1
 */ 

#include "TMP102.h"

//#include <Wire.h>

#define TEMPERATURE_REGISTER 0x00
#define CONFIG_REGISTER 0x01
#define T_LOW_REGISTER 0x02
#define T_HIGH_REGISTER 0x03

#define TMP102_address 0x48

//////////////////////////////////////////////////////////////////////////
// ********************** I2C module dirving *****************************

#define CONF_I2C_MODULE   					SERCOM1

struct i2c_master_module i2c_master_instance;
struct i2c_master_packet master_packet;

void   configure_i2c_master1(void)
{
	/* Create and initialize config structure */
	struct i2c_master_config config_i2c;
	i2c_master_get_config_defaults(&config_i2c);
	
	config_i2c.buffer_timeout = 1000;

	/* Change pins */
	config_i2c.pinmux_pad0  = PINMUX_PA16C_SERCOM1_PAD0;
	config_i2c.pinmux_pad1  = PINMUX_PA17C_SERCOM1_PAD1;

	/* Initialize and enable device with config */
	i2c_master_init(&i2c_master_instance, CONF_I2C_MODULE, &config_i2c);

	i2c_master_enable(&i2c_master_instance);
}

#define TIMEOUT						1000
#define SLAVE_WAIT_TIMEOUT			10

uint8_t kit_data[256];

static uint8_t write_buffer[2] = {
	0, 0,
};
uint8_t read_buffer[256];

uint32_t timeout = 0;
//////////////////////////////////////////////////////////////////////////
 
void TMP102_init(uint8_t address)
{
//	TMP102_address = address;
	
	// Initialize sensor0 settings
	// These settings are saved in the sensor, even if it loses power
	
	// set the number of consecutive faults before triggering alarm.
	// 0-3: 0:1 fault, 1:2 faults, 2:4 faults, 3:6 faults.
	TMP102_setFault(0);  // Trigger alarm immediately
	
	// set the polarity of the Alarm. (0:Active LOW, 1:Active HIGH).
	TMP102_setAlertPolarity(1); // Active HIGH
	
	// set the sensor in Comparator Mode (0) or Interrupt Mode (1).
	TMP102_setAlertMode(0); // Comparator Mode.
	
	// set the Conversion Rate (how quickly the sensor gets a new reading)
	//0-3: 0:0.25Hz, 1:1Hz, 2:4Hz, 3:8Hz
	TMP102_setConversionRate(2);
	
	//set Extended Mode.
	//0:12-bit Temperature(-55C to +128C) 1:13-bit Temperature(-55C to +150C)
	TMP102_setExtendedMode(0);

	//set T_HIGH, the upper limit to trigger the alert on
	TMP102_setHighTempF(85.0);  // set T_HIGH in F
	//sensor0.setHighTempC(29.4); // set T_HIGH in C
	
	//set T_LOW, the lower limit to shut turn off the alert
	TMP102_setLowTempF(84.0);  // set T_LOW in F
	//sensor0.setLowTempC(26.67); // set T_LOW in C
}

void TMP102_begin(void)
{

	configure_i2c_master1();
	
	/** Send the request token */
	master_packet.address         = TMP102_address;
	master_packet.data            = write_buffer;
	master_packet.data_length     = sizeof(write_buffer);	
	master_packet.ten_bit_address = false;
	master_packet.high_speed      = false;
	master_packet.hs_master_code  = 0x0;
	
}



void TMP102_openPointerRegister(uint8_t pointerReg)
{
	
	write_buffer[0] = pointerReg;
	
	master_packet.address = TMP102_address;
	master_packet.data = write_buffer;
	master_packet.data_length = 1;
	while (i2c_master_write_packet_wait_no_stop(&i2c_master_instance, &master_packet) != STATUS_OK) {
		/* Increment timeout counter and check if timed out. */
		if (timeout++ == TIMEOUT) {
			return;
		}
	}
	
}


uint8_t TMP102_readRegister(bool registerNumber){
	uint8_t registerByte[2];	// We'll store the data from the registers here
	
	master_packet.address = TMP102_address;
	master_packet.data = read_buffer;
	master_packet.data_length = 2;
	while (i2c_master_read_packet_wait(&i2c_master_instance, &master_packet) != STATUS_OK) {
		/* Increment timeout counter and check if timed out. */
		if (timeout++ == TIMEOUT) {
			return;
		}
	}
	
	registerByte[0] = read_buffer[0];	// Read first byte
	registerByte[1] = read_buffer[1];	// Read second byte
	
	
	return registerByte[registerNumber];
}


float TMP102_readTempC(void)
{
	uint8_t registerByte[2];	// Store the data from the register here
	int16_t digitalTemp;  // Temperature stored in TMP102 register
	
	// Read Temperature
	// Change pointer address to temperature register (0)
	TMP102_openPointerRegister(TEMPERATURE_REGISTER);
	// Read from temperature register
	registerByte[0] = TMP102_readRegister(0);
	registerByte[1] = TMP102_readRegister(1);

	// Bit 0 of second byte will always be 0 in 12-bit readings and 1 in 13-bit
	if(registerByte[1]&0x01)	// 13 bit mode
	{
		// Combine bytes to create a signed int
		digitalTemp = ((registerByte[0]) << 5) | (registerByte[1] >> 3);
		// Temperature data can be + or -, if it should be negative,
		// convert 13 bit to 16 bit and use the 2s compliment.
		if(digitalTemp > 0xFFF)
		{
			digitalTemp |= 0xE000;
		}
	}
	else	// 12 bit mode
	{
		// Combine bytes to create a signed int
		digitalTemp = ((registerByte[0]) << 4) | (registerByte[1] >> 4);
		// Temperature data can be + or -, if it should be negative,
		// convert 12 bit to 16 bit and use the 2s compliment.
		if(digitalTemp > 0x7FF)
		{
			digitalTemp |= 0xF000;
		}
	}
	// Convert digital reading to analog temperature (1-bit is equal to 0.0625 C)
	return digitalTemp*0.0625;
}


float TMP102_readTempF(void)
{
	return TMP102_readTempC()*9.0/5.0 + 32.0;
}


void TMP102_setConversionRate(uint8_t rate)
{
	uint8_t registerByte[2]; // Store the data from the register here
	rate = rate&0x03; // Make sure rate is not set higher than 3.
	
	// Change pointer address to configuration register (0x01)
	TMP102_openPointerRegister(CONFIG_REGISTER);
	
	// Read current configuration register value
	registerByte[0] = TMP102_readRegister(0);
	registerByte[1] = TMP102_readRegister(1);

	// Load new conversion rate
	registerByte[1] &= 0x3F;  // Clear CR0/1 (bit 6 and 7 of second byte)
	registerByte[1] |= rate<<6;	// Shift in new conversion rate

	// Set configuration registers
	write_buffer[0] = CONFIG_REGISTER;
	write_buffer[1] = registerByte[0];
	write_buffer[2] = registerByte[1];
	
	master_packet.address = TMP102_address;
	master_packet.data = write_buffer;
	master_packet.data_length = 3;
	while (i2c_master_write_packet_wait_no_stop(&i2c_master_instance, &master_packet) != STATUS_OK) {
		/* Increment timeout counter and check if timed out. */
		if (timeout++ == TIMEOUT) {
			return;
		}
	}
	
}


void TMP102_setExtendedMode(bool mode)
{
	uint8_t registerByte[2]; // Store the data from the register here

	// Change pointer address to configuration register (0x01)
	TMP102_openPointerRegister(CONFIG_REGISTER);
	
	// Read current configuration register value
	registerByte[0] = TMP102_readRegister(0);
	registerByte[1] = TMP102_readRegister(1);

	// Load new value for extention mode
	registerByte[1] &= 0xEF;		// Clear EM (bit 4 of second byte)
	registerByte[1] |= mode<<4;	// Shift in new exentended mode bit

	// Set configuration registers
	write_buffer[0] = CONFIG_REGISTER;
	write_buffer[1] = registerByte[0];
	write_buffer[2] = registerByte[1];
	
	master_packet.address = TMP102_address;
	master_packet.data = write_buffer;
	master_packet.data_length = 3;
	while (i2c_master_write_packet_wait_no_stop(&i2c_master_instance, &master_packet) != STATUS_OK) {
		/* Increment timeout counter and check if timed out. */
		if (timeout++ == TIMEOUT) {
			return;
		}
	}
}


void TMP102_sleep(void)
{
	uint8_t registerByte; // Store the data from the register here

	// Change pointer address to configuration register (0x01)
	TMP102_openPointerRegister(CONFIG_REGISTER);
	
	// Read current configuration register value
	registerByte = TMP102_readRegister(0);

	registerByte |= 0x01;	// Set SD (bit 0 of first byte)

	// Set configuration register	
	write_buffer[0] = CONFIG_REGISTER;
	write_buffer[1] = registerByte;
	
	master_packet.address = TMP102_address;
	master_packet.data = write_buffer;
	master_packet.data_length = 2;
	while (i2c_master_write_packet_wait_no_stop(&i2c_master_instance, &master_packet) != STATUS_OK) {
		/* Increment timeout counter and check if timed out. */
		if (timeout++ == TIMEOUT) {
			return;
		}
	}
}


void TMP102_wakeup(void)
{
	uint8_t registerByte; // Store the data from the register here

	// Change pointer address to configuration register (1)
	TMP102_openPointerRegister(CONFIG_REGISTER);
	
	// Read current configuration register value
	registerByte = TMP102_readRegister(0);

	registerByte &= 0xFE;	// Clear SD (bit 0 of first byte)

	// Set configuration registers	
	write_buffer[0] = CONFIG_REGISTER;
	write_buffer[1] = registerByte;
	
	master_packet.address = TMP102_address;
	master_packet.data = write_buffer;
	master_packet.data_length = 2;
	while (i2c_master_write_packet_wait_no_stop(&i2c_master_instance, &master_packet) != STATUS_OK) {
		/* Increment timeout counter and check if timed out. */
		if (timeout++ == TIMEOUT) {
			return;
		}
	}
}


void TMP102_setAlertPolarity(bool polarity)
{
	uint8_t registerByte; // Store the data from the register here

	// Change pointer address to configuration register (1)
	TMP102_openPointerRegister(CONFIG_REGISTER);
	
	// Read current configuration register value
	registerByte = TMP102_readRegister(0);

	// Load new value for polarity
	registerByte &= 0xFB; // Clear POL (bit 2 of registerByte)
	registerByte |= polarity<<2;  // Shift in new POL bit

	// Set configuration register	
	write_buffer[0] = CONFIG_REGISTER;
	write_buffer[1] = registerByte;
	
	master_packet.address = TMP102_address;
	master_packet.data = write_buffer;
	master_packet.data_length = 2;
	while (i2c_master_write_packet_wait_no_stop(&i2c_master_instance, &master_packet) != STATUS_OK) {
		/* Increment timeout counter and check if timed out. */
		if (timeout++ == TIMEOUT) {
			return;
		}
	}
}


bool TMP102_alert(void)
{
	uint8_t registerByte; // Store the data from the register here

	// Change pointer address to configuration register (1)
	TMP102_openPointerRegister(CONFIG_REGISTER);
	
	// Read current configuration register value
	registerByte = TMP102_readRegister(1);
	
	registerByte &= 0x20;	// Clear everything but the alert bit (bit 5)
	return registerByte>>5;
}


void TMP102_setLowTempC(float temperature)
{
	uint8_t registerByte[2];	// Store the data from the register here
	bool extendedMode;	// Store extended mode bit here 0:-55C to +128C, 1:-55C to +150C
	
	// Prevent temperature from exceeding 150C or -55C
	if(temperature > 150.0f)
	{
		temperature = 150.0f;
	}
	if(temperature < -55.0)
	{
		temperature = -55.0f;
	}
	
	//Check if temperature should be 12 or 13 bits
	TMP102_openPointerRegister(CONFIG_REGISTER);	// Read configuration register settings
	
	// Read current configuration register value
	registerByte[0] = TMP102_readRegister(0);
	registerByte[1] = TMP102_readRegister(1);
	extendedMode = (registerByte[1]&0x10)>>4;	// 0 - temp data will be 12 bits
	// 1 - temp data will be 13 bits

	// Convert analog temperature to digital value
	temperature = temperature/0.0625;
	
	// Split temperature into separate bytes
	if(extendedMode)	// 13-bit mode
	{
		registerByte[0] = (int)(temperature)>>5;
		registerByte[1] = (int)(temperature)<<3;
	}
	else	// 12-bit mode
	{
		registerByte[0] = (int)(temperature)>>4;
		registerByte[1] = (int)(temperature)<<4;
	}
	
	// Write to T_LOW Register	
	write_buffer[0] = T_LOW_REGISTER;
	write_buffer[1] = registerByte[0];
	write_buffer[2] = registerByte[1];
	
	master_packet.address = TMP102_address;
	master_packet.data = write_buffer;
	master_packet.data_length = 3;
	while (i2c_master_write_packet_wait_no_stop(&i2c_master_instance, &master_packet) != STATUS_OK) {
		/* Increment timeout counter and check if timed out. */
		if (timeout++ == TIMEOUT) {
			return;
		}
	}
}


void TMP102_setHighTempC(float temperature)
{
	uint8_t registerByte[2];	// Store the data from the register here
	bool extendedMode;	// Store extended mode bit here 0:-55C to +128C, 1:-55C to +150C
	
	// Prevent temperature from exceeding 150C
	if(temperature > 150.0f)
	{
		temperature = 150.0f;
	}
	if(temperature < -55.0)
	{
		temperature = -55.0f;
	}
	
	// Check if temperature should be 12 or 13 bits
	TMP102_openPointerRegister(CONFIG_REGISTER);	// Read configuration register settings
	
	// Read current configuration register value
	registerByte[0] = TMP102_readRegister(0);
	registerByte[1] = TMP102_readRegister(1);
	extendedMode = (registerByte[1]&0x10)>>4;	// 0 - temp data will be 12 bits
	// 1 - temp data will be 13 bits

	// Convert analog temperature to digital value
	temperature = temperature/0.0625;
	
	// Split temperature into separate bytes
	if(extendedMode)	// 13-bit mode
	{
		registerByte[0] = (int)(temperature)>>5;
		registerByte[1] = (int)(temperature)<<3;
	}
	else	// 12-bit mode
	{
		registerByte[0] = (int)(temperature)>>4;
		registerByte[1] = (int)(temperature)<<4;
	}
	
	// Write to T_HIGH Register	
	write_buffer[0] = T_HIGH_REGISTER;
	write_buffer[1] = registerByte[0];
	write_buffer[2] = registerByte[1];
	
	master_packet.address = TMP102_address;
	master_packet.data = write_buffer;
	master_packet.data_length = 3;
	while (i2c_master_write_packet_wait_no_stop(&i2c_master_instance, &master_packet) != STATUS_OK) {
		/* Increment timeout counter and check if timed out. */
		if (timeout++ == TIMEOUT) {
			return;
		}
	}
}


void TMP102_setLowTempF(float temperature)
{
	temperature = (temperature - 32)*5/9; // Convert temperature to C
	TMP102_setLowTempC(temperature); // Set T_LOW
}


void TMP102_setHighTempF(float temperature)
{
	temperature = (temperature - 32)*5/9; // Convert temperature to C
	TMP102_setHighTempC(temperature); // Set T_HIGH
}


float TMP102_readLowTempC(void)
{
	uint8_t registerByte[2];	// Store the data from the register here
	bool extendedMode;	// Store extended mode bit here 0:-55C to +128C, 1:-55C to +150C
	int16_t digitalTemp;		// Store the digital temperature value here
//	float temperature;	// Store the analog temperature value here
	
	// Check if temperature should be 12 or 13 bits
	TMP102_openPointerRegister(CONFIG_REGISTER);	// Read configuration register settings
	// Read current configuration register value
	registerByte[0] = TMP102_readRegister(0);
	registerByte[1] = TMP102_readRegister(1);
	extendedMode = (registerByte[1]&0x10)>>4;	// 0 - temp data will be 12 bits
	// 1 - temp data will be 13 bits
	TMP102_openPointerRegister(T_LOW_REGISTER);
	registerByte[0] = TMP102_readRegister(0);
	registerByte[1] = TMP102_readRegister(1);
	
	if(extendedMode)	// 13 bit mode
	{
		// Combine bytes to create a signed int
		digitalTemp = ((registerByte[0]) << 5) | (registerByte[1] >> 3);
		// Temperature data can be + or -, if it should be negative,
		// convert 13 bit to 16 bit and use the 2s compliment.
		if(digitalTemp > 0xFFF)
		{
			digitalTemp |= 0xE000;
		}
	}
	else	// 12 bit mode
	{
		// Combine bytes to create a signed int
		digitalTemp = ((registerByte[0]) << 4) | (registerByte[1] >> 4);
		// Temperature data can be + or -, if it should be negative,
		// convert 12 bit to 16 bit and use the 2s compliment.
		if(digitalTemp > 0x7FF)
		{
			digitalTemp |= 0xF000;
		}
	}
	// Convert digital reading to analog temperature (1-bit is equal to 0.0625 C)
	return digitalTemp*0.0625;
}


float TMP102_readHighTempC(void)
{
	uint8_t registerByte[2];	// Store the data from the register here
	bool extendedMode;	// Store extended mode bit here 0:-55C to +128C, 1:-55C to +150C
	int16_t digitalTemp;		// Store the digital temperature value here
//	float temperature;	// Store the analog temperature value here
	
	// Check if temperature should be 12 or 13 bits
	TMP102_openPointerRegister(CONFIG_REGISTER);	// read configuration register settings
	// Read current configuration register value
	registerByte[0] = TMP102_readRegister(0);
	registerByte[1] = TMP102_readRegister(1);
	extendedMode = (registerByte[1]&0x10)>>4;	// 0 - temp data will be 12 bits
	// 1 - temp data will be 13 bits
	TMP102_openPointerRegister(T_HIGH_REGISTER);
	registerByte[0] = TMP102_readRegister(0);
	registerByte[1] = TMP102_readRegister(1);
	
	if(extendedMode)	// 13 bit mode
	{
		// Combine bytes to create a signed int
		digitalTemp = ((registerByte[0]) << 5) | (registerByte[1] >> 3);
		// Temperature data can be + or -, if it should be negative,
		// convert 13 bit to 16 bit and use the 2s compliment.
		if(digitalTemp > 0xFFF)
		{
			digitalTemp |= 0xE000;
		}
	}
	else	// 12 bit mode
	{
		// Combine bytes to create a signed int
		digitalTemp = ((registerByte[0]) << 4) | (registerByte[1] >> 4);
		// Temperature data can be + or -, if it should be negative,
		// convert 12 bit to 16 bit and use the 2s compliment.
		if(digitalTemp > 0x7FF)
		{
			digitalTemp |= 0xF000;
		}
	}
	// Convert digital reading to analog temperature (1-bit is equal to 0.0625 C)
	return digitalTemp*0.0625;
}


float TMP102_readLowTempF(void)
{
	return TMP102_readLowTempC()*9.0/5.0 + 32.0;
}


float TMP102_readHighTempF(void)
{
	return TMP102_readHighTempC()*9.0/5.0 + 32.0;
}


void TMP102_setFault(uint8_t faultSetting)
{
	uint8_t registerByte; // Store the data from the register here

	faultSetting = faultSetting&3; // Make sure rate is not set higher than 3.
	
	// Change pointer address to configuration register (0x01)
	TMP102_openPointerRegister(CONFIG_REGISTER);
	
	// Read current configuration register value
	registerByte = TMP102_readRegister(0);

	// Load new conversion rate
	registerByte &= 0xE7;  // Clear F0/1 (bit 3 and 4 of first byte)
	registerByte |= faultSetting<<3;	// Shift new fault setting

	// Set configuration registers	
	write_buffer[0] = CONFIG_REGISTER;
	write_buffer[1] = registerByte;
	
	master_packet.address = TMP102_address;
	master_packet.data = write_buffer;
	master_packet.data_length = 2;
	while (i2c_master_write_packet_wait_no_stop(&i2c_master_instance, &master_packet) != STATUS_OK) {
		/* Increment timeout counter and check if timed out. */
		if (timeout++ == TIMEOUT) {
			return;
		}
	}
	
}


void TMP102_setAlertMode(bool mode)
{
	uint8_t registerByte; // Store the data from the register here
	
	// Change pointer address to configuration register (1)
	TMP102_openPointerRegister(CONFIG_REGISTER);
	
	// Read current configuration register value
	registerByte = TMP102_readRegister(0);

	// Load new conversion rate
	registerByte &= 0xFD;	// Clear old TM bit (bit 1 of first byte)
	registerByte |= mode<<1;	// Shift in new TM bit

	// Set configuration registers	
	write_buffer[0] = CONFIG_REGISTER;
	write_buffer[1] = registerByte;
	
	master_packet.address = TMP102_address;
	master_packet.data = write_buffer;
	master_packet.data_length = 2;
	while (i2c_master_write_packet_wait_no_stop(&i2c_master_instance, &master_packet) != STATUS_OK) {
		/* Increment timeout counter and check if timed out. */
		if (timeout++ == TIMEOUT) {
			return;
		}
	}
}