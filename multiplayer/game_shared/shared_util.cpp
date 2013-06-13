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

#include "extdll.h"
#include "util.h"
#include "shared_util.h"

char *BufPrintf( char *buf, int &len, const char *fmt, ... )
{
    va_list argptr;
    va_start( argptr, fmt );

    if( len > 0 )
    {
        _vsnprintf( buf, len, fmt, argptr );
        va_end( argptr );

        len -= strlen( buf );
        return &buf[ strlen( buf ) ];
    }

    return NULL;
}

char *NumAsString( int val )
{
    static char string[ 16 ][ 4 ];
    static int curstring;
    int len;

    curstring = ( curstring + 1 ) % 4;
    return BufPrintf( string[ curstring ], len, "%d", val );
}
