//
// Weather Station Control
//
//
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "wxstn.h"
#include "hshbme280.h"

int WsInit (void)
{
    BME280Setup();
    return 1;
}

reading_s WsGetReadings(void)
{
	reading_s now = {0};

	now.rtime = time(NULL);
	now.temperature = WsGetTemperature();
	now.humidity = WsGetHumidity();
	now.pressure = WsGetPressure();
	now.light = WsGetLight();
	now.windspeed = WsGetWindspeed();
	now.winddirection = WsGetWinddirection();
	return now;
}

double WsGetTemperature(void)
{
#if SIMTEMP
	return (double) WsGetRandom(USTEMP-LSTEMP)+LSTEMP;
#else
	return GetBME280TempC();
#endif
}

double WsGetHumidity(void)
{
#if SIMHUMID
	return (double) WsGetRandom(USHUMID-LSHUMID)+LSHUMID;
#else
	return GetBME280Humidity();
#endif
}

double WsGetPressure(void)
{
#if SIMPRESS
	return (double) WsGetRandom(USPRESS-LSPRESS)+LSPRESS;
#else
	return PaTomB(GetBME280Pressure());
#endif
}

double WsGetLight(void)
{
#if SIMLIGHT
	return (double) WsGetRandom(USLIGHT-LSLIGHT)+LSLIGHT;
#else
	return 0.0;
#endif
}

double WsGetWindspeed(void)
{
#if SIMWINDSPD
    return (double) WsGetRandom(USWINDSPD-LSWINDSPD)+LSWINDSPD;
#else
	return 20.0;
#endif
}

double WsGetWinddirection(void)
{
#if SIMWINDDIR
    return (double) WsGetRandom(USWINDDIR-LSWINDDIR)+LSWINDDIR;
#else
	return 320.0;
#endif
}

int WsGetRandom(int range)
{
	return rand() % range;
}

double PaTomB(double pa)
{
	return pa/100.0;
}

void WsDisplayReadings(reading_s dreads)
{
    time_t now;

    now = time(NULL);

    printf("\n%s",ctime(&now));
    printf("Readings\t T: %3.1lfC\t H: %3.0lf%%\t P: %5.1lfmb\t WS: %3.0lfkmh\t WD: %3.0lfdegrees\n",
            dreads.temperature,dreads.humidity,dreads.pressure,dreads.windspeed,dreads.winddirection);
}

void WsDelay(int milliseconds)
{
    long wait;
    clock_t now,start;

    wait = milliseconds*(CLOCKS_PER_SEC/1000);
    start = clock();
    now = start;

    while((now-start) < wait)
    {
        now = clock();
    }
}
