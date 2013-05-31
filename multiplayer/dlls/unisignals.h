
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
#ifndef UNISIGNALS_H
#define UNISIGNALS_H

class CUnifiedSignals 
{
    public :

        int Update( void );
        void Signal( int flSignal );
        int GetState( void );

    private:

        int     m_flSignal;           /*     0     4 */
        int     m_flState;            /*     4     4 */

    /* size: 8, cachelines: 1, members: 2 */
    /* last cacheline: 8 bytes */
};

#endif // UNISIGNALS_H
