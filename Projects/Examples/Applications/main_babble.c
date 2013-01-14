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
#include "stdio.h"
#include "string.h"

/* the message type is used in the first byte of every message to define what kind
 * of data is carried by the message */
typedef enum msg_type
{
	msg_sync = 0x01,
	msg_count = 0x05
}msg_type;

/* to be able to use bit operations we have to use integers to represent the bitfield.
 * The devices are capable of tranmitting up to 10bytes in one transmission. As the first
 * byte of every message is used to define the message type, we can count up to 
 * 32+32+8 = 72 nodes with this basic scheme. It is easily extendable by adding
 * msg_count_2 message type, that carries another 72 nodes and so on */ 
static uint32_t bitfield_1;
static uint8_t buffer[MAX_APP_PAYLOAD];

/* For FHSS systems, calls to NWK_DELAY() will also call nwk_pllBackgrounder()
 * during the delay time so if you use the system delay mechanism in a loop,
 * you don't need to also call the nwk_pllBackgrounder() function.
 */
#define SPIN_ABOUT_A_SECOND           NWK_DELAY(1000)
#define SPIN_ABOUT_A_QUARTER_SECOND   NWK_DELAY(250)

#define CHECK_RATE (5)

#define UNIQUE_ID 0x01

static void countingAlgorithm(void);
static void broadcastBitfield(void);
static void listenBitfield(void);
static void broadcastSync(void);
static void waitSync(void);
static void printBitfield(const uint32_t* bf);

