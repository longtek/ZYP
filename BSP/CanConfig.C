#include "canconfig.h"
#include "def.h"
CanConfig canconfig=0;
void GetCanConfigInfo(void)
{
    canconfig.CAN_bandrate=0;
	canconfig.CAN_IsExt=0;
	canconfig.CAN_endian=1;//1=big 0=little              
	canconfig.RPM.ID=250; 
	canconfig.RPM.BYTENUM=7;
	canconfig.RPM.BITPOS=7;
	canconfig.RPM.DATALEN=16;
	canconfig.RPM.DATACOEF=1;
	canconfig.SPEED.ID=257;
	canconfig.SPEED.BYTENUM=3;
	canconfig.SPEED.BITPOS=7;
	canconfig.SPEED.DATALEN=16;
	canconfig.SPEED.DATACOEF=0.5;
	canconfig.THROTTLE.ID=256;                 
	canconfig.THROTTLE.BYTENUM=3;
	canconfig.THROTTLE.BITPOS=7;
	canconfig.THROTTLE.DATALEN=8;
	canconfig.THROTTLE.DATACOEF=0.7;
}