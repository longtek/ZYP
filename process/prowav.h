#ifndef _PROWAV_H_
#define _PROWAV_H_
#include "def.h"
extern void  CreatePerRpmDatasize(U32 *DataSize);
extern void  WavadataCreateWithSin(U32 *DataSize,S16 *m_Wavadata);
extern void  CaculateDataAddress(U32 *datasize, U32 *StartAddr);
extern U32 rpm_sizefromzero[];
extern U32 rpm_datasize[];
#endif