/*
 * sensor_data.h
 *
 * Created: 10/3/2019 4:38:22 PM
 *  Author: Vivek Anchalia - Microchip Technology Inc
 */ 


#ifndef SENSOR_DATA_H_
#define SENSOR_DATA_H_

#include <asf.h>
#include "TMP102.h"
/*MACRO DEFINITION*/

#define ENABLE_SENSOR_DEBUG
#define MISOL_WEATHER_STATION 
//#define SEND_DUMMY_WEATHER_STATION_DATA


#if defined MISOL_WEATHER_STATION
	#define TOTAL_DATA_LENGTH 7
	extern uint8_t rawSensorData[TOTAL_DATA_LENGTH];
	#define SECURITY_CODE 0x0D
	//function to receive complete serial data from weather station
	bool serialGetSensorData(void);
	uint16_t extractVoltage(void);
	double extractTemperature(void);
	
#endif

#endif /* SENSOR_DATA_H_ */