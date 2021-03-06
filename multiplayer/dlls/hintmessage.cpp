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
#include "hintmessage.h"

char *CloneString( const char *str )
{
    char *cloneStr = new char [ strlen(str)+1 ];
    strcpy( cloneStr, str );
    return cloneStr;
}

CHintMessage::CHintMessage( const char *hintString, bool isHint, CUtlVector< const char* > *args, float duration )
{
    // m_args.m_Memory.m_pMemory = 0;
    // m_args.m_Memory.m_NumAllocated = 0;
    // m_args.m_Memory.m_GrowSize = 0;

    m_hintString = hintString;
    m_duration = duration;

    if( args )
    {
        for( int i = 0; i< args->Count(); ++i )
        {
            m_args.AddToTail( CloneString( ( *args )[i] ) );
        }
    }
}

CHintMessage::~CHintMessage()
{
    for( int i = 0; i < m_args.Count(); ++i )
    {
        delete[] m_args[i];
    }

    m_args.RemoveAll();
}

void CHintMessage::Send( CBaseEntity *client )
{
    UTIL_ShowMessageArgs( m_hintString, client, &m_args, m_isHint );
}


void CHintMessageQueue::Reset()
{
    m_tmMessageEnd = 0;

    for( int i = 0; i < m_messages.Count(); ++i )
    {
        delete m_messages[ i ];
    }

    m_messages.RemoveAll();
}

void CHintMessageQueue::Update( CBaseEntity *client )
{
    if( gpGlobals->time > m_tmMessageEnd && m_messages.Count() )
    {
        CHintMessage *msg = m_messages[ 0 ];
        m_tmMessageEnd = gpGlobals->time + msg->GetDuration();

        UTIL_ShowMessageArgs( msg->m_hintString, client, &msg->m_args, msg->m_isHint );

        delete msg;
        m_messages.Remove( 0 );
    }
}

bool CHintMessageQueue::AddMessage( const char* message, float duration, bool isHint, CUtlVector< const char * > *args )
{
    CHintMessage *msg = new CHintMessage( message, isHint, args, duration );
    m_messages.AddToTail( msg );

    return true;
}
