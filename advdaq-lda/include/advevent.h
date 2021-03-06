#ifndef __ADV_DAQ_EVENT_H__
#define __ADV_DAQ_EVENT_H__

/**
 * 
 * advevent.h   header file for event code
 *  
 * Copyright (C) 2006 Advantech Co., Ltd
 *
 * Change Log:
 *       2006/07/13  Initial Version
 *
 */

/**
 * Notes:
 * 1. Bit-mapped ID:
 *      0x0001 ~ 0x1000 : Reserved Event ID (ID-mapped).
 *      0x1000 ~ 0xffff : ID-mapped Event ID.
 *      
 *
 * 2. EventID = 0x0000 is reserved for non event or error.
 *    EventID = 0xffff is reserved for time out.
 *    
 *
 * 3. The sub event naming standard from 0x1000 to 0x6fff:
 *      0x1000 ~ 0x1fff  for AI Event Group
 *      0x1000 ~ 0x17ff  for General AI Event ID
 *      0x1800 ~ 0x18ff  for AI others Event ID
 *      
 *      0x2000 ~ 0x2fff  for AO Event Group
 *      0x2000 ~ 0x27ff  for General AO Event ID
 *      0x2800 ~ 0x28ff  for AO others Event ID
 *      
 *      0x3000 ~ 0x3fff  for DI Event Group
 *      0x3800 ~ 0x3fff  for others Event ID
 *      
 *      0x4000 ~ 0x4fff  for DO Event Group
 *      0x4000 ~ 0x47ff  for DO Gernal Event ID
 *      0x4800 ~ 0x48ff  for DO others Event ID
 *      
 *      0x5000 ~ 0x5fff  for Counter/Timer Event Group
 *      0x5000 ~ 0x50ff  for Counter TC Event ID
 *      0x5100 ~ 0x51ff  for Counter OverFlow Event ID
 *      0x5200 ~ 0x52ff  for Counter UnderFlow Event ID
 *      0x5300 ~ 0x53ff  for Counter OverCompare Event ID
 *      0x5400 ~ 0x54ff  for Counter UnderCompare Event ID
 *      0x5500 ~ 0x55ff  for General Timer Event ID
 *      0x5800 ~ 0x58ff  for Counter/Timer others Event ID
 *      
 *      0x6000 ~ 0x6fff  is reserved
 */


/* 0x0000 and 0xffff */
#define ADS_EVT_NO_EVENT             0x0000
#define ADS_EVT_TIME_OUT             0xffff
#define ADS_EVT_DEVICE_CLOSED        0xfffe
#define ADS_EVT_DEVREMOVED           0xfffd

#define ADS_EVT_WATCHDOG             0x0010 /* watchdog actived */
#define ADS_EVT_WATCHDOG_OVERRUN     0x0020 /* watchdog overrun */
#define ADS_EVT_ALARM                0x0040 /* alarm event */

/* for PCI-1760 */
#define ADS_EVT_FILTER               0x0050 

#define ADS_EVT_COUNTER              0x0060
#define ADS_EVT_COUNTERMATCH         (ADS_EVT_COUNTER + 1)
#define ADS_EVT_COUNTEROVERFLOW      (ADS_EVT_COUNTER + 2)
/* end PCI-1760 */

/* for PCI-1750 */
#define ADS_EVT_PORT0                0x0070
#define ADS_EVT_PORT1                0x0071
/* for PCI-1750 end */
/* for PCI-1757  */
#define ADS_EVT_INTERRUPT            0x0072 
/* for PCI-1757 end */
#define ADS_EVT_AI_INTERRUPT         0x1000
#define ADS_EVT_AI_BUFFER1_FULL      (ADS_EVT_AI_INTERRUPT + 1)
#define ADS_EVT_AI_BUFFER2_FULL      (ADS_EVT_AI_INTERRUPT + 2)
#define ADS_EVT_AI_BUFCHANGE         (ADS_EVT_AI_INTERRUPT + 3)
#define ADS_EVT_AI_TERMINATED        (ADS_EVT_AI_INTERRUPT + 4)
#define ADS_EVT_AI_OVERRUN           (ADS_EVT_AI_INTERRUPT + 5)
#define ADS_EVT_AI_LOBUFREADY        (ADS_EVT_AI_INTERRUPT + 6)
#define ADS_EVT_AI_HIBUFREADY        (ADS_EVT_AI_INTERRUPT + 7)

