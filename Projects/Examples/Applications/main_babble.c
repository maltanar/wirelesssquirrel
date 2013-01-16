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

/* unique ID for this network node, also used to decide the order of the algorithm steps */
#define UNIQUE_ID 0x00
/* number of iterations of the (broadcast-listen) or (listen-broadcast) loop */
#define BROADCAST_ITERATIONS 4
/* radio active period during broadcast and listen step (in multiples of 10ms)*/
#define RADIO_PERIOD 50
/* sleep period between broadcasting cycles (in seconds) */
#define SLEEP_PERIOD 10
/* set the number of bitfields to keep in memory */
#define BITFIELD_MEMORY 10

/* the message type is used in the first byte of every message to define what kind
 * of data is carried by the message */
typedef enum msg_type
{
	msg_sync = 0x01,
	msg_collect = 0x02,
	msg_count = 0x04
}msg_type;

/* to be able to use bit operations we have to use integers to represent the bitfield.
 * The devices are capable of tranmitting up to 10bytes in one transmission. As the first
 * byte of every message is used to define the message type, we can count up to 
 * 32+32+8 = 72 nodes with this basic scheme. It is easily extendable by adding
 * msg_count_2 message type, that carries another 72 nodes and so on */ 
static uint32_t bitfieldA;
static uint8_t buffer[MAX_APP_PAYLOAD];

/* global variables used for the active period functionality */
static volatile bool activePeriod;
static volatile uint32_t activePeriodOfLimit;
/* global variables used for the sleep timer configuration */
static unsigned char st2 =0, st1=0, st0=0;
static unsigned long countVal=0;
static unsigned long offset = 0;

/* reserve memory space in XDATA memory locations, that contains data in sleep modes 2 and 3 
 * and is used to store the last n bitfields */
__xdata static uint32_t bitfieldMemory[BITFIELD_MEMORY];
__xdata static uint8_t bfIdx = 0;

/* define Interrupt handler function for timer1 overflow IR */
#pragma vector=0x4b
__interrupt void timer1IR(void);

/* function prototypes */
static void countingAlgorithm(bool joinNetwork);
static void broadcastBitfield(void);
static void listenBitfield(void);
static void broadcastSync(void);
static bool waitSync(void);
static void storeBitfield(const uint32_t *bitfield);
static void transmitBitfields(void);
static void setActivePeriod(uint32_t timeoutX100ms);
static void initSleepTimer(void);
static void exitSleepTimer(void);
static void sleepPm2(uint32_t seconds);

void main (void)
{
	/* initialize the board hardware */
	BSP_Init();

	/* Assign a unique address to the radio device, based on the the unique NW id.
	 * The first three bytes can be selected arbitrarlily */
	addr_t lAddr = {{0x71, 0x56, 0x34, UNIQUE_ID}};
	SMPL_Ioctl(IOCTL_OBJ_ADDR, IOCTL_ACT_SET, &lAddr);
	
	/* This call will fail because the join will fail since there is no Access Point
	 * in this scenario. but we don't care -- just use the default link token later.
	 * we supply a callback pointer to handle the message returned by the peer */
	SMPL_Init(0);
		
	/* wait for a sync message or a button press to start the algorithm ... */
	countingAlgorithm(waitSync());
}

/* realizes the counting algorithm for the low energy mesh-network architecture
 * In: joinNetwork - if the node joins an existing network the first broadcast of
 * 					the sync message will be skipped */
static void countingAlgorithm(bool joinNetwork)
{
	while (1)
	{ 
		/* broadcast that a new algorithm iteration starts, to allow new nodes
		 * to join in. Nodes that join for the first time will skip this call */
		if (!joinNetwork) 
			broadcastSync();
		joinNetwork = false;
		
		/* start with listening / broadcasting depending on the unique id */
		if (UNIQUE_ID % 2 == 0)
		{
			for (uint8_t i = 0; i < BROADCAST_ITERATIONS; i++)
			{
				BSP_TURN_ON_LED1();
				broadcastBitfield();
				BSP_TURN_OFF_LED1();
				listenBitfield();
			}
		} 
		else
		{
			for (uint8_t i = 0; i < BROADCAST_ITERATIONS; i++) 
			{
				BSP_TURN_OFF_LED1();
				listenBitfield();
				BSP_TURN_ON_LED1();
				broadcastBitfield();
			}
		}
		
		/* store the bitfield in a list in memory */
		storeBitfield(&bitfieldA);
					  
		/* send SOC to sleep */
		BSP_TURN_OFF_LED1();
		sleepPm2(SLEEP_PERIOD);
	}
}

