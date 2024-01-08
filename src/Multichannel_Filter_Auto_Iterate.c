/*********************************************************************************

 Copyright(c) 2012 Analog Devices, Inc. All Rights Reserved.

 This software is proprietary and confidential.  By using this software you agree
 to the terms of the associated Analog Devices License Agreement.

 *********************************************************************************/

/*****************************************************************************
 * Multichannel_Filter_Auto_Iterate.c
 *****************************************************************************/
/*****************************************************************************
 This program perfoms FIR on two channels whose taplengths are 256 and 1024. The
 Window size of the two cannels is 128 and 512. Channel auto iterate is selected to
 iterate through the channels. once the FIR accelerator completes iterating through 
 the channels twice the DMA is disabled. channel complete DMA Interrupt is used to 
 check the number of iterations.
 
 *****************************************************************************/


#include <stdio.h>     /* Get declaration of puts and definition of NULL. */
#include <stdint.h>    /* Get definition of uint32_t. */
#include <builtins.h>  /* Get definitions of compiler builtin functions */
#include <platform_include.h>      /* System and IOP register bit and address definitions. */
#include <processor_include.h>
#include <services/int/adi_int.h>  /* Interrupt Handler API header. */
#include "adi_initialize.h"
#include "ADDS_21479_EzKit.h"

extern void initPLL(void);
extern void initExternalMemory(void);

void ChannelscompISR(uint32_t iid, void *handlerarg);

// this variable informs main flow that processing is done
// it is volatile, because it is set in interrupt
volatile bool iteration_done = false;

float In_Buf1[NUM_SAMPLES+TAPSIZE1-1] = {0};
float Coeff_Buf1[TAPSIZE1]={
							#include "coeffs256.dat"
						};
float Out_Buf1[NUM_SAMPLES] = {0};

float In_Buf2[NUM_SAMPLES+TAPSIZE2-1] = {0};
float Coeff_Buf2[TAPSIZE2] = {
							#include "coeffs1024.dat"	
							};
float Out_Buf2[NUM_SAMPLES] = {0};


/* TCB (Transfer Control Block) Structure */
/* TCB is a data structure that holds information about each channel */

/*CP[18:0]          ---------------      FIRCTL2
  CP[18:0] -0x1     ---------------      II ---- should point to x(n-N-1) ---- x(0)
  CP[18:0] -0x2     ----------------     IM
  CP[18:0] -0x3     -----------------    IL/IC  ---- input data count
  CP[18:0] -0x4     -----------------    IB  ---- Base address for input circular buffer
  CP[18:0] -0x5     -----------------    OI
  CP[18:0] -0x6     -----------------    OM
  CP[18:0] -0x7     -----------------    OL/OC  ---- output data count
  CP[18:0] -0x8     -----------------    OB  ---- Base address for output circular buffer
  CP[18:0] -0x9     -----------------    CI  --- Coefficient Index should point to c(N-1) 
  CP[18:0] -0xA     -----------------    CM
  CP[18:0] -0xB     -----------------    CBL ---- Coefficient buffer Length
  CP[18:0] -0xC     -----------------    CP */

int TCB_Buf1[13] = {
		0,
		TAPSIZE1,
		-1,
		(int)Coeff_Buf1+TAPS1,
		(int)(Out_Buf1),
		NUM_SAMPLES,
		1,
		(int)(Out_Buf1),
		(int)(In_Buf1),
		NUM_SAMPLES+TAPSIZE1-1,
		1,
		(int)(In_Buf1),
		0
};

int TCB_Buf2[13] = {
		0,
		TAPSIZE2,
		-1,
		(int)Coeff_Buf2+TAPS2,
		(int)Out_Buf2,
		NUM_SAMPLES,
		1,
		(int)Out_Buf2,
		(int)In_Buf2,
		NUM_SAMPLES+TAPSIZE2-1,
		1,
		(int)In_Buf2,
		0
};


int main( void )
{
	int temp;
	int temp1;
	/* Initialize managed drivers and/or services at the start of main(). */
	adi_initComponents();

	/* Initialize SHARC PLL*/
	initPLL();

	/* Initialize DDR2 SDRAM controller to access memory */
	initExternalMemory();

	/* Initialize DAI because the SPORT and SPI signals need to be routed*/
	initDAI();

	/* This function will configure the AD1939 codec on the 21469 EZ-KIT*/
	init1939viaSPI();

	/* Turn on SPORT0 TX and SPORT1 RX for Multichannel Operation*/
	initSPORT();

    /* Install and enable a handler for the SPORT1 Receiver interrupt.*/
    adi_int_InstallHandler(ADI_CID_P3I,TalkThroughISR,0,true);//interrupt(SIG_SP1,TalkThroughISR);

	/* Selecting FIR accelerator */
	*pPMCTL1&=~(BIT_17|BIT_18);
	*pPMCTL1|=FIRACCSEL;

	//PMCTL1 effect latency
	NOP();NOP();NOP();NOP();


	sysreg_bit_set(sysreg_MODE1, IRPTEN);

	// add interrupt that fires when accelerator finishes work
	adi_int_InstallHandler (ADI_CID_P0I,ChannelscompISR,0,true);

	// probably set interrupt priority
	temp = *pPICR0;
	temp1 = 0xFFFFFFE0; // interrupt mask
	temp = temp & temp1;
	temp1 = DMAIntrSource;
	temp1 = temp | temp1;
	*pPICR0 = temp1;

    // prepare TCB1
	temp = TAPS1|WINDOWS1;
	TCB_Buf1[12]= temp; // Value of FIRCTL2 for TCB_Buf1;

	// prepare TCB2
	temp = TAPS2|WINDOWS2;
	TCB_Buf2[12] = temp; // Value of FIRCTL2;

	// link TCB1 with TCB2
	temp = (int)TCB_Buf2+12;
	TCB_Buf1[0] = temp; //channel 1 TCB points to channel 2 TCB

	// link TCB2 with TCB1
	temp = (int)TCB_Buf1+12;
	TCB_Buf2[0] = temp; //channel 2 TCB points to channel 1 TCB

	// pass TCBs to accelerator
	temp = (int)TCB_Buf1+12;
	*pCPFIR= temp;

	/* Set the values for FIRCTL1 */
	// two channels with interrupt enabled, no auto iterate
	temp = FIR_CH2 | FIR_DMAEN;
	*pFIRCTL1 = temp;

    /* Be in infinite loop and do nothing until done.*/
    while(1) {
    		if(inputReady)
    		{
    			handleCodecData(buffer_cntr);
    		}
    }
}


void ChannelscompISR(uint32_t iid, void *handlerarg)
{
	iteration_done = true;
}