#define ADS_EVT_AO_INTERRUPT         0x2000
#define ADS_EVT_AO_BUFFER1_EMPTY     (ADS_EVT_AO_INTERRUPT + 1)
#define ADS_EVT_AO_BUFFER2_EMPTY     (ADS_EVT_AO_INTERRUPT + 2)
#define ADS_EVT_AO_BUFCHANGE         (ADS_EVT_AO_INTERRUPT + 3)
#define ADS_EVT_AO_TERMINATED        (ADS_EVT_AO_INTERRUPT + 4)
#define ADS_EVT_AO_UNDERRUN          (ADS_EVT_AO_INTERRUPT + 5)
#define ADS_EVT_AO_HIBUF_TRANSFERED  (ADS_EVT_AO_INTERRUPT + 6)
#define ADS_EVT_AO_LOBUF_TRANSFERED  (ADS_EVT_AO_INTERRUPT + 7)


/* New device driver structure. */ 
#define ADS_EVT_DI_INTERRUPT0         0x3000
#define ADS_EVT_DI_INTERRUPT1         (ADS_EVT_DI_INTERRUPT0 + 1)
#define ADS_EVT_DI_INTERRUPT2         (ADS_EVT_DI_INTERRUPT0 + 2)
#define ADS_EVT_DI_INTERRUPT3         (ADS_EVT_DI_INTERRUPT0 + 3)
#define ADS_EVT_DI_INTERRUPT4         (ADS_EVT_DI_INTERRUPT0 + 4)
#define ADS_EVT_DI_INTERRUPT5         (ADS_EVT_DI_INTERRUPT0 + 5)
#define ADS_EVT_DI_INTERRUPT6         (ADS_EVT_DI_INTERRUPT0 + 6)
#define ADS_EVT_DI_INTERRUPT7         (ADS_EVT_DI_INTERRUPT0 + 7)

#define ADS_EVT_DI_INTERRUPT8         (ADS_EVT_DI_INTERRUPT0 + 8)
#define ADS_EVT_DI_INTERRUPT9         (ADS_EVT_DI_INTERRUPT0 + 9)
#define ADS_EVT_DI_INTERRUPT10        (ADS_EVT_DI_INTERRUPT0 + 10)
#define ADS_EVT_DI_INTERRUPT11        (ADS_EVT_DI_INTERRUPT0 + 11)
#define ADS_EVT_DI_INTERRUPT12        (ADS_EVT_DI_INTERRUPT0 + 12)
#define ADS_EVT_DI_INTERRUPT13        (ADS_EVT_DI_INTERRUPT0 + 13)
#define ADS_EVT_DI_INTERRUPT14        (ADS_EVT_DI_INTERRUPT0 + 14)
#define ADS_EVT_DI_INTERRUPT15        (ADS_EVT_DI_INTERRUPT0 + 15)

#define ADS_EVT_DI_INTERRUPT16        (ADS_EVT_DI_INTERRUPT0 + 16)
#define ADS_EVT_DI_INTERRUPT17        (ADS_EVT_DI_INTERRUPT0 + 17)
#define ADS_EVT_DI_INTERRUPT18        (ADS_EVT_DI_INTERRUPT0 + 18)
#define ADS_EVT_DI_INTERRUPT19        (ADS_EVT_DI_INTERRUPT0 + 19)
#define ADS_EVT_DI_INTERRUPT20        (ADS_EVT_DI_INTERRUPT0 + 20)
#define ADS_EVT_DI_INTERRUPT21        (ADS_EVT_DI_INTERRUPT0 + 21)
#define ADS_EVT_DI_INTERRUPT22        (ADS_EVT_DI_INTERRUPT0 + 22)
#define ADS_EVT_DI_INTERRUPT23        (ADS_EVT_DI_INTERRUPT0 + 23)

