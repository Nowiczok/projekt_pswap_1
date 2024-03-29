/*********************************************************************************

 Copyright(c) 2012 Analog Devices, Inc. All Rights Reserved.

 This software is proprietary and confidential.  By using this software you agree
 to the terms of the associated Analog Devices License Agreement.

 *********************************************************************************/
/*
 * NAME:     initSPORT01_TDM_mode.c (Block-based Talkthrough)
 * PURPOSE:  Talkthrough framework for sending and receiving samples to the AD1939.
 * USAGE:    This file initializes the SPORTs for DMA Chaining
 */


#include "ADDS_21479_EzKit.h"

/*
 *  Here is the mapping between the SPORTS and the ADCs/DACs
 *  ADC1 -> DSP  : SPORT1A : TDM Channel 0,1
 *  ADC2 -> DSP  : SPORT1A : TDM Channel 2,3
 *  DSP -> DAC1 : SPORT0A : TDM Channel 0,1
 *  DSP -> DAC2 : SPORT0A : TDM Channel 2,3
 *  DSP -> DAC3 : SPORT0B : TDM Channel 0,1
 *  DSP -> DAC4 : SPORT0B : TDM Channel 2,3
 */

/* SPORT RX DMA destination buffers */
 	 int RxBlock_A0[RX_BLOCK_SIZE];
 	 int RxBlock_A1[RX_BLOCK_SIZE];

/* SPORT Tx DMA source buffers */
 	 int TxBlock_A0[TX_BLOCK_SIZE];
 	 int TxBlock_A1[TX_BLOCK_SIZE];

 	 int TxBlock_B0[TX_BLOCK_SIZE];
 	 int TxBlock_B1[TX_BLOCK_SIZE];


/* TCB blocks for Chaining
 * Each block will be used for:
 * Filling from the ADC
 * Processing filled data
 */

/* Set up the TCBs to rotate automatically */
int TCB_RxBlock_A0[4] = { 0, RX_BLOCK_SIZE, 1, 0};
int TCB_RxBlock_A1[4] = { 0, RX_BLOCK_SIZE, 1, 0};

int TCB_TxBlock_A0[4] = { 0, TX_BLOCK_SIZE, 1, 0};
int TCB_TxBlock_A1[4] = { 0, TX_BLOCK_SIZE, 1, 0};

int TCB_TxBlock_B0[4] = { 0, TX_BLOCK_SIZE, 1, 0};
int TCB_TxBlock_B1[4] = { 0, TX_BLOCK_SIZE, 1, 0};


