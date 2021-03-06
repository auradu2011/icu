/*
*******************************************************************************
*
*   Copyright (C) 2009, International Business Machines
*   Corporation and others.  All Rights Reserved.
*
*******************************************************************************
*/

/**
 * Purpose: create local or static fe_verlist such as:
 * 
 *   const char *fe_verlist[] = { "4_2", "4_2_3", ..., NULL }; 
 */

#if defined(GLUE_VER)
#undef GLUE_VER
#endif

const char *fe_verlist[] = { 
#define GLUE_VER(x)  #x  ,
#include "icuglue/glver.h"
#undef GLUE_VER
    NULL
};