#define ADS_EVT_DI_INTERRUPT24        (ADS_EVT_DI_INTERRUPT0 + 24)
#define ADS_EVT_DI_INTERRUPT25        (ADS_EVT_DI_INTERRUPT0 + 25)
#define ADS_EVT_DI_INTERRUPT26        (ADS_EVT_DI_INTERRUPT0 + 26)
#define ADS_EVT_DI_INTERRUPT27        (ADS_EVT_DI_INTERRUPT0 + 27)
#define ADS_EVT_DI_INTERRUPT28        (ADS_EVT_DI_INTERRUPT0 + 28)
#define ADS_EVT_DI_INTERRUPT29        (ADS_EVT_DI_INTERRUPT0 + 29)
#define ADS_EVT_DI_INTERRUPT30        (ADS_EVT_DI_INTERRUPT0 + 30)
#define ADS_EVT_DI_INTERRUPT31        (ADS_EVT_DI_INTERRUPT0 + 31)

#define ADS_EVT_DI_INTERRUPT32        (ADS_EVT_DI_INTERRUPT0 + 32)
#define ADS_EVT_DI_INTERRUPT33        (ADS_EVT_DI_INTERRUPT0 + 33)
#define ADS_EVT_DI_INTERRUPT34        (ADS_EVT_DI_INTERRUPT0 + 34)
#define ADS_EVT_DI_INTERRUPT35        (ADS_EVT_DI_INTERRUPT0 + 35)
#define ADS_EVT_DI_INTERRUPT36        (ADS_EVT_DI_INTERRUPT0 + 36)
#define ADS_EVT_DI_INTERRUPT37        (ADS_EVT_DI_INTERRUPT0 + 37)
#define ADS_EVT_DI_INTERRUPT38        (ADS_EVT_DI_INTERRUPT0 + 38)
#define ADS_EVT_DI_INTERRUPT39        (ADS_EVT_DI_INTERRUPT0 + 39)

#define ADS_EVT_DI_INTERRUPT40        (ADS_EVT_DI_INTERRUPT0 + 40)
#define ADS_EVT_DI_INTERRUPT41        (ADS_EVT_DI_INTERRUPT0 + 41)
#define ADS_EVT_DI_INTERRUPT42        (ADS_EVT_DI_INTERRUPT0 + 42)
#define ADS_EVT_DI_INTERRUPT43        (ADS_EVT_DI_INTERRUPT0 + 43)
#define ADS_EVT_DI_INTERRUPT44        (ADS_EVT_DI_INTERRUPT0 + 44)
#define ADS_EVT_DI_INTERRUPT45        (ADS_EVT_DI_INTERRUPT0 + 45)
#define ADS_EVT_DI_INTERRUPT46        (ADS_EVT_DI_INTERRUPT0 + 46)
#define ADS_EVT_DI_INTERRUPT47        (ADS_EVT_DI_INTERRUPT0 + 47)

#define ADS_EVT_DI_INTERRUPT48        (ADS_EVT_DI_INTERRUPT0 + 48)
#define ADS_EVT_DI_INTERRUPT49        (ADS_EVT_DI_INTERRUPT0 + 49)
#define ADS_EVT_DI_INTERRUPT50        (ADS_EVT_DI_INTERRUPT0 + 50)
#define ADS_EVT_DI_INTERRUPT51        (ADS_EVT_DI_INTERRUPT0 + 51)
#define ADS_EVT_DI_INTERRUPT52        (ADS_EVT_DI_INTERRUPT0 + 52)
#define ADS_EVT_DI_INTERRUPT53        (ADS_EVT_DI_INTERRUPT0 + 53)
#define ADS_EVT_DI_INTERRUPT54        (ADS_EVT_DI_INTERRUPT0 + 54)
#define ADS_EVT_DI_INTERRUPT55        (ADS_EVT_DI_INTERRUPT0 + 55)

