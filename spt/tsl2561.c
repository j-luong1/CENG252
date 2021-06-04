#include "tsl2561.h"
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <inttypes.h>
#include <stdio.h>

static int TSL2561fd;

void tsl2561Setup(void)
{
    TSL2561fd = wiringPiI2CSetup(TSL2561_ADDR_FLOAT);
}

int tsl2561GetLux(void)
{
    uint16_t visible_and_ir = 0;
    wiringPiI2CWriteReg8(TSL2561fd, TSL2561_COMMAND_BIT, TSL2561_CONTROL_POWERON);
    wiringPiI2CWriteReg8(TSL2561fd, TSL2561_REGISTER_TIMING, TSL2561_GAIN_AUTO);
    delay(LUXDELAY);
    visible_and_ir = wiringPiI2CReadReg16(TSL2561fd, TSL2561_REGISTER_CHAN0_LOW);
    wiringPiI2CWriteReg8(TSL2561fd, TSL2561_COMMAND_BIT, TSL2561_CONTROL_POWEROFF);
    return visible_and_ir * 2;
}

void tsl2561DisplayLux(int dlux)
{
    printf("Light Intensity: %6dlux\n", dlux);
}
