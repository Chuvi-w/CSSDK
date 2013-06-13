/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

#ifndef SHARED_UTIL_H
#define SHARED_UTIL_H

#include "extdll.h"

extern char *BufPrintf( char *buf, int &len, const char *fmt, ... );
extern char *NumAsString( int val );

#endif // SHARED_UTIL_H