#define ADS_EVT_DI_INTERRUPT56        (ADS_EVT_DI_INTERRUPT0 + 56)
#define ADS_EVT_DI_INTERRUPT57        (ADS_EVT_DI_INTERRUPT0 + 57)
#define ADS_EVT_DI_INTERRUPT58        (ADS_EVT_DI_INTERRUPT0 + 58)
#define ADS_EVT_DI_INTERRUPT59        (ADS_EVT_DI_INTERRUPT0 + 59)
#define ADS_EVT_DI_INTERRUPT60        (ADS_EVT_DI_INTERRUPT0 + 60)
#define ADS_EVT_DI_INTERRUPT61        (ADS_EVT_DI_INTERRUPT0 + 61)
#define ADS_EVT_DI_INTERRUPT62        (ADS_EVT_DI_INTERRUPT0 + 62)
#define ADS_EVT_DI_INTERRUPT63        (ADS_EVT_DI_INTERRUPT0 + 63)

#define ADS_EVT_DI_INTERRUPT64        (ADS_EVT_DI_INTERRUPT0 + 64)
#define ADS_EVT_DI_INTERRUPT65        (ADS_EVT_DI_INTERRUPT0 + 65)
#define ADS_EVT_DI_INTERRUPT66        (ADS_EVT_DI_INTERRUPT0 + 66)
#define ADS_EVT_DI_INTERRUPT67        (ADS_EVT_DI_INTERRUPT0 + 67)
#define ADS_EVT_DI_INTERRUPT68        (ADS_EVT_DI_INTERRUPT0 + 68)
#define ADS_EVT_DI_INTERRUPT69        (ADS_EVT_DI_INTERRUPT0 + 69)
#define ADS_EVT_DI_INTERRUPT70        (ADS_EVT_DI_INTERRUPT0 + 70)
#define ADS_EVT_DI_INTERRUPT71        (ADS_EVT_DI_INTERRUPT0 + 71)

#define ADS_EVT_DI_INTERRUPT72        (ADS_EVT_DI_INTERRUPT0 + 72)
#define ADS_EVT_DI_INTERRUPT73        (ADS_EVT_DI_INTERRUPT0 + 73)
#define ADS_EVT_DI_INTERRUPT74        (ADS_EVT_DI_INTERRUPT0 + 74)
#define ADS_EVT_DI_INTERRUPT75        (ADS_EVT_DI_INTERRUPT0 + 75)
#define ADS_EVT_DI_INTERRUPT76        (ADS_EVT_DI_INTERRUPT0 + 76)
#define ADS_EVT_DI_INTERRUPT77        (ADS_EVT_DI_INTERRUPT0 + 77)
#define ADS_EVT_DI_INTERRUPT78        (ADS_EVT_DI_INTERRUPT0 + 78)
#define ADS_EVT_DI_INTERRUPT79        (ADS_EVT_DI_INTERRUPT0 + 79)

#define ADS_EVT_DI_INTERRUPT80        (ADS_EVT_DI_INTERRUPT0 + 80)
#define ADS_EVT_DI_INTERRUPT81        (ADS_EVT_DI_INTERRUPT0 + 81)
#define ADS_EVT_DI_INTERRUPT82        (ADS_EVT_DI_INTERRUPT0 + 82)
#define ADS_EVT_DI_INTERRUPT83        (ADS_EVT_DI_INTERRUPT0 + 83)
#define ADS_EVT_DI_INTERRUPT84        (ADS_EVT_DI_INTERRUPT0 + 84)
#define ADS_EVT_DI_INTERRUPT85        (ADS_EVT_DI_INTERRUPT0 + 85)
#define ADS_EVT_DI_INTERRUPT86        (ADS_EVT_DI_INTERRUPT0 + 86)
#define ADS_EVT_DI_INTERRUPT87        (ADS_EVT_DI_INTERRUPT0 + 87)

#define ADS_EVT_DI_INTERRUPT88        (ADS_EVT_DI_INTERRUPT0 + 88)
#define ADS_EVT_DI_INTERRUPT89        (ADS_EVT_DI_INTERRUPT0 + 89)
#define ADS_EVT_DI_INTERRUPT90        (ADS_EVT_DI_INTERRUPT0 + 90)
#define ADS_EVT_DI_INTERRUPT91        (ADS_EVT_DI_INTERRUPT0 + 91)
#define ADS_EVT_DI_INTERRUPT92        (ADS_EVT_DI_INTERRUPT0 + 92)
#define ADS_EVT_DI_INTERRUPT93        (ADS_EVT_DI_INTERRUPT0 + 93)
#define ADS_EVT_DI_INTERRUPT94        (ADS_EVT_DI_INTERRUPT0 + 94)
#define ADS_EVT_DI_INTERRUPT95        (ADS_EVT_DI_INTERRUPT0 + 95)

