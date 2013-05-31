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
#ifndef HINTMESSAGE_H
#define HINTMESSAGE_H

#include "../public/UtlVector.h"

class CBaseEntity;
class CBasePlayer;

class CHintMessage 
{
    public :

        CHintMessage( const char *hintString, bool isHint, CUtlVector< const char* > *args, float duration );
        ~CHintMessage( void );

        float GetDuration( void ) const { return m_duration; }
        void Send( CBaseEntity *client );

    public :

        const char                 *m_hintString;         /*     0     4 */
        bool                        m_isHint;             /*     4     1 */
        CUtlVector< char* >         m_args;               /*     8    20 */
        float                       m_duration;           /*    28     4 */

    /* size: 32, cachelines: 1, members: 4 */
    /* sum members: 29, holes: 1, sum holes: 3 */
    /* last cacheline: 32 bytes */
};

class CHintMessageQueue 
{
    public:

        void Reset( void );
        void Update( CBaseEntity *client );
        bool AddMessage( const char *message, float duration, bool isHint, CUtlVector< const char* > *args );
        bool IsEmpty() { return m_messages.Count() == 0; }

    private:

        float                           m_tmMessageEnd;     /*     0     4 */
        CUtlVector< CHintMessage* >     m_messages;         /*     4    20 */

    /* size: 24, cachelines: 1, members: 2 */
    /* last cacheline: 24 bytes */
};

#endif // HINTMESSAGE_H
