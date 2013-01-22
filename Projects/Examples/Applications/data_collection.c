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
#define UNIQUE_ID 0xFE
/* number of iterations of the listen-broadcast loop */
#define BROADCAST_ITERATIONS 4
/* radio active period during broadcast and listen step (in multiples of 10ms) */
#define RADIO_PERIOD 50
/* radio active period during data collection cycle (in multiples of 10ms) */
#define COLLECT_PERIOD 200
/* sleep period between broadcasting cycles (in seconds)
 * ! has to be 1-2 seconds shorter than for the broacasting nodes */
#define SLEEP_PERIOD 6
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
static void gatherAlgorithm(void);
static void collectAlgorithm(void);
static bool collectFromNode(uint8_t node);
static void sendCollectMessage(uint8_t node);
static bool establishLink(linkID_t *linkID);
static void listenBitfield(void);
static bool waitSync(void);
static void storeBitfield(const uint32_t *bitfield);
static void setActivePeriod(uint32_t timeoutX100ms);
static void initSleepTimer(void);
static void exitSleepTimer(void);
static void sleepPm2(uint32_t seconds);
static void printBitfield(uint32_t *bf);
static void initUart(void);
static void txUart(char);

void main (void)
{
	/* initialize the board hardware */
	BSP_Init();

	/* Initialise the UART */
	initUart();
	
	/* Assign a unique address to the radio device, based on the the unique NW id.
	 * The first three bytes can be selected arbitrarlily */
	addr_t lAddr = {{UNIQUE_ID, 0xAB, 0xBC, 0xCD}};
	SMPL_Ioctl(IOCTL_OBJ_ADDR, IOCTL_ACT_SET, &lAddr);
	
	/* This call will fail because the join will fail since there is no Access Point
	 * in this scenario. but we don't care -- just use the default link token later.
	 * we supply a callback pointer to handle the message returned by the peer */
	SMPL_Init(0);
		
	/* start the gather algorithm ... */
	gatherAlgorithm();
}

/* realizes the gather algorithm for the low energy mesh-network architecture */
static void gatherAlgorithm()
{
	while (1)
	{ 
		/* wait for a Synchronization message from the network to sync in, or a button
		 * press to start gathering historic data from the nodes*/
		if (waitSync()) 
		{
			/* reset the stored bitfield and listen listen to the new broadcasts to
			 * collect build the current bitfield.
			 * To be able to use the same defines as for the broadcast nodes, use the
			 * same loop */
			bitfieldA = 0;
			for (uint8_t i = 0; i < BROADCAST_ITERATIONS; i++)
			{
				listenBitfield();
				listenBitfield();
			}
			
			/* print the result of the gather operation */
			printBitfield(&bitfieldA);
			
			/* store the bitfield in a list in memory */
			storeBitfield(&bitfieldA);
		} 
		/* user pushed a button to request collection of historic data, we still need to wait
		 * for the sync message */
		else 
		{
			collectAlgorithm();
		}
		
		BSP_TURN_OFF_LED1();
	}
}

static void listenBitfield()
{
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
				BSP_TOGGLE_LED1();
				tmp_bitfield = (uint32_t)*(&buffer[1]);
				bitfieldA |= tmp_bitfield;
			}
		}
	}
	/* shut down the radio */
	SMPL_Ioctl( IOCTL_OBJ_RADIO, IOCTL_ACT_RADIO_SLEEP, 0);
	BSP_TURN_OFF_LED1();
}

/* prints the lowest 8 bits of the bitfield for demo purposes */
static void printBitfield(uint32_t *bf)
{
	for (uint8_t i = 0; i < 8; i++)
	{
		printf("%c", (*bf & (1 << (7-i)))? '1' : '0');
	}
	printf("\n");
}

/* function to store the current bitfield in a memory location that's not lost
 * when the device enters sleep mode 2 or 3 */
void storeBitfield(const uint32_t *bitfield)
{
	uint32_t tmpBf;
	/* if the storage is full, shift the content by one and add the bitfield to 
	 * the end of the list */
	if (bfIdx >= BITFIELD_MEMORY) 
	{
		for (uint8_t i = 0; i < BITFIELD_MEMORY-1; i++)
		{
			tmpBf = bitfieldMemory[i+1];
			bitfieldMemory[i] = 0;
			bitfieldMemory[i] = tmpBf;
		}
		bitfieldMemory[bfIdx-1] = *bitfield;
	} else
	{
		bitfieldMemory[bfIdx++] = *bitfield;
	}
}

/* algorithm to collect historic data from all nodes of the network */
static void collectAlgorithm()
{
	/* iterate over the bitfield until all fields have been processed */	
	while (bitfieldA != 0)
	{
		/* ignore button pushed and wait for sync message */
		if (waitSync())
		{	
			collectFromNode(2);
			bitfieldA = 0;
//			
//			for (uint8_t i = 0; i < 8; i++)
//			{
//				if (bitfieldA & (1 << i)) {
//					collectFromNode(i);
//					/* mark the node as processed */
//					bitfieldA &= ~(1 << i);
//					break;
//				}
//			}
		}
	}
	printf("Finished\n");
}

/* sends a 'collect_message' to the requested node and tries to establish a direct
 * link to the node. If the link can be established, the function collects stored
 * bitfields from the node and prints them to the terminal */
