/*----------------------------------------------------------------------------
 *      U S B  -  K e r n e l
 *----------------------------------------------------------------------------
 *      Name:    HIDUSER.C
 *      Purpose: HID Custom User Module
 *      Version: V1.10
 *----------------------------------------------------------------------------
*      This software is supplied "AS IS" without any warranties, express,
 *      implied or statutory, including but not limited to the implied
 *      warranties of fitness for purpose, satisfactory quality and
 *      noninfringement. Keil extends you a royalty-free right to reproduce
 *      and distribute executable files created using this software for use
 *      on NXP Semiconductors LPC family microcontroller devices only. Nothing
 *      else gives you the right to use this software.
 *
 *      Copyright (c) 2005-2009 Keil Software.
 *---------------------------------------------------------------------------*/

#include "usb.h"
#include "usbhw.h"
#include "hid.h"
#include "usbcfg.h"
#include "usbcore.h"
#include "hiduser.h"

#include "lib/ringbuf.h"
//#include "demo.h"

extern void GetInReport (void);
extern void SetOutReport (void);

uint8_t HID_Protocol;
uint8_t HID_IdleTime[HID_REPORT_NUM];


uint8_t HID_DepInEmpty = 1;

uint8_t HID_TxBuff[128];
uint8_t HID_RxBuff[64];
RINGBUF HID_TxRingBuff;
RINGBUF HID_RxRingBuff;

void USB_HID_Init(void)
{
	RINGBUF_Init(&HID_TxRingBuff,HID_TxBuff,sizeof(HID_TxBuff));
	RINGBUF_Init(&HID_RxRingBuff,HID_RxBuff,sizeof(HID_RxBuff));
}

void SetOutReport (void)
{
	uint8_t HID_receivedBuf[64];
	int numBytesRead,i;
	uint8_t c;
  // get data from USB into intermediate buffer
  numBytesRead = USB_ReadEP(HID_DATA_OUT,HID_receivedBuf);

	for(i = 0; i < numBytesRead;i++)
	{
		RINGBUF_Put(&HID_RxRingBuff,HID_receivedBuf[i]);
	}
}

void GetInReport  (void)
{
	uint8_t HID_SendBuf[64];
	int numBytesRead = 0;
	uint8_t c;

	while(RINGBUF_Get(&HID_TxRingBuff,&c) == 0)
	{
			HID_SendBuf[numBytesRead] = c;
			numBytesRead++;
			if(numBytesRead >= 64) break;
	}
  // send over USB
  if (numBytesRead > 0) 
	{
		USB_WriteEP (HID_DATA_IN, HID_SendBuf, 64);
  }
  else 
	{
    HID_DepInEmpty = 1;
  }
}

/*
 *  HID Get Report Request Callback
 *   Called automatically on HID Get Report Request
 *    Parameters:      None (global SetupPacket and EP0Buf)
 *    Return Value:    TRUE - Success, FALSE - Error
 */

uint32_t HID_GetReport (void) {

  /* ReportID = SetupPacket.wValue.WB.L; */
  switch (SetupPacket.wValue.WB.H) {
    case HID_REPORT_INPUT:
      GetInReport();
//      EP0Buf[0] = InReport;
      break; 
    case HID_REPORT_OUTPUT:
      return (FALSE);          /* Not Supported */
    case HID_REPORT_FEATURE:
      /* EP0Buf[] = ...; */
      /* break; */
      return (FALSE);          /* Not Supported */
  }
  return (TRUE);
}


/*
 *  HID Set Report Request Callback
 *   Called automatically on HID Set Report Request
 *    Parameters:      None (global SetupPacket and EP0Buf)
 *    Return Value:    TRUE - Success, FALSE - Error
 */

uint32_t HID_SetReport (void) {

  /* ReportID = SetupPacket.wValue.WB.L; */
  switch (SetupPacket.wValue.WB.H) {
    case HID_REPORT_INPUT:
      return (FALSE);          /* Not Supported */
    case HID_REPORT_OUTPUT:
     // OutReport = EP0Buf[0];
      SetOutReport();
      break;
    case HID_REPORT_FEATURE:
      return (FALSE);          /* Not Supported */
  }
  return (TRUE);
}


/*
 *  HID Get Idle Request Callback
 *   Called automatically on HID Get Idle Request
 *    Parameters:      None (global SetupPacket and EP0Buf)
 *    Return Value:    TRUE - Success, FALSE - Error
 */

uint32_t HID_GetIdle (void) {

  EP0Buf[0] = HID_IdleTime[SetupPacket.wValue.WB.L];
  return (TRUE);
}


/*
 *  HID Set Idle Request Callback
 *   Called automatically on HID Set Idle Request
 *    Parameters:      None (global SetupPacket)
 *    Return Value:    TRUE - Success, FALSE - Error
 */

uint32_t HID_SetIdle (void) {

  HID_IdleTime[SetupPacket.wValue.WB.L] = SetupPacket.wValue.WB.H;

  /* Idle Handling if needed */
  /* ... */

  return (TRUE);
}


/*
 *  HID Get Protocol Request Callback
 *   Called automatically on HID Get Protocol Request
 *    Parameters:      None (global SetupPacket)
 *    Return Value:    TRUE - Success, FALSE - Error
 */

uint32_t HID_GetProtocol (void) {

  EP0Buf[0] = HID_Protocol;
  return (TRUE);
}


/*
 *  HID Set Protocol Request Callback
 *   Called automatically on HID Set Protocol Request
 *    Parameters:      None (global SetupPacket)
 *    Return Value:    TRUE - Success, FALSE - Error
 */

uint32_t HID_SetProtocol (void) {

  HID_Protocol = SetupPacket.wValue.WB.L;

  /* Protocol Handling if needed */
  /* ... */

  return (TRUE);
}