void printBitfield(const uint32_t *bf)
{
	printf("bf: ");
	for (uint8_t i = 0; i < 32 ; i++)
	{
		printf("%d", (*bf & 1<<i)? 1:0);
	}
	printf("\n");
}
static unsigned char st2 =0, st1=0, st0=0;
static unsigned long countVal=0;
void main (void)
{
	
  //////////////////////////////////////////////////////////////////////////
  // CODE SECTION 1 : Switch to High Speed RC Oscillator
  //     MUST BE EXECUTED BEFORE ENTERING POWER MODES
  //////////////////////////////////////////////////////////////////////////
  
  // Power up the high speed RC Osc (SLEEP.OSC_PD = 0)
  SLEEP &= ~0x04;
  // Wait until this Osc is stable (SLEEP.HFRC_STABLE = 1)
  while(!(SLEEP & 0x20));
  // Switch the system clock to the RC Osc (CLKCON.OSC = 1)
  CLKCON |= 0x40;
  // Wait until clock has definitely changed
  while(!(CLKCON &= 0x40));
  // Power down 32Mhz crystal Osc (SLEEP.OSC_PD=1)
  SLEEP |= 0x04;
  ////////////////////////////////////////////////////////////////////////////
  // CODE SECTION 1 : END
  ////////////////////////////////////////////////////////////////////////////
  
	BSP_Init();
	
	////////////////////////////////////////////////////////////////////
  // Enter sleep mode 2
  ////////////////////////////////////////////////////////////////////
  
	while(1)
	{
    // Set Sleep mode to PM2
    SLEEP = (SLEEP & 0xFC) | 0x02;
  
    // Apply 3 NOPs to allow any corresponding interrupt blocking to take
    // effect before verifying SLEEP.MODE bits.
    asm("NOP");
    asm("NOP");
    asm("NOP");
  
    // If no interrupts executed during the above NOPs the interrupts are
    // all blocked by this point.
    // If an ISR has fired the SLEEP.MODE bits are cleared and the mode
    // will not be entered.
    if(SLEEP & 0x03){
     
	  //Clear interrupt flags
	  IRCON = 0x00;
	  
	  // Enable Sleep Timer interrupt
	  IEN0 |= 0xA0;
	  
	  
	  
	  st0 = ST0;
	  st1 = ST1;
	  st2 = ST2;
	  
	  countVal = (unsigned long)st2;
	  countVal = countVal << 16;
	  countVal |= (unsigned long)st1 << 8;
	  countVal |= (unsigned long)st0;
	  
	  countVal = countVal + 0x28000;
	  
	  ST2 = (countVal>>16) & 0xFF;
	  ST1 = (countVal>>8) & 0xFF;
	  ST0 = countVal & 0xFF;
	  
	  while(!(ST0==(countVal & 0xFF)));
	  
	  BSP_TURN_OFF_LED1();
	  // Set PCON.IDLE to enter the power mode
	  PCON |= 0x01;
	 
	  // SOC now in PM2 and will only wake up when Sleep timer times out
	 
	  // Apply a NOP as first instruction when exiting sleep mode
	  asm("NOP");
    }

   BSP_TURN_ON_LED1();
   IRCON = 0x00;

   IEN0 = 0x00;
   SPIN_ABOUT_A_SECOND;
   SPIN_ABOUT_A_SECOND;
   SPIN_ABOUT_A_SECOND;
   SPIN_ABOUT_A_SECOND;
   SPIN_ABOUT_A_SECOND;
}	

/* Assign a unique address to the radio device, based on the the unique NW id.
* The first three bytes can be selected arbitrarlily */
	addr_t lAddr = {{0x71, 0x56, 0x34, UNIQUE_ID}};
	SMPL_Ioctl(IOCTL_OBJ_ADDR, IOCTL_ACT_SET, &lAddr);
	
	/* This call will fail because the join will fail since there is no Access Point
	* in this scenario. but we don't care -- just use the default link token later.
	* we supply a callback pointer to handle the message returned by the peer.
	*/
	SMPL_Init(0);
	
	/* wait for a sync message or a button press to start the process... */
	waitSync();
	
	BSP_TURN_ON_LED1();
	
	/* never coming back... */
	
	
  ////////////////////////////////////////////////////////////////////
  // Enter sleep mode 2
  ////////////////////////////////////////////////////////////////////
  
  // Set Sleep mode to PM2
  SLEEP = (SLEEP & 0xFC) | 0x02;
  
  // Apply 3 NOPs to allow any corresponding interrupt blocking to take
  // effect before verifying SLEEP.MODE bits.
  asm("NOP");
  asm("NOP");
  asm("NOP");
  
  // If no interrupts executed during the above NOPs the interrupts are
  // all blocked by this point.
  // If an ISR has fired the SLEEP.MODE bits are cleared and the mode
  // will not be entered.
  if(SLEEP & 0x03){
     
	  //Clear interrupt flags
	  IRCON &= 0x00;
	  
	  // Enable Sleep Timer interrupt
	  IEN0 &= 0xA0;
	  
	  // Set sleep timer to 1 minute
	  ST2=0x1D;
	  ST1=0x4C;
	  ST0=0x00;
	  
	  // Set PCON.IDLE to enter the power mode
	 PCON |= 0x01;
	 
	 // SOC now in PM2 and will only wake up when Sleep timer times out
	 
	 // Apply a NOP as first instruction when exiting sleep mode
	 asm("NOP");
  }
  
  BSP_TURN_OFF_LED1();
	
	/* but in case we do... */
	while (1) ;
}

static void countingAlgorithm()
{
	uint8_t i;

	while (1)
	{
		/* enter listen mode if button is pressed */
		if (BSP_BUTTON1())
		{
			listenBitfield();
		}
		/* broadcast sync message and unique ID */
		broadcastSync();
		broadcastBitfield();
		
		/* spoof MCU sleeping... */
		BSP_TURN_OFF_LED1();
		for (i=0; i<CHECK_RATE; ++i)
		{
			SPIN_ABOUT_A_SECOND;
		}
		BSP_TURN_ON_LED1();
	}
}

