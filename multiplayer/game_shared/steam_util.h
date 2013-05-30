
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
#ifndef STEAM_UTIL_H
#define STEAM_UTIL_H

class SteamFile 
{
    public:
    
        SteamFile( const char *filename );
        ~SteamFile( int );

        bool IsValid( void );
        bool Read( void *data, int length );

    private:
    
        byte       *m_fileData;           /*     0     4 */
        int         m_fileDataLength;     /*     4     4 */
        byte       *m_cursor;             /*     8     4 */
        int         m_bytesLeft;          /*    12     4 */

    /* size: 16, cachelines: 1, members: 4 */
    /* last cacheline: 16 bytes */
};

#endif // STEAM_UTIL_H