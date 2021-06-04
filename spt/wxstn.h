#ifndef WXSTN_H
#define WXSTN_H
#include <time.h>

// Constants

// Simulation Constants
#define SIMULATE 1
#define SIMTEMP 0
#define SIMHUMID 0
#define SIMPRESS 0
#define SIMLIGHT 1
#define SIMWINDSPD 1
#define SIMWINDDIR 1
#define USTEMP 50
#define LSTEMP -10
#define USHUMID 100
#define LSHUMID 0
#define USPRESS 1016
#define LSPRESS 985
#define USLIGHT 5000
#define LSLIGHT 0
#define USWINDSPD 100
#define LSWINDSPD 0
#define USWINDDIR 360
#define LSWINDDIR 0

// I2C Device Addresses
#define HSH_RTC_I2CADR		0x68
#define HSH_MCP23017_I2CADR	0x20
#define HSH_PCF8591_I2CADR	0x48
#define HSH_BME280_I2CADR	0x76

// wiringPi Device Pin Base
#define HSH_MCP23017_PINBASE	100
#define HSH_PCF8591_PINBASE	200
#define HSH_BME280_PINBASE	300

// Structures
typedef struct readings
{
	time_t rtime;
	double temperature;
	double humidity;
	double pressure;
    double light;
	double windspeed;
	double winddirection;
}reading_s;

// Function Prototypes
int WsInit (void);
reading_s WsGetReadings(void);
double WsGetTemperature(void);
double WsGetHumidity(void);
double WsGetPressure(void);
double WsGetLight(void);
double WsGetWindspeed(void);
double WsGetWinddirection(void);
int WsGetRandom(int range);
double PaTomB(double pa);
void WsDisplayReadings(reading_s dreads);
void WsDelay(int milliseconds);

#endif
