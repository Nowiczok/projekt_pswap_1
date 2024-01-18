/*********************************************************************************

 Copyright(c) 2012 Analog Devices, Inc. All Rights Reserved.

 This software is proprietary and confidential.  By using this software you agree
 to the terms of the associated Analog Devices License Agreement.

 *********************************************************************************/
/*
 * NAME:     ADDS_21479_EzKit.h
 * PURPOSE:  Header file with definitions use in the C-based talk-through examples
 */

/* include files */

#include <stdio.h>     /* Get declaration of puts and definition of NULL. */
#include <stdint.h>    /* Get definition of uint32_t. */
#include <assert.h>    /* Get the definition of support for standard C asserts. */
#include <builtins.h>  /* Get definitions of compiler builtin functions */
#include <platform_include.h>      /* System and IOP register bit and address definitions. */
#include <processor_include.h>
#include <services/int/adi_int.h>  /* Interrupt Handler API header. */
#include "adi_initialize.h"
#include <sru.h>
#include "ad1939.h"
/* Block Size per Audio Channel*/
#define NUM_SAMPLES 256

#define TAPS1 64
#define WINDOWS1 NUM_SAMPLES<<14

#define TAPS2 64
#define WINDOWS2 NUM_SAMPLES<<14

#define TAPSIZE1 65
#define TAPSIZE2 65

#define DMAIntrSource 27  /*FIR DMA interrupt source */



/* Number of stereo channels*/
#define NUM_RX_SLOTS 4
#define NUM_TX_SLOTS 4

#define RX_BLOCK_SIZE (NUM_SAMPLES*NUM_RX_SLOTS)
#define TX_BLOCK_SIZE (NUM_SAMPLES*NUM_TX_SLOTS)

#define SPIB_MODE (CPHASE | CLKPL)
#define AD1939_CS DS0EN
//#define AD1939_CS DS1EN
#define CLEAR_DSXEN_BITS 0xF00

#define SELECT_SPI_SLAVE(select) (*pSPIFLG &= ~(spiselect<<8))
#define DESELECT_SPI_SLAVE(select) (*pSPIFLG |= (spiselect<<8))

#define PCI  0x00080000
#define OFFSET 0x00080000
#define OFFSET_MASK 0x7FFFF


/* Function prototypes for this talk-through code*/
void initPLL(void);
void initExternalMemory(void);
static void clearDAIpins(void);
void initDAI(void);
void init1939viaSPI(void);
void initSPORT(void);

static void ProccessingTooLong(void);
void TalkThroughISR(uint32_t, void*);
void ClearSPORT(void);
void handleCodecData(unsigned int);

static void SetupSPI1939(unsigned int);
static void DisableSPI1939();
static void Configure1939Register(unsigned char,unsigned char,unsigned char,unsigned int);
static unsigned char Get1939Register(unsigned char,unsigned int);

static void Delay(int i);

// Global Variables
extern volatile int count;
extern volatile int isProcessing;
extern volatile int inputReady;
extern volatile int buffer_cntr;

