//#include "HSHBME280.h"
#include <stdint.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include "hshbme280.h"

static struct SensorCalibration MySensor;
static int BME280fd;
static int32_t t_fine;

//  Settings and configuration
void BME280Setup(void)
{
	uint8_t dataToWrite = 0;  //Temporary variable

	BME280fd = wiringPiI2CSetup(I2CADDRESS);
	BME280Reset();
	wiringPiI2CReadReg8(BME280fd,BME280_CHIP_ID_REG);

	// Calibration Section
	// Reading all compensation data, range 0x88:A1, 0xE1:E7
	MySensor.dig_T1 = ((uint16_t)((wiringPiI2CReadReg8(BME280fd,BME280_DIG_T1_MSB_REG) << 8) + wiringPiI2CReadReg8(BME280fd,BME280_DIG_T1_LSB_REG)));
	MySensor.dig_T2 = ((int16_t)((wiringPiI2CReadReg8(BME280fd,BME280_DIG_T2_MSB_REG) << 8) + wiringPiI2CReadReg8(BME280fd,BME280_DIG_T2_LSB_REG)));
	MySensor.dig_T3 = ((int16_t)((wiringPiI2CReadReg8(BME280fd,BME280_DIG_T3_MSB_REG) << 8) + wiringPiI2CReadReg8(BME280fd,BME280_DIG_T3_LSB_REG)));

	MySensor.dig_P1 = ((uint16_t)((wiringPiI2CReadReg8(BME280fd,BME280_DIG_P1_MSB_REG) << 8) + wiringPiI2CReadReg8(BME280fd,BME280_DIG_P1_LSB_REG)));
	MySensor.dig_P2 = ((int16_t)((wiringPiI2CReadReg8(BME280fd,BME280_DIG_P2_MSB_REG) << 8) + wiringPiI2CReadReg8(BME280fd,BME280_DIG_P2_LSB_REG)));
	MySensor.dig_P3 = ((int16_t)((wiringPiI2CReadReg8(BME280fd,BME280_DIG_P3_MSB_REG) << 8) + wiringPiI2CReadReg8(BME280fd,BME280_DIG_P3_LSB_REG)));
	MySensor.dig_P4 = ((int16_t)((wiringPiI2CReadReg8(BME280fd,BME280_DIG_P4_MSB_REG) << 8) + wiringPiI2CReadReg8(BME280fd,BME280_DIG_P4_LSB_REG)));
	MySensor.dig_P5 = ((int16_t)((wiringPiI2CReadReg8(BME280fd,BME280_DIG_P5_MSB_REG) << 8) + wiringPiI2CReadReg8(BME280fd,BME280_DIG_P5_LSB_REG)));
	MySensor.dig_P6 = ((int16_t)((wiringPiI2CReadReg8(BME280fd,BME280_DIG_P6_MSB_REG) << 8) + wiringPiI2CReadReg8(BME280fd,BME280_DIG_P6_LSB_REG)));
	MySensor.dig_P7 = ((int16_t)((wiringPiI2CReadReg8(BME280fd,BME280_DIG_P7_MSB_REG) << 8) + wiringPiI2CReadReg8(BME280fd,BME280_DIG_P7_LSB_REG)));
	MySensor.dig_P8 = ((int16_t)((wiringPiI2CReadReg8(BME280fd,BME280_DIG_P8_MSB_REG) << 8) + wiringPiI2CReadReg8(BME280fd,BME280_DIG_P8_LSB_REG)));
	MySensor.dig_P9 = ((int16_t)((wiringPiI2CReadReg8(BME280fd,BME280_DIG_P9_MSB_REG) << 8) + wiringPiI2CReadReg8(BME280fd,BME280_DIG_P9_LSB_REG)));

	MySensor.dig_H1 = ((uint8_t)(wiringPiI2CReadReg8(BME280fd,BME280_DIG_H1_REG)));
	MySensor.dig_H2 = ((int16_t)((wiringPiI2CReadReg8(BME280fd,BME280_DIG_H2_MSB_REG) << 8) + wiringPiI2CReadReg8(BME280fd,BME280_DIG_H2_LSB_REG)));
	MySensor.dig_H3 = ((uint8_t)(wiringPiI2CReadReg8(BME280fd,BME280_DIG_H3_REG)));
	MySensor.dig_H4 = ((int16_t)((wiringPiI2CReadReg8(BME280fd,BME280_DIG_H4_MSB_REG) << 4) + (wiringPiI2CReadReg8(BME280fd,BME280_DIG_H4_LSB_REG) & 0x0F)));
	MySensor.dig_H5 = ((int16_t)((wiringPiI2CReadReg8(BME280fd,BME280_DIG_H5_MSB_REG) << 4) + ((wiringPiI2CReadReg8(BME280fd,BME280_DIG_H4_LSB_REG) >> 4) & 0x0F)));
	MySensor.dig_H6 = ((uint8_t)wiringPiI2CReadReg8(BME280fd,BME280_DIG_H6_REG));

	// Configuration section
	// Set the oversampling control words.
	// config will only be writeable in sleep mode, so first insure that.
	wiringPiI2CWriteReg8(BME280fd,BME280_CTRL_MEAS_REG, 0x00);
	delay(5);

	//Set the config word
	dataToWrite = (TSTANDBY << 0x5) & 0xE0;
	dataToWrite |= (FILTER << 0x02) & 0x1C;
	wiringPiI2CWriteReg8(BME280fd,BME280_CONFIG_REG, dataToWrite);

	//Set ctrl_hum first, then ctrl_meas to activate ctrl_hum
	dataToWrite = HOVRSAMP & 0x07; //all other bits can be ignored
	wiringPiI2CWriteReg8(BME280fd,BME280_CTRL_HUMIDITY_REG, dataToWrite);

	//set ctrl_meas
	wiringPiI2CWriteReg8(BME280fd,BME280_CTRL_MEAS_REG, CTRLMEASBME280);
}


