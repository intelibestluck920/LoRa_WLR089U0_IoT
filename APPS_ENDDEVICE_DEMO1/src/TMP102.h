/*
 * TMP102.h
 *
 * Created: 5/9/2023 4:50:18 AM
 *  Author: 1
 */ 


#ifndef TMP102_H_
#define TMP102_H_

#include <string.h>
#include <asf.h>

//////////////////////////////////////////////////////////////////////////
//********************** I2C modue driving *******************************
void   configure_i2c_master1(void);

//////////////////////////////////////////////////////////////////////////

void TMP102_init(uint8_t address);	// Initialize TMP102 sensor at given address
void TMP102_begin(void);  // Join I2C bus
float TMP102_readTempC(void);	// Returns the temperature in degrees C
float TMP102_readTempF(void);	// Converts readTempC result to degrees F
void TMP102_sleep(void);	// Switch sensor to low power mode
void TMP102_wakeup(void);	// Wakeup and start running in normal power mode
bool TMP102_alert(void);	// Returns state of Alert register
void TMP102_setLowTempC(float temperature);  // Sets T_LOW (degrees C) alert threshold
void TMP102_setHighTempC(float temperature); // Sets T_HIGH (degrees C) alert threshold
void TMP102_setLowTempF(float temperature);  // Sets T_LOW (degrees F) alert threshold
void TMP102_setHighTempF(float temperature); // Sets T_HIGH (degrees F) alert threshold
float TMP102_readLowTempC(void);	// Reads T_LOW register in C
float TMP102_readLowTempF(void);	// Reads T_LOW register in F
float TMP102_readHighTempC(void);	// Reads T_HIGH register in C
float TMP102_readHighTempF(void);	// Reads T_HIGH register in F
		
// Set the conversion rate (0-3)
// 0 - 0.25 Hz
// 1 - 1 Hz
// 2 - 4 Hz (default)
// 3 - 8 Hz
void TMP102_setConversionRate(uint8_t rate);
		
// Enable or disable extended mode
// 0 - disabled (-55C to +128C)
// 1 - enabled  (-55C to +150C)
void TMP102_setExtendedMode(bool mode);
		
// Set the polarity of Alert
// 0 - Active LOW
// 1 - Active HIGH
void TMP102_setAlertPolarity(bool polarity);
		
// Set the number of consecutive faults
// 0 - 1 fault
// 1 - 2 faults
// 2 - 4 faults
// 3 - 6 faults
void TMP102_setFault(uint8_t faultSetting);
		
// Set Alert type
// 0 - Comparator Mode: Active from temp > T_HIGH until temp < T_LOW
// 1 - Thermostat Mode: Active when temp > T_HIGH until any read operation occurs
void TMP102_setAlertMode(bool mode);
		

int TMP102_address; // Address of Temperature sensor (0x48,0x49,0x4A,0x4B)
void TMP102_openPointerRegister(uint8_t pointerReg); // Changes the pointer register
uint8_t TMP102_readRegister(bool registerNumber);	// reads 1 byte of from register
	


#endif /* TMP102_H_ */