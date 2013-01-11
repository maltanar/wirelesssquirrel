/*----------------------------------------------------------------------------
 *  Demo Application for SimpliciTI
 * 
 *  L. Friedman
 *  Texas Instruments, Inc.
 *---------------------------------------------------------------------------- */

/********************************************************************************************
  Copyright 2004-2007 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights granted under
  the terms of a software license agreement between the user who downloaded the software,
  his/her employer (which must be your employer) and Texas Instruments Incorporated (the
  "License"). You may not use this Software unless you agree to abide by the terms of the
  License. The License limits your use, and you acknowledge, that the Software may not be
  modified, copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio frequency
  transceiver, which is integrated into your product. Other than for the foregoing purpose,
  you may not use, reproduce, copy, prepare derivative works of, modify, distribute,
  perform, display or sell this Software and/or its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE PROVIDED “AS IS”
  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY
  WARRANTY OF MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
  IN NO EVENT SHALL TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER LEGAL EQUITABLE
  THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES INCLUDING BUT NOT LIMITED TO ANY
  INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST
  DATA, COST OF PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY
  THIRD PARTIES (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com.
**************************************************************************************************/

#include "bsp.h"
#include "mrfi.h"
#include "nwk_types.h"
#include "nwk_api.h"
#include "bsp_leds.h"
#include "bsp_buttons.h"
#include "nwk.h"
#include "nwk_pll.h"

void toggleLED(uint8_t);
void sleep(void);

/* For FHSS systems, calls to NWK_DELAY() will also call nwk_pllBackgrounder()
 * during the delay time so if you use the system delay mechanism in a loop,
 * you don't need to also call the nwk_pllBackgrounder() function.
 */
#define SPIN_ABOUT_A_SECOND           NWK_DELAY(1000)
#define SPIN_ABOUT_A_QUARTER_SECOND   NWK_DELAY(250)

#define BAD_NEWS   (1)
#define CHECK_RATE (5)

void main (void)
{
  BSP_Init();

  /* If an on-the-fly device address is generated it must be done before the
   * call to SMPL_Init(). If the address is set here the ROM value will not 
   * be used. If SMPL_Init() runs before this IOCTL is used the IOCTL call 
   * will not take effect. One shot only. The IOCTL call below is conformal.
   */
#ifdef I_WANT_TO_CHANGE_DEFAULT_ROM_DEVICE_ADDRESS_PSEUDO_CODE
  {
    addr_t lAddr;

    createRandomAddress(&lAddr);
    SMPL_Ioctl(IOCTL_OBJ_ADDR, IOCTL_ACT_SET, &lAddr);
  }
#endif /* I_WANT_TO_CHANGE_DEFAULT_ROM_DEVICE_ADDRESS_PSEUDO_CODE */

  /* This call will fail because the join will fail since there is no Access Point
   * in this scenario. but we don't care -- just use the default link token later.
   * we supply a callback pointer to handle the message returned by the peer.
   */
  SMPL_Init(0);
  
  BSP_TURN_ON_LED1();

  /* wait for a button press... */
  do {
    FHSS_ACTIVE( nwk_pllBackgrounder( false ) ); /* manage FHSS */
    if (BSP_BUTTON1())
    {
      break;
    }
  } while (1);
  
  while(1){
     BSP_TURN_OFF_LED1();

  ///////////// Code section #2 begin: Switch to [HS XOSC] /////////////
  // Power up [HS XOSC] (SLEEP.OSC_PD = 0)
     SLEEP &= ~0x04;
   // Wait until [HS XOSC] is stable (SLEEP.XOSC_STB = 1)
     while ( ! (SLEEP & 0x40) );
   // Apply NOPs equivalent to 64 ìs
     asm("NOP");
   // Switch system clock source to [HS XOSC] (CLKCON.OSC = 0)
     CLKCON &= ~0x40;
   // Wait until system clock source has actually changed (CLKCON.OSC = 0)
     while ( CLKCON & 0x40 );
   // Power down [HS RCOSC] (SLEEP.OSC_PD = 1)
     SLEEP |= 0x04;

   // Clear IRCON.STIF (Sleep Timer CPU interrupt flag)
     IRCON &= 0x7F;
     SLEEP &= 0xFC; // Not required when resuming from PM0

     ST1=0x4C;
     ST2=0x1D;
     ST0=0x00;
  
     sleep();
     BSP_TURN_ON_LED1();
     for(int i =0; i<30; i++){
         SPIN_ABOUT_A_SECOND;
     }
  }
}

/////////////////////////////////////
//SLEEP
////////////////////////////////////
static void sleep(void)
{
  STIF=0;// Clear IRCON.STIF (Sleep Timer CPU interrupt flag)
  STIE=1;
  EA=1;
  
  ST1=0x4C;
  ST2=0x1D;
  ST0=0x00;
  
  SLEEP = (SLEEP & 0xFC) | 0x02;
  asm("NOP");
  asm("NOP");
  asm("NOP");
  // Power up [HS RCOSC] (SLEEP.OSC_PD = 0)
SLEEP &= ~0x04;
// Wait until [HS RCOSC] is stable (SLEEP.HFRC_STB = 1)
while ( ! (SLEEP & 0x20) );
// Switch system clock source to HS RCOSC (CLKCON.OSC = 1)
CLKCON |= 0x40;
// Wait until system clock source has actually changed (CLKCON.OSC = 1)
while ( !(CLKCON & 0x40) );
// Power down [HS XOSC] (SLEEP.OSC_PD = 1)
SLEEP |= 0x04;

  if (SLEEP & 0x03)
  {
    PCON |= 0x01;
    asm("NOP");
  }

}

void toggleLED(uint8_t which)
{
  if (1 == which)
  {
    BSP_TOGGLE_LED1();
  }
  else if (2 == which)
  {
    BSP_TOGGLE_LED2();
  }
  return;
}
