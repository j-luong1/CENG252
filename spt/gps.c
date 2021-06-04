#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "gps.h"
#include "nmea.h"
//#include "serial.h"

extern void gps_init(void)
{
//    serial_init();
//    serial_config();

    //Write commands
}

extern void gps_on(void)
{
    //Write on
}

// Compute the GPS location using decimal scale
extern void gps_location(loc_t *coord)
{
    uint8_t status = _EMPTY;
    while(status != _COMPLETED)
	{
        gpgga_t gpgga;
        gprmc_t gprmc;
		char buffer[256] = {0};
        //char buffer[256] = "$GPGGA,002153.000,3342.6618,N,11751.3858,W,1,10,1.2,27.0,M,-34.2,M,,0000*5E";
		//char buffer[256] = "$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A";
 //       serial_readln(buffer, 256);
        switch (nmea_get_message_type(buffer))
		{
            case NMEA_GPGGA:
                nmea_parse_gpgga(buffer, &gpgga);
				coord->utc = gpgga.utc;
                gps_convert_deg_to_dec(&(gpgga.latitude), gpgga.lat, &(gpgga.longitude), gpgga.lon);
                coord->latitude = gpgga.latitude;
                coord->longitude = gpgga.longitude;
                coord->altitude = gpgga.altitude;
				status |= NMEA_GPGGA;
				break;
            case NMEA_GPRMC:
                nmea_parse_gprmc(buffer, &gprmc);
				coord->speed = gprmc.speed;
                coord->course = gprmc.course;
				coord->date = gprmc.date;
                status |= NMEA_GPRMC;
 				break;
        }
    }
}

extern void gps_off(void)
{
    //Write off
//    serial_close();
}

// Convert lat e lon to decimals (from deg)
void gps_convert_deg_to_dec(double *latitude, char ns,  double *longitude, char we)
{
    double lat = (ns == 'N') ? *latitude : -1 * (*latitude);
    double lon = (we == 'E') ? *longitude : -1 * (*longitude);

    *latitude = gps_deg_dec(lat);
    *longitude = gps_deg_dec(lon);
}

double gps_deg_dec(double deg_point)
{
    double ddeg;
    double sec = modf(deg_point, &ddeg)*60;
    int deg = (int)(ddeg/100);
    int min = (int)(deg_point-(deg*100));

    double absmlat = round(min * 1000000.);
    double absslat = round(sec * 1000000.);
    double absdlat = round(deg * 1000000.);

    return round(absdlat + (absmlat/60) + (absslat/3600)) /1000000;
}

