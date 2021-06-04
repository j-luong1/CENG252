#ifndef GPS_H
#define GPS_H

#define round(x) ((x < 0) ? (ceil((x)-0.5)) : (floor((x)+0.5)))

struct location
{
	double utc;
	double date;
    double latitude;
    double longitude;
    double speed;
    double altitude;
    double course;
};

typedef struct location loc_t;

// Initialize device
extern void gps_init(void);

// Activate device
extern void gps_on(void);

// Get the actual location
extern void gps_location(loc_t *);


// Turn off device (low-power consumption)
extern void gps_off(void);

// -------------------------------------------------------------------------
// Internal functions
// -------------------------------------------------------------------------

// convert deg to decimal deg latitude, (N/S), longitude, (W/E)
void gps_convert_deg_to_dec(double *, char, double *, char);
double gps_deg_dec(double);

#endif
