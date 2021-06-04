#include <stdio.h>
#include "panel.h"
#include "wxstn.h"
#include "tsl2561.h"
#include "sptglgmain.h"

extern positiondata_s positiontable[STMAXPSZ];

int main(void)
{
    reading_s creads = {0};
    int clux = 0;
    ldrsensor_s cldr = {0};
    panelpos_s selaz;
    int az[11] = {180,165,150,135,120,105,90,75,60,45,30};
    int el[11] = {180,170,160,150,140,130,120,110,100,90,90};
    int i;

    StPanelInitialization();
    printf("\nCENG252 Solar Panel Tracker\n");

    i=StRetrievePositionTable();
    for(i=0; i<11; i++)
    {
        int j;
        selaz.Azimuth = az[i];
        selaz.Elevation = el[i];
        StSetPanelPosition(selaz);
        printf("Az; %3d - SPWM: %4d El: %3d HPWM: %4d\n",az[i],positiontable[az[i]].spwm,el[i],positiontable[el[i]].hpwm);
        do
        {
                printf("Enter New SPWM: ");
                scanf("%4d",&j);
                if(j == -1) break;
                positiontable[az[i]].spwm = j;
                StSetPanelPosition(selaz);
        } while(1);
        do
        {
            printf("Enter New HPWM: ");
            scanf("%4d",&j);
            if(j == -1) break;
            positiontable[el[i]].hpwm = j;
            StSetPanelPosition(selaz);
        } while(1);
    }

    for(i=30; i<STMAXPSZ; i+=15)
    {
        int j;
        int astart = i;
        int aend = i+15;
        int arange = positiontable[aend].spwm - positiontable[astart].spwm;
        for(j=1; j <=15; j++)
        {
            positiontable[astart+j].spwm = (j*arange/15.0)+positiontable[astart].spwm+.5;
        }

    }

    for(i=90; i<STMAXPSZ; i+=10)
    {
        int j;
        int astart = i;
        int aend = i+10;
        int arange = positiontable[aend].hpwm - positiontable[astart].hpwm;
        for(j=1; j <=10; j++)
        {
            positiontable[astart+j].hpwm = (j*arange/10.0)+positiontable[astart].hpwm+.5;
        }

    }
        for(i=0; i<11; i++)
    {
        selaz.Azimuth = az[i];
        selaz.Elevation = el[i];
        StSetPanelPosition(selaz);
        WsDelay(3000);
    }
    return 1;
}