static void broadcastBitfield()
{
	/* wake up radio. */
	SMPL_Ioctl( IOCTL_OBJ_RADIO, IOCTL_ACT_RADIO_AWAKE, 0);
	
	/* set the own bit in the bitfield */
	bitfield_1 |= 1 << UNIQUE_ID;
	
	/* copy the current bitfield into the transmit buffer */
	memset(buffer, 0, sizeof(buffer));
	buffer[0] = msg_count;
	memcpy(&buffer[1], &bitfield_1, sizeof(bitfield_1));
	
	/* re-broadcast the current bitfield n times */
	for (int i = 0; i < 20; i++) 
	{
		NWK_DELAY(100);
		SMPL_Send(SMPL_LINKID_USER_UUD, buffer, sizeof(buffer));
		BSP_TOGGLE_LED1();
	}
	
	/* shut the radio down */
	SMPL_Ioctl( IOCTL_OBJ_RADIO, IOCTL_ACT_RADIO_SLEEP, 0);
}

static void listenBitfield()
{
	BSP_TURN_OFF_LED1();

	uint32_t tmp_bitfield = 0;
	uint8_t len = 0;
	
	/* wake up radio. We need it to listen for broadcasts */
    SMPL_Ioctl( IOCTL_OBJ_RADIO, IOCTL_ACT_RADIO_AWAKE, 0);
    /* turn on RX. default is RX off. */
    SMPL_Ioctl( IOCTL_OBJ_RADIO, IOCTL_ACT_RADIO_RXON, 0);

    /* stay in receive mode for a while */
    SPIN_ABOUT_A_QUARTER_SECOND;

    /* shut the radio down */
    SMPL_Ioctl( IOCTL_OBJ_RADIO, IOCTL_ACT_RADIO_SLEEP, 0);

    /* we might have received multiple messages, iterate over the receive buffer
	 * to make sure all messages are being processed */
	while (SMPL_SUCCESS == SMPL_Receive(SMPL_LINKID_USER_UUD, buffer, &len)) 
	{
		BSP_TURN_ON_LED1();
		if (buffer[0] == msg_count) {
			tmp_bitfield = (uint32_t)*(&buffer[1]);
			bitfield_1 |= tmp_bitfield;
		}
	}
	printBitfield(&bitfield_1);
}

static void waitSync()
{
	uint8_t len = 0;
	
	BSP_TURN_OFF_LED1();
	
	/* wake up radio. We need it to listen for broadcasts */
    SMPL_Ioctl( IOCTL_OBJ_RADIO, IOCTL_ACT_RADIO_AWAKE, 0);
    /* turn on RX. default is RX off. */
    SMPL_Ioctl( IOCTL_OBJ_RADIO, IOCTL_ACT_RADIO_RXON, 0);

	/* enter a infinite loop, listening for a sync message from the Network.
	 * When the message is received start the RTC to sync to the Nw and break 
	 * the loop */
    bool exitLoop = false;
	while (1) 
	{
		/* stay in receive mode for a while */
		NWK_DELAY(50);
	
		/* we might have received multiple messages, iterate over the receive buffer
		 * to make sure all messages are being processed */
		while (SMPL_SUCCESS == SMPL_Receive(SMPL_LINKID_USER_UUD, buffer, &len)) 
		{
			BSP_TURN_ON_LED1();
			if (buffer[0] == msg_sync) {
				// sync message from network received -> start the RTC 
				
				exitLoop = true;
				break;
			}
		}
		
		/* check if the user pressed the "start network" button */
		if (exitLoop || BSP_BUTTON1()) 
		{
			break;
		}
	}
	
	/* shut the radio down */
	SMPL_Ioctl( IOCTL_OBJ_RADIO, IOCTL_ACT_RADIO_SLEEP, 0);
}

/* broadcasts a message of the syncronization type */
static void broadcastSync()
{
	/* wake up radio. */
	SMPL_Ioctl( IOCTL_OBJ_RADIO, IOCTL_ACT_RADIO_AWAKE, 0);
	
	/* create a sync message in the transmit buffer */
	memset(buffer, 0, sizeof(buffer));
	buffer[0] = msg_sync;
	
	/* broadcast the sync message (only transmit the message type byte of the msg) */
	SMPL_Send(SMPL_LINKID_USER_UUD, buffer, sizeof(uint8_t));

	/* shut the radio down */
	SMPL_Ioctl( IOCTL_OBJ_RADIO, IOCTL_ACT_RADIO_SLEEP, 0);
}
	