static void broadcastBitfield()
{
	/* wake up radio. */
	SMPL_Ioctl( IOCTL_OBJ_RADIO, IOCTL_ACT_RADIO_AWAKE, 0);
	
	/* set the own bit in the bitfield */
	bitfieldA |= 1 << UNIQUE_ID;
	
	/* copy the current bitfield into the transmit buffer */
	memset(buffer, 0, sizeof(buffer));
	buffer[0] = msg_count;
	memcpy(&buffer[1], &bitfieldA, sizeof(bitfieldA));
	
	/* broadcast the current bitfield one single time, then shutdown the radio
	 * and wait for the transmit period to expire */
	bool bcast_sent = false;
	setActivePeriod(RADIO_PERIOD);
	while (activePeriod)
	{
		if (!bcast_sent)
		{
			SMPL_Send(SMPL_LINKID_USER_UUD, buffer, sizeof(buffer));
			bcast_sent = true;
			/* shut the radio down early as possible to save energy */
			SMPL_Ioctl( IOCTL_OBJ_RADIO, IOCTL_ACT_RADIO_SLEEP, 0);
		}
	}

	/* fixed delay after active period for simulating shutting down the radio
	 * to keep duration of listenBitfield() and broadcastBitfield() equal */
	setActivePeriod(1);
    while(activePeriod);
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
	
    /* stay in receive mode for a fixed peridod of time */
	setActivePeriod(RADIO_PERIOD);
    while(activePeriod)
	{
		/* we might have received multiple messages, iterate over the receive buffer
		 * to make sure all messages are being processed */
		while (SMPL_SUCCESS == SMPL_Receive(SMPL_LINKID_USER_UUD, buffer, &len)) 
		{
			if (buffer[0] == msg_count) {
				tmp_bitfield = (uint32_t)*(&buffer[1]);
				bitfieldA |= tmp_bitfield;
			/* if a collect message is received, check if it's addressed to this 
		     * network node. Transmit the stored bitfields if it is */
			} else if (buffer[0] == msg_collect) {
				tmp_bitfield = (uint32_t)*(&buffer[1]);
				uint32_t cmp_bitfield = (1 << UNIQUE_ID);
				if (tmp_bitfield == cmp_bitfield)
					transmitBitfields();
			}
		}
	}
	
	/* fixed delay after active period for shutting down the radio to keep 
	 * duration of listenBitfield() and broadcastBitfield() equal */
	setActivePeriod(1);
	bool radioShutDown = false;
    while(activePeriod) 
	{
		/* shut the radio down */
		if (!radioShutDown) 
		{
			SMPL_Ioctl( IOCTL_OBJ_RADIO, IOCTL_ACT_RADIO_SLEEP, 0);
			radioShutDown = true;
		}
	}
}

/* enables the receiver and waits for a sync message from the network or a button
 * press. If a button press is detected, the node will start it's own network 
 * Return: true - if sync message was received
 * 		   false - if button press was detected */
