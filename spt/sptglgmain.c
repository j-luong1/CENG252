//
//  This example demonstrates how to use GLG control widgets as they are,
//  without emdedding them into another GLG drawing, and handle user
//  interaction.
//
//  The drawing name is GhGUI.g.
//

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <GlgApi.h>
#include "sptglgmain.h"
#include "panel.h"
#include "wxstn.h"
#include "tsl2561.h"

// Top level global variables
GlgAppContext AppContext;
GlgObject SptDrawing;
GlgLong UpdateInterval = UPDATE_INTERVAL;
int TrackOn = STOFF;

// Defines a platform-specific program entry point
#include "GlgMain.h"

int GlgMain(int argc,char *argv[],GlgAppContext InitAppContext )
{
	// Initialization Section
	AppContext = GlgInit( False, InitAppContext, argc, argv );
	GlgSetDResource(AppContext,"$config/GlgOpenGLMode",0.); // Disable openGL
 //   reading_s creads = {0};


    // Initialize the Solar Panel Tracker
    StPanelInitialization();

	// Load a drawing from the file.
	SptDrawing = GlgLoadWidgetFromFile( "SPTrack.g" );

	if(!SptDrawing) { exit( 1 ); }

	// Setting widget dimensions using world coordinates [-1000;1000].
	GlgSetGResource(SptDrawing,"Point1",-600.,-600.,0.);
	GlgSetGResource(SptDrawing,"Point2",600.,600.,0.);

	// Setting the window name (title).
	GlgSetSResource(SptDrawing,"ScreenName","Jonathan Luong's Solar Panel Tracker" );

	// Add Input callback to handle user interraction in the GLG control.
	GlgAddCallback(SptDrawing,GLG_INPUT_CB,(GlgCallbackProc)Input,NULL );

	// Paint the drawing.
	GlgInitialDraw(SptDrawing);
	GlgAddTimeOut(AppContext,UpdateInterval,(GlgTimerProc)UpdateControls,NULL );
	return (int) GlgMainLoop( AppContext );
}

//
// This callback is invoked when user interacts with input objects in GLG
// a  drawing, such as a slider, dial or a button.
//
void Input(GlgObject GhDrawing,GlgAnyType client_data,GlgAnyType call_data)
{
	GlgObject message_obj;
	char * format,* action,* origin, * full_origin;
	double pstate,svalue;
	message_obj = (GlgObject) call_data;

	// Get the message's format, action and origin.
	GlgGetSResource(message_obj,"Format", &format);
	GlgGetSResource(message_obj,"Action",&action);
	GlgGetSResource(message_obj,"Origin",&origin);
	GlgGetSResource(message_obj,"FullOrigin",&full_origin);

	// Handle window closing. May use GhDrawing's name.
	if(strcmp(format,"Window") == 0 &&
		strcmp(action,"DeleteWindow") == 0)
	{ exit(0); }

    // Retrieve OnState value from Track1
    if( strcmp(format,"Button") == 0 && strcmp(action,"ValueChanged") == 0)
    {
        GlgGetDResource(SptDrawing,"Track1/OnState", &svalue );
        if(svalue == 1.0)
        {
            TrackOn = 1;
            GlgSetDResource(SptDrawing,"Elevation1/DisableInput",1);
            GlgSetDResource(SptDrawing,"Azimuth1/DisableInput",1);
        }
        else
        {
            TrackOn = 0;
            GlgSetDResource(SptDrawing,"Elevation1/DisableInput",0);
            GlgSetDResource(SptDrawing,"Azimuth1/DisableInput",0);
        }
    }
}

//
// Timer procedure to update controls, and drawing values
//
void UpdateControls(GlgAnyType data,GlgIntervalID * id)
{

//Initial declarations for variables
    reading_s creads = {0};
    ldrsensor_s cldr = {0};
    panelpos_s selaz = {0};
    int clux = 0;
    double svalue = {0.0};

//Getting values for variables
    creads = WsGetReadings();
    clux = tsl2561GetLux();


//If statement to check track for real el/az or random
    if(TrackOn && SIMTRACK)
    {
        cldr = StGetLdrReadings();
        selaz.Elevation = (double) (rand() % 90);
        selaz.Azimuth = (double) (rand() % 180);
    }
    else if(TrackOn==0)
    {
        GlgGetDResource(SptDrawing,"Elevation1/Value", &svalue );
            selaz.Elevation = svalue;
        GlgGetDResource(SptDrawing,"Azimuth1/Value", &svalue);
            selaz.Azimuth = svalue;
    }
    else
    {


        StTrackSun();
        selaz.Elevation = tpos.elevation;
        selaz.Azimuth = tpos.azimuth;

    }

//Physically adjust position of panel
    StSetPanelPosition(selaz);

//Update GUI with values from creads/selaz
	GlgSetDResource(SptDrawing,"Temp1/Value",creads.temperature);
	GlgSetDResource(SptDrawing,"Humid1/Value",creads.humidity);
	GlgSetDResource(SptDrawing,"Press1/Value",creads.pressure);
	GlgSetDResource(SptDrawing,"Windspeed1/Value", creads.windspeed);
	GlgSetDResource(SptDrawing,"WindDirection1/Value",creads.winddirection);
	GlgSetDResource(SptDrawing,"Luminosity1/Value",clux);
	GlgSetDResource(SptDrawing,"Elevation1/Value",selaz.Elevation);
	GlgSetDResource(SptDrawing,"Azimuth1/Value",selaz.Azimuth);
	GlgUpdate(SptDrawing);
	GlgAddTimeOut(AppContext,UpdateInterval,(GlgTimerProc)UpdateControls,NULL );
}
