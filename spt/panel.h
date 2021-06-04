#ifndef PANEL_H
#define PANEL_H

// Constants
// I2C Device Addresses
#define ST_RTC_I2CADR       0x68
#define ST_MCP23017_I2CADR  0x20
#define ST_PCF8591_I2CADR   0x48
#define ST_BME280_I2CADR    0x77

// wiringPi Device Pin Base
#define ST_MCP23017_PINBASE 100
#define ST_PCF8591_PINBASE  200
#define ST_BME280_PINBASE   300

// LDR
#define SIMLDR 0
#define STSECTR 127
#define STSACTR 127

// Panel Position constants
#define STSIMPOS 1
#define STMAXEL     180.0
#define STMINEL     30.0
#define STMAXAZ     180.0
#define STMINAZ     30.0
#define STMAXDEGREE 180
#define STMINDEGREE 30.0
#define STSTEPRANGE 180.0
#define STPWMCLOCK  384
#define STPWMRANGE  1000
#define STPWMMAX    105.0
#define STPWMMIN    5.0
#define STSPWMRANGE 2350
#define STSPWMMAX   2350.0
#define STSPWMMIN   570.0
#define STSTEPDELAY 250
#define STELPIN     1
#define STAZPIN     17
#define STPE000     10.0
#define STPE090     55.0
#define STPE180     120.0
#define STPA000     10.0
#define STPA180     120.0
#define STSAVEPRD   1
#define STMAXPSZ    181

// Solar Position Constants
#define DELTAUT1 0.0
#define DELTAT 67.0
#define DTIMEZONE -4.0
#define DLATITUDE 43.63409
#define DLONGITUDE -79.45930
#define DALTITUDE 166.0
#define DPANELHDG 0.0
#define PAZIMUTH 180.0
#define PELEVATION 0.0
#define DSLOPE 0.0
#define DAZROT 0.0
#define DATMREF 0.5667

#define SIMTRACK 1

// Data structure
struct panelpos
{
    double Azimuth;
    double Elevation;
};
typedef struct panelpos panelpos_s;

struct positiondata
{
    int apos;
    int epos;
    int hpwm;
    int spwm;
};
typedef struct positiondata positiondata_s;

struct paneldata
{
    int adeg;
    int edeg;
    int apos;
    int epos;
    int hpwm;
    int spwm;
};
typedef struct paneldata paneldata_s;

struct ldrsensor
{
    int aset;
    int eset;
};
typedef struct ldrsensor ldrsensor_s;

// Function Prototypes
int StPanelInitialization(void);
void StServoSetup(void);
ldrsensor_s StGetLdrReadings(void);
void StDisplayLdrReadings(ldrsensor_s dsens);
panelpos_s StGetPanelPosition(void);
void StSetPanelPosition(panelpos_s newpos);
int StSavePositionData(positiondata_s posdata);
positiondata_s StRetrievePositionData(void);
int StLogPanelData(paneldata_s pdata);
int StSavePositionTable(void);
int StRetrievePositionTable(void);
panelpos_s StCalculateNewPosition(void);
panelpos_s StTrackSun(void);

#endif
