#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <pcf8591.h>
#include <pigpio.h>
#include "spa.h"
#include "tsl2561.h"
#include "panel.h"
#include "wxstn.h"

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
panelpos_s StCalculateNewPosition(void)
{
    panelpos_s newpos = {0.0};
    spa_data csp = {0.0};
    time_t rawtime;
    struct tm * ct;

    time(&rawtime);
    ct = localtime(&rawtime);

    csp.year = ct->tm_year+1900;
    csp.month = ct->tm_mon+1;
    csp.day = ct->tm_mday;
    csp.hour = ct->tm_hour;
    csp.minute = ct->tm_min;
    csp.second = ct->tm_sec;
    csp.timezone = DTIMEZONE;
    csp.delta_ut1 = DELTAUT1;
    csp.delta_t = DELTAT;
    csp.longitude = DLONGITUDE;
    csp.latitude = DLATITUDE;
    csp.elevation = PELEVATION;
    csp.pressure = WsGetPressure();
    csp.temperature = WsGetTemperature();

    spa_calculate(&csp);

    newpos.Azimuth = csp.azimuth;
//    newpos.Elevation = csp.elevation;

    return newpos;
}

panelpos_s StTrackSun(void)
{

    panelpos_s tpos = {0.0};
    ldrsensor_s spos = {0.0};

    tpos = StCalculateNewPosition();
    if(tpos.Elevation<0)
    {
        tpos.Elevation = 0;
        return tpos;
    }
        return tpos;
}

void StServoSetup(void)
{
    int i;

    // Hardware PWM for elevation
    pinMode(STELPIN,PWM_OUTPUT);
    pwmSetMode(PWM_MODE_MS);
    pwmSetClock(STPWMCLOCK);
    pwmSetRange(STPWMRANGE);

    // pigpio PWM for azimuth
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
 //   cpos.Elevation = el*180.0/(STPE180-STPE000);
 //   cpos.Azimuth = az*180/(STPA180-STPA000);

    return cpos;
}

void StSetPanelPosition(panelpos_s newpos)
{
    int pwmnel, pwmnaz;

    pwmnel = positiontable[(int)newpos.Elevation].hpwm;
    pwmnaz = positiontable[(int)newpos.Azimuth].spwm;
    gpioPWM(STAZPIN,pwmnaz);
    pwmWrite(STELPIN,pwmnel);       // Move panel
    delay(STSTEPDELAY);
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

positiondata_s StRetrievePositionData(void)
{
    FILE * fp;
    positiondata_s posdata = {0};

    fp = fopen("position.dat","r");
    if(fp == NULL) { return posdata; }
    fread(&posdata,sizeof(positiondata_s),1,fp);
    fclose(fp);
    return posdata;
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

 int StLogPanelData(paneldata_s pdata)
 {
    FILE * fp;

    fp = fopen("paneldata.csv","a");
    if(fp == NULL) { return 0; }
    fprintf(fp,"%4d,%4d,%4d,%4d,%4d,%4d\n",pdata.adeg,pdata.edeg,pdata.apos,pdata.epos,pdata.spwm,pdata.hpwm);
    fclose(fp);
    return 1;
 }