static bool waitSync()
{
	uint8_t len = 0;
	bool syncMessageReceived = false;
	BSP_TURN_OFF_LED1();
	
	/* wake up radio. We need it to listen for broadcasts */
    SMPL_Ioctl( IOCTL_OBJ_RADIO, IOCTL_ACT_RADIO_AWAKE, 0);
    /* turn on RX. default is RX off. */
    SMPL_Ioctl( IOCTL_OBJ_RADIO, IOCTL_ACT_RADIO_RXON, 0);

	/* enter a infinite loop, listening for a sync message from the Network.
	 * When the message is received start the RTC to sync to the Nw and break 
	 * the loop */
    bool exitLoop = false;
	while (!exitLoop) 
	{
		if (SMPL_SUCCESS == SMPL_Receive(SMPL_LINKID_USER_UUD, buffer, &len)) 
		{
			BSP_TURN_ON_LED1();
			if (buffer[0] == msg_sync) {
				/* sync message from network received -> start the algorithm */ 
				syncMessageReceived = true;
				exitLoop = true;
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
	
	return syncMessageReceived;
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

/* function to store the current bitfield in a memory location that's not lost
 * when the device enters sleep mode 2 or 3 */
void storeBitfield(const uint32_t *bitfield)
{
	/* if the storage is full, shift the content by one and add the bitfield to 
	 * the end of the list */
	if (bfIdx >= BITFIELD_MEMORY) 
	{
		for (uint8_t i = 0; i < BITFIELD_MEMORY-1; i++)
		{
			bitfieldMemory[i] = bitfieldMemory[i+i];
		}
		bitfieldMemory[bfIdx-1] = *bitfield;
	} else
	{
		bitfieldMemory[bfIdx++] = *bitfield;
	}
}

/* opens up a link for a data-collection device to connect to, if the connection
 * is established, the stored bitfields are sent out. If no connection is established
 * after a period of time, the function returns and the device enters the sleep mode
 * again */
void transmitBitfields(void)
{
	/* open up a link for the data-collection device, by default this function
	 * waits for 5 seconds, use LINKLISTEN_MILLISECONDS_2_WAIT (nwk_api.c) to modify */
	linkID_t linkID;
	if (SMPL_TIMEOUT == SMPL_LinkListen(&linkID))
		/* return early if no connection is established within the timelimit */
		return;
	
	/* transmit all stored bitfields to the data-collection device */
	for (uint8_t i = 0; i < bfIdx; i++)
	{
		if (SMPL_SUCCESS != SMPL_Send(linkID, (uint8_t*)&bitfieldMemory[i], sizeof(bitfieldMemory[0])))
			/* transmission failed, end transmission */
			break;
	}
}

/* sets the board up to be able to use the sleep timer for waking up from power
 * mode 2
 * !!  MUST BE EXECUTED BEFORE ENTERING POWER MODES !! */
void initSleepTimer(void)
{
	/* Switch to High Speed RC Oscillator */
	
	/* Power up the high speed RC Osc (SLEEP.OSC_PD = 0) */
	SLEEP &= ~0x04;
	/* Wait until this Osc is stable (SLEEP.HFRC_STABLE = 1) */
	while(!(SLEEP & 0x20));
	/* Switch the system clock to the RC Osc (CLKCON.OSC = 1) */
	CLKCON |= 0x40;
	/* Wait until clock has definitely changed */
	while(!(CLKCON &= 0x40));
	/* Power down 32Mhz crystal Osc (SLEEP.OSC_PD=1) */
	SLEEP |= 0x04;
}

/* re-configures the board to the same configuration is was in before entering
 * sleep mode (switch to christal oscillator) */
void exitSleepTimer(void){
	/* Power up HS XOSC (SLEEP.OSC_PD = 0) */
	SLEEP &= ~0x04;
	/* Wait until XOSC is stable (SLEEP.XOSC_STB = 1) */
	while(!(SLEEP & 0x40));
	/* Apply NOPs */
	asm("NOP");
	/* Switch system clock to HS XOSC (CLKCON.OSC = 0) */
	CLKCON &= ~0x40;
	/* Wait until clock has changed */
	while(CLKCON & 0x40);
	/* Power down HS RCOSC (SLEEP.OSC_PD=1) */
	SLEEP|= 0x04; 
}

/* sends the device to sleep mode 2 for n seconds */
static void sleepPm2(uint32_t seconds)
{
	/* Setup 16Mhz clock as system clock - this is necessary to enter sleep mode */
	initSleepTimer();
	
	/* Convert number of sleep seconds into sleep timer cycles */
	offset = seconds * 32768;
	
	/* Set Sleep mode to PM2 */
	SLEEP = (SLEEP & 0xFC) | 0x02;
	
	/* Apply 3 NOPs to allow any corresponding interrupt blocking to take
	* effect before verifying SLEEP.MODE bits */
	asm("NOP");
	asm("NOP");
	asm("NOP");
	
	/* If no interrupts executed during the above NOPs the interrupts are
	* all blocked by this point.
	* If an ISR has fired the SLEEP.MODE bits are cleared and the mode
	* will not be entered */
	if(SLEEP & 0x03)
	{
		/* variables to store current register values */
		unsigned char ircon_prev = 0, ien_prev = 0;
		/* Store current interrupt register & clear interrupt flags */
		ircon_prev = IRCON;
		IRCON = 0x00;
		
		/* Store interrupt mask register & enable Sleep Timer interrupt */
		ien_prev = IEN0;
		IEN0 |= 0xA0;
		
		/* read the current count values for sleep timer, and use them to calculate
		* the current 24bit count value */ 
		st0 = ST0;
		st1 = ST1;
		st2 = ST2;
		countVal = (unsigned long)st2;
		countVal = countVal << 16;
		countVal |= (unsigned long)st1 << 8;
		countVal |= (unsigned long)st0;
		
		/* add an offset to the count value, defining the sleep duration 
		* formula: offset = t_sleep / 32kHz */
		countVal = countVal + offset;
		if(countVal>0xFFFFFF){countVal = countVal - 0xFFFFFF;}
		
		 /* set the new compare values for sleep timer */
		ST2 = (countVal>>16) & 0xFF;
		ST1 = (countVal>>8) & 0xFF;
		ST0 = countVal & 0xFF;
		
		/* wait until the compare values have been processed */
		while(!(ST0==(countVal & 0xFF)));
		
		/* Set PCON.IDLE to enter the power mode */
		PCON |= 0x01;
		
		/* SOC now in PM2 and will only wake up when Sleep timer times out */
		
		/* Apply a NOP as first instruction when exiting sleep mode */
		asm("NOP");

		/* Reestablish 32MHz clock as system clock */
		exitSleepTimer();
		
		/* restore register values to status before entering sleep mode */
		SLEEP &= 0xFC; 	// set cpu to pm0
		IRCON = ircon_prev;
		IEN0 = ien_prev;
	}
}

/* function for setting the global "activePeriod" variable to true after n * 100 ms have passed */
void setActivePeriod(uint32_t period)
{
	/* set the global "activePeriod" variable and define the overflow limit 
	 * (the times is configured to gererate one interrupt every 100ms) */
	activePeriod = true;
	activePeriodOfLimit = period;
	
	/* enable Timer 1 overflow interrupt */
	TIMIF |= (6 << 1); 	// Timer1 overflow interrupt mask
	IEN1 |= (1 << 1); 	// Timer1 interrupt enable
	
	/* enable global interrupts */
	IEN0 |= (1 << 7);	// Each interrupt source is individually enabled or 
						// disabled by setting its corresponding enable bit
	
	/* set the clock divede prescaler to the lowest possible value */
	T1CTL = 0x0C; //set the clk divide to 128 -> increment every 4us
	
	/* generate an interrupt every 10ms
	 * --> use module mode, with a compare value of 4us * 2500 = 10ms*/
	union modulo
	{ 
		uint16_t val_16; 
		uint8_t val_8[2]; 
	};
	union modulo mod;
	mod.val_16 = 2500;
	
	T1CC0L = mod.val_8[0];
	T1CC0H = mod.val_8[1];
    
	/* reset the counter to 0 */
	T1CNTL = 0;
	
	/* Start Timer 1 in modulo mode */
	T1CTL |= (1<<1);	
}
	
__interrupt void timer1IR(void){
	static uint32_t ofCnt = 0;
	ofCnt++;
	
	/* check if the active period has expired */
	if (ofCnt >= activePeriodOfLimit) 
	{
		activePeriod = false;
		ofCnt = 0;
		/* stop the timer */
		T1CTL &= !(1<<1);
	}
}