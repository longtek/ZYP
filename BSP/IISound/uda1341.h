#ifndef _UDA1341_H_
#define _UDA1341_H_
#define	MAX_VOLUME	61
#define L3CLOCK (1<<4) //GPB4
#define L3DATA  (1<<3) //GPB3
#define L3MODE  (1<<2) //GPB2
#define L3CLOCK_HIGH() {rGPBDAT|=L3CLOCK;}
#define L3CLOCK_LOW()  {rGPBDAT&=(~L3CLOCK);}
#define L3DATA_HIGH()  {rGPBDAT|=L3DATA;}
#define L3DATA_LOW()   {rGPBDAT&=(~L3DATA);}
#define L3MODE_HIGH()  {rGPBDAT|=L3MODE;}
#define L3MODE_LOW()   {rGPBDAT&=(~L3MODE);}
static void uda1341_init(char mode);
static void uda1341_writeL3addr(unsigned char data);
static void uda1341_writeL3data(unsigned char data ,int mode);
static void Init1341port();
void IIS_Init();
static void AdjVolume(unsigned short volume);
#endif  