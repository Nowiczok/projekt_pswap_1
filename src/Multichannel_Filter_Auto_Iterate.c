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

extern void initPLL(void);
extern void initExternalMemory(void);

#define TAPS1 255
#define WINDOWS1 127<<14

#define TAPS2 1023
#define WINDOWS2 511<<14

#define TAPSIZE1 256
#define WINDOWSIZE1 128

#define TAPSIZE2 1024
#define WINDOWSIZE2 512

#define DMAIntrSource 27  /*FIR DMA interrupt source */

void ChannelscompISR(uint32_t iid, void *handlerarg);


float In_Buf1[256+TAPSIZE1-1] = {
								#include "indata256.dat"
								};
float Coeff_Buf1[256]={
							#include "coeffs256.dat"
						};
float Out_Buf1[256];

float In_Buf2[1024+TAPSIZE2-1] ={
								#include "indata1024.dat"
								};
float Coeff_Buf2[1024] = {
							#include "coeffs1024.dat"	
							};
float Out_Buf2[1024];

int IntrCount =0;


/* TCB (Transfer Control Block) Structure */

/*CP[18:0]          ---------------      FIRCTL2
  CP[18:0] -0x1     ---------------      II ---- should point to x(n-N-1) ---- x(0)
  CP[18:0] -0x2     ----------------     IM
  CP[18:0] -0x3     -----------------    IL/IC
  CP[18:0] -0x4     -----------------    IB  ---- Base address for input circular buffer
  CP[18:0] -0x5     -----------------    OI
  CP[18:0] -0x6     -----------------    OM
  CP[18:0] -0x7     -----------------    OL/OC
  CP[18:0] -0x8     -----------------    OB  ---- Base address for output circular buffer
  CP[18:0] -0x9     -----------------    CI  --- Coefficient Index should point to c(N-1) 
  CP[18:0] -0xA     -----------------    CM
  CP[18:0] -0xB     -----------------    CBL ---- Coefficient buffer Length
  CP[18:0] -0xC     -----------------    CP */

int TCB_Buf1[13] = {0,TAPSIZE1,-1,(int)Coeff_Buf1+255,(int)Out_Buf1,256,1,(int)Out_Buf1,(int)In_Buf1,256+TAPSIZE1-1,1,(int)In_Buf1,0};
int TCB_Buf2[13] = {0,TAPSIZE2,-1,(int)Coeff_Buf2+1023,(int)Out_Buf2,1024,1,(int)Out_Buf2,(int)In_Buf2,1024+TAPSIZE2-1,1,(int)In_Buf2,0};


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

	/* Selecting FIR accelerator */

	*pPMCTL1&=~(BIT_17|BIT_18);
	*pPMCTL1|=FIRACCSEL;

	//PMCTL1 effect latency
	NOP();NOP();NOP();NOP();


	sysreg_bit_set(sysreg_MODE1, IRPTEN);


	adi_int_InstallHandler (ADI_CID_P0I,ChannelscompISR,0,true);


	temp = *pPICR0;
	temp1 = 0xFFFFFFE0; // interrupt mask
	temp = temp & temp1;
	temp1 = DMAIntrSource;
	temp1 = temp | temp1;
	*pPICR0 = temp1;


	temp = TAPS1|WINDOWS1;
	TCB_Buf1[12]= temp; // Value of FIRCTL2 for TCB_Buf1;

	temp = TAPS2|WINDOWS2;
	TCB_Buf2[12] = temp; // Value of FIRCTL2;


	temp = (int)TCB_Buf2+12;
	TCB_Buf1[0] = temp; //channel 1 TCB points to channel 2 TCB

	temp = (int)TCB_Buf1+12;
	TCB_Buf2[0] = temp; //channel 2 TCB points to channel 1 TCB



	temp = (int)TCB_Buf1+12;
	*pCPFIR= temp;


	/* Set the values for FIRCTL1 */

	temp = FIR_CH2|FIR_EN| FIR_DMAEN|FIR_CAI;/* Auto iterate bit and the channels complete interrupt is set */

	*pFIRCTL1 = temp;  /* Enables and runs the DMA */

	while ((IntrCount && 0x2) == 0)
	{
		NOP();
	}


	temp = 0;
	*pFIRCTL1 = temp;
	NOP();

	puts("Multichannel Filter AutoIteration completed");
	return 0;
}


void ChannelscompISR(uint32_t iid, void *handlerarg)
{
	int temp;
	temp = IntrCount;
	temp = temp +1;
	IntrCount = temp;

	temp=*pFIRCTL1;
	temp&=~ (FIR_CAI);
	*pFIRCTL1=temp;
}