//Strictly resets.  Run BME280Setup afterwards
void BME280Reset(void)
{
	wiringPiI2CWriteReg8(BME280fd,BME280_RST_REG, 0xb6);
	delay(5);
}

// Returns pressure Pa as unsigned 32 bit integer in Q24.8 format (24 integer bits and 8 fractional bits).
//	Output value of “24674867” represents 24674867/256 = 96386.2 Pa = 963.862 hPa
double GetBME280Pressure(void)
{
	int32_t adc_P,msb,lsb,xlsb;
	int64_t var1, var2, p_acc;

	wiringPiI2CWriteReg8(BME280fd,BME280_CTRL_MEAS_REG,CTRLMEASBME280);
	msb = (uint32_t)wiringPiI2CReadReg8(BME280fd,BME280_PRESSURE_MSB_REG) << 12;
	lsb = (uint32_t)wiringPiI2CReadReg8(BME280fd,BME280_PRESSURE_LSB_REG) << 4;
	xlsb = (wiringPiI2CReadReg8(BME280fd,BME280_PRESSURE_XLSB_REG) >> 4) & 0x0F;
	adc_P = msb | lsb | xlsb;
	var1 = ((int64_t)t_fine) - 128000;
	var2 = var1 * var1 * (int64_t)MySensor.dig_P6;
	var2 = var2 + ((var1 * (int64_t)MySensor.dig_P5)<<17);
	var2 = var2 + (((int64_t)MySensor.dig_P4)<<35);
var1 = ((var1 * var1 * (int64_t)MySensor.dig_P3)>>8) + ((var1 * (int64_t)MySensor.dig_P2)<<12);
	var1 = (((((int64_t)1)<<47)+var1))*((int64_t)MySensor.dig_P1)>>33;
	if (var1 == 0)
	{
		return 0; // avoid exception caused by division by zero
	}
	p_acc = 1048576 - adc_P;
	p_acc = (((p_acc<<31) - var2)*3125)/var1;
	var1 = (((int64_t)MySensor.dig_P9) * (p_acc>>13) * (p_acc>>13)) >> 25;
	var2 = (((int64_t)MySensor.dig_P8) * p_acc) >> 19;
	p_acc = ((p_acc + var1 + var2) >> 8) + (((int64_t)MySensor.dig_P7)<<4);
	p_acc = p_acc >> 8; // /256
	return p_acc;
}

// Returns humidity %RH as unsigned 32 bit integer in Q22. 10 format (22 integer and 10 fractional bits).
// Output value of “47445” represents 47445/1024 = 46. 333 %RH
double GetBME280Humidity(void)
{
	int32_t adc_H, msb,lsb;
	int32_t var1;

	wiringPiI2CWriteReg8(BME280fd,BME280_CTRL_MEAS_REG,CTRLMEASBME280); // Force
	msb = (uint32_t)wiringPiI2CReadReg8(BME280fd,BME280_HUMIDITY_MSB_REG) << 8;
	lsb = (uint32_t)wiringPiI2CReadReg8(BME280fd,BME280_HUMIDITY_LSB_REG);
	adc_H = msb | lsb;
	var1 = (t_fine - ((int32_t)76800));
	var1 = (((((adc_H << 14) - (((int32_t)MySensor.dig_H4) << 20) - (((int32_t)MySensor.dig_H5) * var1)) +
	((int32_t)16384)) >> 15) * (((((((var1 * ((int32_t)MySensor.dig_H6)) >> 10) * (((var1 * ((int32_t)MySensor.dig_H3)) >> 11) + ((int32_t)32768))) >> 10) + ((int32_t)2097152)) *
	((int32_t)MySensor.dig_H2) + 8192) >> 14));
	var1 = (var1 - (((((var1 >> 15) * (var1 >> 15)) >> 7) * ((int32_t)MySensor.dig_H1)) >> 4));
	var1 = (var1 < 0 ? 0 : var1);
	var1 = (var1 > 419430400 ? 419430400 : var1);
	return ((var1>>12) >> 10);
}

//	Returns temperature DegC, resolution is 0.01 DegC. Output value of “5123” equals 51.23 DegC.
//	t_fine carries fine temperature as global value
double GetBME280TempC(void)
{
	int32_t adc_T, msb,lsb,xlsb;
	int64_t var1, var2;
	double output;

	wiringPiI2CWriteReg8(BME280fd,BME280_CTRL_MEAS_REG,CTRLMEASBME280); // ForceMode
	//get the reading (adc_T);
	msb = (uint32_t)wiringPiI2CReadReg8(BME280fd,BME280_TEMPERATURE_MSB_REG) << 12;
	lsb = (uint32_t)wiringPiI2CReadReg8(BME280fd,BME280_TEMPERATURE_LSB_REG) << 4;
xlsb = ((uint32_t)wiringPiI2CReadReg8(BME280fd,BME280_TEMPERATURE_XLSB_REG) >> 4) & 0x0F;
	adc_T = msb | lsb | xlsb;
	//By datasheet, calibrate
	var1 = ((((adc_T>>3) - ((int32_t)MySensor.dig_T1<<1))) * ((int32_t)MySensor.dig_T2)) >> 11;
var2 = (((((adc_T>>4) - ((int32_t)MySensor.dig_T1)) * ((adc_T>>4) - ((int32_t)MySensor.dig_T1))) >> 12) *
	((int32_t)MySensor.dig_T3)) >> 14;
	t_fine = var1 + var2;
	output = (t_fine * 5 + 128) >> 8;
	output = output / 100;
	return output;
}