void initSPORT(void)
{

/* Set up "ping-pong" chained DMAs for AD1939 */

/*Proceed from Block A0 to Block A1 */
/* Extract 19-bits of the receive buffer A1 address with setting PCI bit */
    TCB_RxBlock_A0[0] = (((unsigned int) TCB_RxBlock_A1 + 3)& OFFSET_MASK)|PCI ;
/*  Extract 19-bits of the receive buffer A0 address */
    TCB_RxBlock_A0[3] = ((int) RxBlock_A0)& OFFSET_MASK ;

/* Proceed from Block A0 to Block A1*/
/* Extract 19-bits of the transmit buffer A1 address */
    TCB_TxBlock_A0[0] = ((unsigned int) TCB_TxBlock_A1 + 3)& OFFSET_MASK ;
/*  Extract 19-bits of the transmit buffer A0 address */
    TCB_TxBlock_A0[3] = ((int) TxBlock_A0)& OFFSET_MASK ;
    
/* Proceed from Block B0 to Block B1 */
/* Extract 19-bits of the transmit buffer B1 address */
    TCB_TxBlock_B0[0] = ((unsigned int) TCB_TxBlock_B1 + 3)& OFFSET_MASK ;
/*  Extract 19-bits of the transmit buffer B0 address */
    TCB_TxBlock_B0[3] = ((int) TxBlock_B0)& OFFSET_MASK ;

/* Proceed from Block A1 to Block A0 */
/* Extract 19-bits of the receive buffer A0 address with setting PCI bit */
    TCB_RxBlock_A1[0] = (((unsigned int) TCB_RxBlock_A0 + 3)& OFFSET_MASK)|PCI ;
/*  Extract 19-bits of the receive buffer A1 address  */
    TCB_RxBlock_A1[3] = ((int) RxBlock_A1)& OFFSET_MASK ;

/* Proceed from Block A1 to Block A0 */
/* Extract 19-bits of the transmit buffer A0 address */
    TCB_TxBlock_A1[0] = ((unsigned int) TCB_TxBlock_A0 + 3)& OFFSET_MASK ;
/*  Extract 19-bits of the transmit buffer A1 address */
    TCB_TxBlock_A1[3] = ((int) TxBlock_A1)& OFFSET_MASK ;
    
/* Proceed from Block AB to Block B0 */
/* Extract 19-bits of the transmit buffer B0 address */
    TCB_TxBlock_B1[0] = ((unsigned int) TCB_TxBlock_B0 + 3)& OFFSET_MASK ;
/*  Extract 19-bits of the transmit buffer B1 address */
    TCB_TxBlock_B1[3] = ((int) TxBlock_B1)& OFFSET_MASK ;

    
/*Clear out SPORT 0/1 registers */
	*pSPMCTL0 = 0;
	*pSPMCTL1 = 0;
    *pSPCTL0 = 0;
    *pSPCTL1 = 0;

    
/*     Analog Input and output setup  */


/* External clock and frame syncs generated by AD1939 */
   	*pDIV0 = 0x00000000;  /* Transmitter (SPORT0) */
    *pDIV1 = 0x00000000;  /* Receiver (SPORT1) at 12.288 MHz SCLK and 48 kHz sample rate */
    
/* Configuring SPORT0 & SPORT1 for "Multichannel" mode */
/* This is synonymous TDM mode which is the operating mode for the AD1939 */
	
/* Enabling DMA Chaining for SPORT0 TX/SPORT1 RX */
/* Block 1 will be filled first */
    /* Initialize the chain pointer for the transmitter with PCI bit set to enable the interrupts after every TCB */
    *pCPSP0A = (unsigned int)TCB_TxBlock_A0 - OFFSET + 3 + PCI;
    /* Initialize the chain pointers for transmitters */
    *pCPSP0B = (unsigned int)TCB_TxBlock_B0 - OFFSET + 3;
    *pCPSP1A = (unsigned int)TCB_RxBlock_A0 - OFFSET + 3;
    
	
/* sport1 control register set up as a receiver in MCM
 * sport 1 control register SPCTL1 = 0x000C01F0
 * externally generated SCLK1 and RFS1
 */
	*pSPCTL1 = 	SCHEN_A | SDEN_A | SLEN32; 
	
/* sport0 control register set up as a transmitter in MCM */
/* sport 0 control register, SPCTL0 = 0x000C01F0 */
	*pSPCTL0 = 	SCHEN_B | SDEN_B | SCHEN_A | SDEN_A | SPTRAN | SLEN32;

/* sport1 receive & sport0 transmit multichannel word enable registers */
    *pSP1CS0 = 0x0000000F;	/* Set to receive on channel 0-3 on SPORT1 A */
    *pSP0CS0 = 0x0000000F;	/* Set to transmit on channel 0-3 on SPORT0 A/B */

/*  sport1 & sport0 receive & transmit multichannel companding enable registers
 *  no companding for our 4 RX and 4 TX active timeslots
 *  no companding on SPORT1 receive
 *  no companding on SPORT0 transmit
 */
	*pMR1CCS0 = *pMT0CCS0 = 0;

/* SPORT 0&1  Miscellaneous Control Bits Registers */
/* SP01MCTL = 0x000000E2,  Hold off on MCM enable,
 * and number of TDM slots to 8 active channels
 */

/* Multichannel Frame Delay=1, Number of Channels = 8, LB disabled */
	*pSPMCTL0 = NCH3 | MFD1;
	*pSPMCTL1 = NCH3 | MFD1;

    
    
/* Enable multichannel operation (SPORT mode and DMA in standby and ready) */
	*pSPMCTL0 |= MCEB | MCEA;
	*pSPMCTL1 |= MCEA;
}