#define ADS_EVT_DI_INTERRUPT96        (ADS_EVT_DI_INTERRUPT0 + 96)
#define ADS_EVT_DI_INTERRUPT97        (ADS_EVT_DI_INTERRUPT0 + 97)
#define ADS_EVT_DI_INTERRUPT98        (ADS_EVT_DI_INTERRUPT0 + 98)
#define ADS_EVT_DI_INTERRUPT99        (ADS_EVT_DI_INTERRUPT0 + 99)
#define ADS_EVT_DI_INTERRUPT100       (ADS_EVT_DI_INTERRUPT0 + 100)
#define ADS_EVT_DI_INTERRUPT101       (ADS_EVT_DI_INTERRUPT0 + 101)
#define ADS_EVT_DI_INTERRUPT102       (ADS_EVT_DI_INTERRUPT0 + 102)
#define ADS_EVT_DI_INTERRUPT103       (ADS_EVT_DI_INTERRUPT0 + 103)

#define ADS_EVT_DI_INTERRUPT104       (ADS_EVT_DI_INTERRUPT0 + 104)
#define ADS_EVT_DI_INTERRUPT105       (ADS_EVT_DI_INTERRUPT0 + 105)
#define ADS_EVT_DI_INTERRUPT106       (ADS_EVT_DI_INTERRUPT0 + 106)
#define ADS_EVT_DI_INTERRUPT107       (ADS_EVT_DI_INTERRUPT0 + 107)
#define ADS_EVT_DI_INTERRUPT108       (ADS_EVT_DI_INTERRUPT0 + 108)
#define ADS_EVT_DI_INTERRUPT109       (ADS_EVT_DI_INTERRUPT0 + 109)
#define ADS_EVT_DI_INTERRUPT110       (ADS_EVT_DI_INTERRUPT0 + 110)
#define ADS_EVT_DI_INTERRUPT111       (ADS_EVT_DI_INTERRUPT0 + 111)

#define ADS_EVT_DI_INTERRUPT112       (ADS_EVT_DI_INTERRUPT0 + 112)
#define ADS_EVT_DI_INTERRUPT113       (ADS_EVT_DI_INTERRUPT0 + 113)
#define ADS_EVT_DI_INTERRUPT114       (ADS_EVT_DI_INTERRUPT0 + 114)
#define ADS_EVT_DI_INTERRUPT115       (ADS_EVT_DI_INTERRUPT0 + 115)
#define ADS_EVT_DI_INTERRUPT116       (ADS_EVT_DI_INTERRUPT0 + 116)
#define ADS_EVT_DI_INTERRUPT117       (ADS_EVT_DI_INTERRUPT0 + 117)
#define ADS_EVT_DI_INTERRUPT118       (ADS_EVT_DI_INTERRUPT0 + 118)
#define ADS_EVT_DI_INTERRUPT119       (ADS_EVT_DI_INTERRUPT0 + 119)

#define ADS_EVT_DI_INTERRUPT120       (ADS_EVT_DI_INTERRUPT0 + 120)
#define ADS_EVT_DI_INTERRUPT121       (ADS_EVT_DI_INTERRUPT0 + 121)
#define ADS_EVT_DI_INTERRUPT122       (ADS_EVT_DI_INTERRUPT0 + 122)
#define ADS_EVT_DI_INTERRUPT123       (ADS_EVT_DI_INTERRUPT0 + 123)
#define ADS_EVT_DI_INTERRUPT124       (ADS_EVT_DI_INTERRUPT0 + 124)
#define ADS_EVT_DI_INTERRUPT125       (ADS_EVT_DI_INTERRUPT0 + 125)
#define ADS_EVT_DI_INTERRUPT126       (ADS_EVT_DI_INTERRUPT0 + 126)
#define ADS_EVT_DI_INTERRUPT127       (ADS_EVT_DI_INTERRUPT0 + 127)

