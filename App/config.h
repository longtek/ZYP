
/********************************************************************************************************
** File Name:          config.h
** Last modified Date: 2006-01-06
** Last Version:       v2.0
** Descriptions:       �û�����ͷ�ļ�
**
**------------------------------------------------------------------------------------------------------
** Created date:       2005-12-31 
** Version:            v1.0
** Descriptions:       ����
**
**------------------------------------------------------------------------------------------------------
** Modified date:      2006-01-06 
** Version:            v2.0
** Descriptions:       �޸�����S3C2410
**
**------------------------------------------------------------------------------------------------------
** Modified by:         
** Modified date:      
** Version:            
** Descriptions:        
**
********************************************************************************************************/

#ifndef  CONFIG_H
#define  CONFIG_H

//��һ������Ķ�
#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef NULL
#define NULL  		0
#endif

typedef unsigned char  uint8;                   // �޷���8λ���ͱ���
typedef signed   char  int8;                    // �з���8λ���ͱ���
typedef unsigned short uint16;                  // �޷���16λ���ͱ���
typedef signed   short int16;                   // �з���16λ���ͱ���
typedef unsigned int   uint32;                  // �޷���32λ���ͱ���
typedef signed   int   int32;                   // �з���32λ���ͱ���
typedef float          fp32;                    // �����ȸ�����(32λ����)
typedef double         fp64;                    // ˫���ȸ�����(64λ����)


/********************************/
/*      uC/OS-II���������      */
/********************************/
#define     USER_USING_MODE    0x10             // �û�ģʽ,ARM����
                                                // ֻ����0x10,0x30,0x1f,0x3f֮һ
//#include   "INCLUDES.H"  

#include "ucos_ii.h"                                      
          
                                           
/********************************/
/*        ARM���������         */
/********************************/
// ��һ������Ķ�
#include    "2416addr.h"
#include    <stdio.h>
#include    <ctype.h>
#include    <stdlib.h>


// IRQ�ж�������ַ��
//extern  uint32 VICVectAddr[];


/********************************/
/*      Ӧ�ó�������            */
/********************************/
//#include    <stdio.h>
//#include    <ctype.h>
//#include    <stdlib.h>
//#include    <setjmp.h>
//#include    <rt_misc.h>

// ���¸�����Ҫ�Ķ�

#include   "Target.h"

//#include "2440lib.h"



/********************************/
/*       �û������ļ�           */
/********************************/
#include "led.h"
#include "bluetooth.h"
#include "MCP2515.h"
#include "def.h"
#include "canconfig.h"
#include "wm8731.h"
#include "wav.h"
#include "dma.h"
#include "prowav.h"
#include "basicdata.h"
// ���¸�����Ҫ�Ķ�
/********************************/
/*       �û���������           */
/********************************/
// ���¸�����Ҫ�Ķ�

#endif

/********************************************************************************************************
**                            End Of File
********************************************************************************************************/