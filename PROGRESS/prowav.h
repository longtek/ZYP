#ifndef _PROWAV_H_
#define _PROWAV_H_
#include "def.h"
void   CreatePerRpmDatasize(U32 *DataSize);
void   WavadataCreateWithSin(U32 *DataSize,S16 *m_Wavadata);
void   CaculateDataAddress(U32 *datasize, U32 *StartAddr);
extern unsigned int rpm_sizefromzero[];
extern unsigned int rpm_datasize[];
#endif