/*
** adi_initialize.c source file generated on October 2, 2012 at 16:20:08.
**
** Copyright (C) 2000-2012 Analog Devices Inc., All Rights Reserved.
**
** This file is generated automatically. You should not modify this source file,
** as your changes will be lost if this source file is re-generated.
*/

#include "adi_initialize.h"


int32_t adi_initComponents(void)
{
	int32_t result = 0;

#ifdef __HAS_SEC__
	result = adi_sec_Init();
#endif


	return result;
}

