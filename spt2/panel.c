#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <pcf8591.h>
#include <pigpio.h>
#include <time.h>
#include <string.h>
#include "spa.h"
#include "tsl2561.h"
#include "panel.h"
#include "wxstn.h"
#include "gps.h"

//static positiondata_s positiontable = {0};
positiondata_s positiontable[STMAXPSZ] = {0};

int StPanelInitialization(void)
{
    int i;

#if SIMULATE
    srand(time(NULL));
#endif
    wiringPiSetup();
    pcf8591Setup(ST_PCF8591_PINBASE,ST_PCF8591_I2CADR);
    WsInit();
    tsl2561Setup();
    StServoSetup();
    gps_init();

    i = StRetrievePositionTable();   //check to see if this is right
    if (i ==0)
    {
        for(i=0; i<STMAXPSZ; i++)
        {
            positiontable[i].apos = (int) ((double)i/180.0*(STPA180-STPA000))+STPA000;
            positiontable[i].epos = (int) ((double)i/180.0*(STPE180-STPE000))+STPE000;
            positiontable[i].spwm = (int) ((double)i/180.0*(STSPWMMAX-STSPWMMIN))+STSPWMMIN;
            positiontable[i].hpwm = (int) ((double)i/180.0*(STPWMMAX-STPWMMIN))+STPWMMIN;
        }
    }

    i = StSavePositionTable();
    return 1;
}

void StServoSetup(void)
{
    // Hardware PWM for elevation
    pinMode(STELPIN,PWM_OUTPUT);
    pwmSetMode(PWM_MODE_MS);
    pwmSetClock(STPWMCLOCK);
    pwmSetRange(STPWMRANGE);

    // pigpio PWM for azimuth
    int i;
    i = gpioInitialise();
    gpioSetPWMfrequency(STAZPIN,STPWMCLOCK);
    gpioSetPWMrange(STAZPIN,STSPWMRANGE);
}

ldrsensor_s StGetLdrReadings(void)
{
    ldrsensor_s csens = {0};

#if SIMLDR
    csens.aset = STSACTR;
    csens.eset = STSECTR;
#else
    csens.aset = analogRead(ST_PCF8591_PINBASE+2);
    csens.eset = analogRead(ST_PCF8591_PINBASE+3);
#endif
    return csens;
}

void StDisplayLdrReadings(ldrsensor_s dsens)
{
    printf("Elevation LDR: %3d  Azimuth LDR: %3d\n",dsens.eset, dsens.aset);
}

panelpos_s StGetPanelPosition(void)
{
    panelpos_s cpos = {0.0};
    double el,az;

    el = analogRead(ST_PCF8591_PINBASE+0)-STPE000;
    az = analogRead(ST_PCF8591_PINBASE+1)-STPA000;
    if(el<0.0){el=0.0;}
    if(az<0.0){az=0.0;}
	cpos.Elevation = el*180.0/(STPE180-STPE000);
	cpos.Azimuth = az*180/(STPA180-STPA000);
	
    return cpos;
}

positiondata_s StGetPositionData(panelpos_s cpos)
{
    positiondata_s pdata = {0};
    int paindex,peindex;

    paindex = (int) cpos.Azimuth;
    peindex = (int) cpos.Elevation;

    pdata.apos = positiontable[paindex].apos;
    pdata.epos = positiontable[peindex].epos;
    pdata.spwm = positiontable[paindex].spwm;
    pdata.hpwm = positiontable[peindex].hpwm;

    return pdata;
}

void StSetPanelPosition(panelpos_s newpos)
{
    int pwmnel, pwmnaz;

    if(newpos.Azimuth < 30.0) {newpos.Azimuth = 30.0;}
    if(newpos.Azimuth > 180.0) {newpos.Azimuth = 180.0;}
    pwmnel = positiontable[(int)newpos.Elevation].hpwm;
    pwmnaz = positiontable[(int)newpos.Azimuth].spwm;
    gpioPWM(STAZPIN,pwmnaz);
    pwmWrite(STELPIN,pwmnel);       // Move panel
    delay(STSTEPDELAY);
 }

int StSavePositionData(positiondata_s posdata)
{
    FILE * fp;

    fp = fopen("position.dat","w");
    if(fp == NULL) { return 0; }
    fwrite(&posdata,sizeof(positiondata_s),1,fp);
    fclose(fp);
    return 1;
 }