#define ADS_EVT_DI_INTERRUPT136       (ADS_EVT_DI_INTERRUPT0 + 136)
#define ADS_EVT_DI_INTERRUPT160       (ADS_EVT_DI_INTERRUPT0 + 160) 
#define ADS_EVT_DI_INTERRUPT184       (ADS_EVT_DI_INTERRUPT0 + 184) 
/* End of new device driver structure. */


#define ADS_EVT_FIFO_OVERFLOW_DI      0x3800
#define ADS_EVT_FIFO_UNDERFLOW_DI     0x3801
#define ADS_EVT_BUFFER1_FULL_DI       0x3802
#define ADS_EVT_BUFFER2_FULL_DI       0x3803

#define ADS_EVT_DI_LOBUFREADY         0x3804   
#define ADS_EVT_DI_HIBUFREADY         0x3805

#define ADS_EVT_DI_TERMINATED         0x3810   
#define ADS_EVT_DI_OVERRUN            0x3811   
#define ADS_EVT_DI_PATTERNMATCH       0x3812   
#define ADS_EVT_DI_STATUSCHANGE       0x3813 


/* New driver structure  */

/* New Event ID for DI Pattern Match. Max 32 ports.  */
#define ADS_EVT_DI_PATTERNMATCH_PORT0       0x3814
#define ADS_EVT_DI_PATTERNMATCH_PORT12      (ADS_EVT_DI_PATTERNMATCH_PORT0 + 12)
#define ADS_EVT_DI_PATTERNMATCH_PORT31      (ADS_EVT_DI_PATTERNMATCH_PORT0 + 31)

/* DI Status Change Event Type: port 0 ~ port 31 */
#define ADS_EVT_DI_STATUSCHANGE_PORT0       0x3834
#define ADS_EVT_DI_STATUSCHANGE_PORT1       (ADS_EVT_DI_STATUSCHANGE_PORT0 + 1 )
#define ADS_EVT_DI_STATUSCHANGE_PORT13      (ADS_EVT_DI_STATUSCHANGE_PORT0 + 13)
#define ADS_EVT_DI_STATUSCHANGE_PORT31      (ADS_EVT_DI_STATUSCHANGE_PORT0 + 31)

/* End of new driver structure */



#define ADS_EVT_FIFO_OVERFLOW_DO      0x4800
#define ADS_EVT_FIFO_UNDERFLOW_DO     0x4801
#define ADS_EVT_BUFFER1_EMPTY_DO      0x4802
#define ADS_EVT_BUFFER2_EMPTY_DO      0x4803

#define ADS_EVT_DO_LOBUFTRANS         0x4804
#define ADS_EVT_DO_HIBUFTRANS         0x4805
#define ADS_EVT_DO_TERMINATED         0x4810
#define ADS_EVT_DO_UNDERRUN           0x4811


#define ADS_EVT_TERMINATE_CNT0       0x5000
#define ADS_EVT_TERMINATE_CNT1       (ADS_EVT_TERMINATE_CNT0 + 1)
#define ADS_EVT_TERMINATE_CNT2       (ADS_EVT_TERMINATE_CNT0 + 2)
#define ADS_EVT_TERMINATE_CNT3       (ADS_EVT_TERMINATE_CNT0 + 3)
#define ADS_EVT_TERMINATE_CNT4       (ADS_EVT_TERMINATE_CNT0 + 4)
#define ADS_EVT_TERMINATE_CNT5       (ADS_EVT_TERMINATE_CNT0 + 5)
#define ADS_EVT_TERMINATE_CNT6       (ADS_EVT_TERMINATE_CNT0 + 6)
#define ADS_EVT_TERMINATE_CNT7       (ADS_EVT_TERMINATE_CNT0 + 7)