static bool collectFromNode(uint8_t node)
{
	/* allocate memory space to store the received bitfields temporarly */
	uint32_t bf[BITFIELD_MEMORY];
	
	/* notify the target node of the collect request, send one request during
	 * every active radio period, to make sure the request arrives, and the duration
	 * matches on the different nodes */
	for (uint8_t i = 0; i < BROADCAST_ITERATIONS*2; i++)
		sendCollectMessage(node);
	
	/* wake up radio */
    SMPL_Ioctl( IOCTL_OBJ_RADIO, IOCTL_ACT_RADIO_AWAKE, 0);
	 /* turn on RX. default is RX off. */
    SMPL_Ioctl( IOCTL_OBJ_RADIO, IOCTL_ACT_RADIO_RXON, 0);
	
	/* try to establish a link to the node */
	linkID_t linkID;
	if (!establishLink(&linkID)) 
	{
			/* shut the radio down */
			SMPL_Ioctl( IOCTL_OBJ_RADIO, IOCTL_ACT_RADIO_SLEEP, 0);
			return false;
	}
	
	/* receive the stored bitfields */
	uint8_t len, idx = 0;
	memset(buffer, 0, sizeof(buffer));	
	/* stay in receive mode for a fixed period of time, (equal to the 
	 * time the node tries to send the stored data) */
	setActivePeriod(COLLECT_PERIOD);
	while (activePeriod)
	{
		if (SMPL_SUCCESS == SMPL_Receive(linkID, &buffer[0], &len))
		{
			/* current version of transmitBitfield function sends the last N versions
			 * of a bitfield with a length of 32bit */
			memcpy(&(bf[idx++]), &buffer, sizeof(uint32_t));
		}
	}	
	/* shut the radio down */
	SMPL_Ioctl( IOCTL_OBJ_RADIO, IOCTL_ACT_RADIO_SLEEP, 0);
	
	/* print the bitfields to the terminal */
	printf("Collecting from Node %d:\n", node);
	for (uint8_t i = 0; i < idx; i++)
		printBitfield(&bf[i]);
	
	return true;
}

/* make multiple attempts to open up a link to a node
 * IN/OUT: linkID - link ID of the new connection 
 * Returns: true if link could be established */
static bool establishLink(linkID_t *linkID)
{
	smplStatus_t linkStatus;
	BSP_TURN_ON_LED1();
	for (uint8_t i = 0; i < 8 ; i++)
	{
		if (SMPL_SUCCESS == (linkStatus = SMPL_Link(linkID))) 
			break;
	}
			
	if (linkStatus != SMPL_SUCCESS) 
		{
			printf("No Link to node");
			return false;
		}
	BSP_TURN_OFF_LED1();
	return true;
}

/* sends one collect message to the requested node, and blocks for duration of
 * RADIO_PERIOD */
static void sendCollectMessage(uint8_t node)
{
	/* define the bitfield that signals the target node a request */
	uint32_t collectBf = 1 << node;
	/* copy the bitfield into the transmit buffer */
	memset(buffer, 0, sizeof(buffer));
	buffer[0] = msg_collect;
	memcpy(&buffer[1], &collectBf, sizeof(collectBf));
	
	/* wake up radio */
    SMPL_Ioctl( IOCTL_OBJ_RADIO, IOCTL_ACT_RADIO_AWAKE, 0);
	
	/* broadcast the data collection request */
	bool bcast_sent = false;
	setActivePeriod(RADIO_PERIOD);
	while(activePeriod)
	{
		if (!bcast_sent) 
		{
			SMPL_Send(SMPL_LINKID_USER_UUD, buffer, sizeof(buffer));
			SMPL_Send(SMPL_LINKID_USER_UUD, buffer, sizeof(buffer));
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


/* enables the receiver and waits for a sync message from the network or a button
 * press. If a button press is detected, the node will start collecting historic data
 * Return: true - if sync message was received
 *                 false - if button press was detected */
static bool waitSync()
{
	uint8_t len = 0;
	bool syncMessageReceived = false;
	BSP_TURN_ON_LED1();
	
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
					if (buffer[0] == msg_sync) {
							/* sync message from network received -> start the algorithm */ 
							syncMessageReceived = true;
							exitLoop = true;
					}
			}
			
			/* check if the user pressed the "collect historic data" button */
			if (exitLoop || BSP_BUTTON1()) 
			{
					break;
			}
	}
	/* shut the radio down */
	SMPL_Ioctl( IOCTL_OBJ_RADIO, IOCTL_ACT_RADIO_SLEEP, 0);
	
	BSP_TURN_OFF_LED1();
	return syncMessageReceived;
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

/* Initialise parameters for Uart transmission */
void initUart()
{
	// Set Baud rate to 9600bps
	U0BAUD = 0x59;
	U0GCR |= 0x08;
	
	// Set UART mode and disable receiver
	U0CSR = (U0CSR | 0x80) & ~0x40;
	
	// Select correct I/O pins. Use alternative 1 from datasheet 13.4.6.4
	PERCFG &= ~0xFE;
	P0SEL = 0x3C; 
		
}

void txUart(char testByte)
{
	// Load bitfield into tx register
	U0DBUF = testByte;
	
	// Wait while byte is transmitted
	while(U0CSR & 0x01);
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