int StSavePositionTable(void)
{
    FILE * fp;

    fp = fopen("position.dat","w");
    if(fp == NULL) { return 0; }
    fwrite(&positiontable,sizeof(positiontable),1,fp);
    fclose(fp);
    return 1;
}

int StRetrievePositionTable(void)
{
    FILE * fp;

    fp = fopen("position.dat","r");
    if(fp == NULL) { return 0; }
    fread(&positiontable,sizeof(positiontable),1,fp);
    fclose(fp);
    return 1;
}

int StLogPanelData(paneldata_s pdata, reading_s creads)
{
    FILE * fp;
	char ltime[25];

    fp = fopen("paneldata.csv","a");
    if(fp == NULL) { return 0; }
	strcpy(ltime,ctime(&creads.rtime));
	ltime[3] = ',';
	ltime[7] = ',';
	ltime[10] = ',';
	ltime[19] = ',';
    fprintf(fp,"%.24s,%3.0lf,%3.0lf,%lf,%lf,%3.1lf,%3.1lf,%5.1lf,%3.0lf\n",
			ltime,pdata.azimuth,pdata.elevation,pdata.latitude,pdata.longitude,
			creads.temperature,creads.humidity,creads.pressure,creads.light);
    fclose(fp);
    return 1;
}

panelpos_s StCalculateNewPosition(void)
{
    panelpos_s newpos = {0.0};
    spa_data csp = {0.0};
    time_t rawtime;
    struct tm * ct;
    loc_t gpspos = {0};
    
	time(&rawtime);
    ct = localtime(&rawtime);
    gpspos = gps_location();

    csp.year = ct->tm_year+1900;
    csp.month = ct->tm_mon+1;
    csp.day = ct->tm_mday;
    csp.hour = ct->tm_hour;
    csp.minute = ct->tm_min;
    csp.second = ct->tm_sec;
    csp.timezone = DTIMEZONE;
    csp.delta_ut1 = DELTAUT1;
    csp.delta_t = DELTAT;

    if(gpspos.latitude != 0 && gpspos.longitude != 0)
    {
        csp.longitude = gpspos.longitude;
        csp.latitude = gpspos.latitude;
        csp.elevation = gpspos.altitude;
        newpos.gpsdata = gpspos;
    }
    else
    {
        csp.longitude = DLONGITUDE;
        csp.latitude = DLATITUDE;
        csp.elevation = PELEVATION;
        newpos.gpsdata.latitude = csp.latitude;
        newpos.gpsdata.longitude = csp.longitude;
    }
    csp.pressure = WsGetPressure();
    csp.temperature = WsGetTemperature();
    csp.slope = DSLOPE;
    csp.azm_rotation = DAZROT;
    csp.atmos_refract = DATMREF;
    csp.function = SPA_ALL;
    spa_calculate(&csp);
// if(csp.azimuth > 180.0) {csp.azimuth = 180.0 - csp.azimuth;} //might need this someday
    newpos.Azimuth = csp.azimuth;
    newpos.Elevation = 90.0 - csp.incidence;

    return newpos;
}

panelpos_s StTrackSun(void)
{
    panelpos_s tpos = {0.0};
    ldrsensor_s spos = {0.0};
	int i;
	
    tpos = StCalculateNewPosition();
    tpos.Elevation = tpos.Elevation + 90.0;
    if(tpos.Elevation <= 0.0)
    {
        tpos.Azimuth = PAZIMUTH;
        tpos.Elevation = PELEVATION;
        StSetPanelPosition(tpos);
        return tpos;
    }
    else
    {
		StSetPanelPosition(tpos);
	}
	//check to see if it works, adjusted from 4.4.2018
    //This Adjusts the panel position using LDR data
	for (i=0;i<10;i++)
    {
		spos = StGetLdrReadings();
		if(spos.eset < STSECTR - 10) {tpos.Elevation++;}
		if(spos.eset > STSECTR + 10) {tpos.Elevation--;}
		if(spos.aset < STSACTR - 10) {tpos.Azimuth--;}
		if(spos.aset > STSACTR + 10) {tpos.Azimuth++;}
		StSetPanelPosition(tpos);
	}
    return tpos;
}