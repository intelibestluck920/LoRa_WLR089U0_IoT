/*
 * sensor_data.c
 *
 * Created: 10/3/2019 4:38:34 PM
 *  Author: Vivek Anchalia - Microchip Technology Inc
 */ 

#include "sensor_data.h"
#include "sio2host.h"
#include "conf_app.h"

//#if defined MISOL_WEATHER_STATION
#define IDENTIFY_TYPE  0x24
uint8_t rawSensorData[7];
//function to receive complete serial data from weather station
bool serialGetSensorData(void)
{
	// variable to store first byte coming from weather station
	uint8_t identifyByte;
	// Sensor Data + CRC + CheckSum
	int i = 1;
//	identifyByte = sio2host_getchar();
	identifyByte = 0x24;
	rawSensorData[0] = identifyByte;
	
	uint8_t ss = 0;
	for (i = 1; i < 7; i++)
	{
		rawSensorData[i] = i*10;
		ss = ss + rawSensorData[i];
	}
	
	rawSensorData[7] = ss + identifyByte;
	
	if (identifyByte == IDENTIFY_TYPE)
	{
		/*
		int rxChar;
		while(i < 7)
		{
			if((-1) != (rxChar = sio2host_getchar_nowait()))
			{
				rawSensorData[i++] = rxChar;
			}
		}
		rawSensorData[0] = identifyByte;
		#if defined (ENABLE_SENSOR_DEBUG)
			printf("\r\n..rawSensorDataReceived..\r\n");
		#endif
		uint8_t sum = 0;
		// checksum check
		// According to MISOL WEATHER STATUION PROTOCOL sum of 6 bytes received = 6th byte
		for (i = 0; i < 6; i++)
		{
			sum = sum + rawSensorData[i];
		}
		if (sum == rawSensorData[6])
		return true;
		else return false;
		*/
		
		uint8_t sum = 0;
		for (i =0; i < 7; i++)
		{
			sum = sum + rawSensorData[i];
		}
		if (sum == rawSensorData[7])
			return true;
		else return false;
	}
	else
		return false;

}

uint16_t extractVoltage(void)
{
	uint16_t wd = 0;
	// Voltage (V) is a combination from Nibbles 5th - 7th values
	if (rawSensorData[0] == 0x24)
	{
		wd = (uint16_t) (rawSensorData[2]) + ((uint16_t) ((rawSensorData[3] & 80)) << 1);
		#if defined (ENABLE_SENSOR_DEBUG)
		printf("\r\nWind Direction(deg): %d \r\n",wd);
		#endif
	}
	return wd;
}

extern int transmission_interval;

double extractTemperature(void)
{
	double temperature = 0;
	
	// read temperature data
	//temperature = TMP102_readHighTempF();
	temperature = TMP102_readTempC();
	
	if (temperature <= 24.5 && temperature >= 23.5)
		transmission_interval = CUSTOM_APP_SLEEP_TIME_MS_1;
	else if (temperature >= 25.5 && temperature <= 26.5)
		transmission_interval = CUSTOM_APP_SLEEP_TIME_MS_2;
	else
		transmission_interval = DEMO_CONF_DEFAULT_APP_SLEEP_TIME_MS;
	
	return temperature;
}