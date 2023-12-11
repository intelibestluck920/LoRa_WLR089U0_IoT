/*
 * lpp.c
 *
 * Created: 10/5/2019 4:43:56 PM
 *  Author: Vivek Anchalia - Microchip Technology Inc
 */
#include <asf.h> 
#include <lpp.h>
uint8_t *buffer;
uint8_t maxsize;
uint8_t cursor;
	
void CayenneLPP(uint8_t size)	
//Initialize the payload buffer with the given maximum size.
{
	buffer = (uint8_t*) malloc(size);
	cursor = 0;
	maxsize = 200;
}

void FreeCayenneLPP(void)
{
	cursor = 0;
}

void reset(void){
	cursor = 0;
}
uint8_t getSize(void){
	return cursor;
}
uint8_t* getBuffer(void){
	return buffer;
}

// Analog voltage
uint8_t addAnalogInput(uint8_t channel, float value){
    if ((cursor + LPP_ANALOG_INPUT_SIZE) > maxsize) {
	    return 0;
    }
    
    int16_t val = value * 100;
    buffer[cursor++] = channel;
    buffer[cursor++] = LPP_ANALOG_INPUT;
 //   buffer[cursor++] = val >> 8;
 //   buffer[cursor++] = val;
	
	buffer[cursor++] = 128;
	buffer[cursor++] = 1;

    return cursor;
}
// Temperature
uint8_t addTemperature(uint8_t channel, float celsius)
{
	if ((cursor + LPP_TEMPERATURE_SIZE) > maxsize) {
		return 0;
	}
	int16_t val = celsius * 10;
	buffer[cursor++] = channel;
	buffer[cursor++] = LPP_TEMPERATURE;
	
//	buffer[cursor++] = val >> 8;
//	buffer[cursor++] = val;

	buffer[cursor++] = 128;
	buffer[cursor++] = 129;

	return cursor;
}