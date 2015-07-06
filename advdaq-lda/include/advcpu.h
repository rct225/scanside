/****************************************************************************
				
		Copyright 2006 IAG Software Team, 

			BeiJing R&D Center 

			Advantech Co., Ltd.

		Advantech Device driver for Linux

File Name:
	advcpu.h
Abstract:
	Several data type be defined for 64-bit system and
	32-bit system in this file.

        ------------------------------
        DATA TYPE    ILP32        LP64
        ------------------------------
        char         8            8
        short        16           16
        int          32           32
        long         32           64
        long long    32           64
        pointer      32           64
        -----------------------------

        NOTE:
        64-bit Linux System adoptes LP64 standard.
        
Version history
	06/28/2006		Create by zhiyong.xie

************************************************************************/
#ifndef __ADV_TYPEDEF_H__
#define __ADV_TYPEDEF_H__

//#ifdef _ADV_32_BIT_X86_
/* for 32-bit system */
typedef unsigned char INT8U;
typedef signed char   INT8S;
typedef unsigned short INT16U;
typedef signed short   INT16S;
typedef unsigned int   INT32U;
typedef signed int     INT32S;
typedef unsigned long long INT64U;
typedef signed long long INT64S;
typedef float          FP32;
typedef double         FP64;
typedef unsigned long  PTR_T;
typedef char           BYTE;
typedef void           VOID;
typedef long           LONG;
typedef unsigned long  ULONG;

//#endif

#ifdef _ADV_64_BIT_X86_
/* for 64-bit system */
typedef unsigned char INT8U;
typedef signed char   INT8S;
typedef unsigned short INT16U;
typedef signed short   INT16S;
typedef unsigned int   INT32U;
typedef signed int     INT32S;
typedef unsigned long INT64U;
typedef signed long  INT64S;
typedef float          FP32;
typedef double         FP64;
typedef unsigned long  PTR_T;
typedef char           BYTE;
typedef void           VOID;
typedef long           LONG;
typedef unsigned long  ULONG;

#endif

#endif