#define ADS_EVT_OVERFLOW_CNT0        0x5100
#define ADS_EVT_OVERFLOW_CNT1        (ADS_EVT_OVERFLOW_CNT0 + 1)
#define ADS_EVT_OVERFLOW_CNT2        (ADS_EVT_OVERFLOW_CNT0 + 2)
#define ADS_EVT_OVERFLOW_CNT3        (ADS_EVT_OVERFLOW_CNT0 + 3)
#define ADS_EVT_OVERFLOW_CNT4        (ADS_EVT_OVERFLOW_CNT0 + 4)
#define ADS_EVT_OVERFLOW_CNT5        (ADS_EVT_OVERFLOW_CNT0 + 5)
#define ADS_EVT_OVERFLOW_CNT6        (ADS_EVT_OVERFLOW_CNT0 + 6)
#define ADS_EVT_OVERFLOW_CNT7        (ADS_EVT_OVERFLOW_CNT0 + 7)


#define ADS_EVT_UNDERFLOW_CNT0       0x5200
#define ADS_EVT_UNDERFLOW_CNT1       (ADS_EVT_UNDERFLOW_CNT0 + 1)
#define ADS_EVT_UNDERFLOW_CNT2       (ADS_EVT_UNDERFLOW_CNT0 + 2)
#define ADS_EVT_UNDERFLOW_CNT3       (ADS_EVT_UNDERFLOW_CNT0 + 3)
#define ADS_EVT_UNDERFLOW_CNT4       (ADS_EVT_UNDERFLOW_CNT0 + 4)
#define ADS_EVT_UNDERFLOW_CNT5       (ADS_EVT_UNDERFLOW_CNT0 + 5)
#define ADS_EVT_UNDERFLOW_CNT6       (ADS_EVT_UNDERFLOW_CNT0 + 6)
#define ADS_EVT_UNDERFLOW_CNT7       (ADS_EVT_UNDERFLOW_CNT0 + 7)


#define ADS_EVT_OVERCOMPARE_CNT0     0x5300
#define ADS_EVT_OVERCOMPARE_CNT1     (ADS_EVT_OVERCOMPARE_CNT0 + 1)
#define ADS_EVT_OVERCOMPARE_CNT2     (ADS_EVT_OVERCOMPARE_CNT0 + 2)
#define ADS_EVT_OVERCOMPARE_CNT3     (ADS_EVT_OVERCOMPARE_CNT0 + 3)


#define ADS_EVT_UNDERCOMPARE_CNT0     0x5400
#define ADS_EVT_UNDERCOMPARE_CNT1     (ADS_EVT_UNDERCOMPARE_CNT0 + 1)
#define ADS_EVT_UNDERCOMPARE_CNT2     (ADS_EVT_UNDERCOMPARE_CNT0 + 2)
#define ADS_EVT_UNDERCOMPARE_CNT3     (ADS_EVT_UNDERCOMPARE_CNT0 + 3)


#define ADS_EVT_INTERRUPT_TIMER0     0x5500       
#define ADS_EVT_INTERRUPT_TIMER1     (ADS_EVT_INTERRUPT_TIMER0 + 1)
#define ADS_EVT_INTERRUPT_TIMER2     (ADS_EVT_INTERRUPT_TIMER0 + 2)
#define ADS_EVT_INTERRUPT_TIMER3     (ADS_EVT_INTERRUPT_TIMER0 + 3)
#define ADS_EVT_INTERRUPT_TIMER4     (ADS_EVT_INTERRUPT_TIMER0 + 4)
#define ADS_EVT_INTERRUPT_TIMER5     (ADS_EVT_INTERRUPT_TIMER0 + 5)
#define ADS_EVT_INTERRUPT_TIMER6     (ADS_EVT_INTERRUPT_TIMER0 + 6)
#define ADS_EVT_INTERRUPT_TIMER7     (ADS_EVT_INTERRUPT_TIMER0 + 7)


#define ADS_EVT_INDEX_CNT0           0x5800
#define ADS_EVT_INDEX_CNT1           (ADS_EVT_INDEX_CNT0 + 1)
#define ADS_EVT_INDEX_CNT2           (ADS_EVT_INDEX_CNT0 + 2)
#define ADS_EVT_INDEX_CNT3           (ADS_EVT_INDEX_CNT0 + 3)


#endif
