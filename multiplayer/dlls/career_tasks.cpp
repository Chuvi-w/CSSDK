
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
#include "cbase.h"
#include "player.h"
#include "career_tasks.h"
#include "enginecallback.h"

CCareerTaskManager *TheCareerTasks = NULL;

CCareerTask::CCareerTask( const char *taskName, GameEventType event, const char *weaponName, int n, bool mustLive, bool crossRounds, int id, bool isComplete )
{
    m_eventsSeen     = 0;
    m_name           = taskName;
    m_isComplete     = isComplete;
    m_event          = event;
    m_eventsNeeded   = n;
    m_mustLive       = mustLive;
    m_diedThisRound  = 0;
    m_crossRounds    = crossRounds;
    m_id             = id;
    //m_weaponId       = AliasToWeaponID( weaponName );
    //m_weaponClassId  = AliasToWeaponClass( weaponName );
    m_rescuer        = _strcmpi( taskName, "stoprescue" ) == 0;
    m_defuser        = _strcmpi( taskName, "killdefuser" ) == 0;
    m_vip            = _strcmpi( taskName, "killvip" ) == 0;

    if( event == EVENT_ALL_HOSTAGES_RESCUED )
    {
        m_mustLive    = 1;
        m_crossRounds = false;
    }

    if( m_isComplete )
    {
        MESSAGE_BEGIN( MSG_ALL, gmsgCZCareer );
            WRITE_STRING( "TASKDONE" );
            WRITE_BYTE( m_id );
        MESSAGE_END();
    }
}

CCareerTask *NewTask( const char *taskName, GameEventType event, const char *weaponName, int n, bool mustLive, bool crossRounds, int id, bool isComplete )
{
    CCareerTask* pTask = new CCareerTask( taskName, event, weaponName, n, mustLive, crossRounds, id, isComplete );

    return pTask;
}

void CCareerTask::OnEvent( GameEventType event, CBasePlayer *pAttacker, CBasePlayer *pVictim )
{
    if( !m_isComplete )
    {
        OnEvent( event, pAttacker, pVictim );
    }
}

void CCareerTask::OnWeaponKill( int weaponId, int weaponClassId, bool headshot, bool killerHasShield, CBasePlayer *pAttacker, CBasePlayer *pVictim )
{
    if( m_isComplete )
    {
        return;
    }

    if( m_event != EVENT_KILL || ( headshot && m_event != EVENT_HEADSHOT  ) )
    {
        return;
    }

    if( !pVictim || ( m_defuser && !pVictim->m_bIsDefusing ) || ( m_vip && !pVictim->m_bIsVIP ) )
    {
        return;
    }

    /*if( m_rescuer )
    {
        CHostage* pHostage = NULL;
        
        while( ( pHostage = ( CHostage* )UTIL_FindEntityByClassname( pEntity, "hostage_entity" ) ) != NULL )
        {
            if( !pHostage )
            {
                return;
            }

            if( pHostage->pev->takedamage )
            {

            }
        }    
    }
    else
    {

    }*/
    
    // ...
}

void CCareerTask::OnWeaponInjury( int weaponId, int weaponClassId, bool attackerHasShield, CBasePlayer *pAttacker )
{
    if( m_isComplete || m_event != EVENT_PLAYER_TOOK_DAMAGE  )
    {
        return;
    }
    
    if( m_weaponId != WEAPON_SHIELDGUN && ( m_weaponId != weaponId || m_weaponClassId != weaponClassId ) )
    {
        return;
    }
    
    if( attackerHasShield )
    {
        return;
    }

    ++m_eventsSeen;

    MESSAGE_BEGIN( MSG_ALL, gmsgCZCareer );
        WRITE_STRING( "TASKPART" );
        WRITE_BYTE( m_id );
        WRITE_SHORT( m_eventsSeen );
    MESSAGE_END();

    UTIL_LogPrintf( "Career Task Partial %d %d\n", m_id, m_eventsSeen );
}

void CCareerTask::Reset( void )
{
    m_eventsSeen = 0;
    m_isComplete = false;

    MESSAGE_BEGIN( MSG_ALL, gmsgCZCareer );
        WRITE_STRING( "TASKUNDONE" );
        WRITE_BYTE( m_id );
    MESSAGE_END();

    MESSAGE_BEGIN( MSG_ALL, gmsgCZCareer );
        WRITE_STRING( "TASKPART" );
        WRITE_BYTE( m_id );
        WRITE_SHORT( m_eventsSeen );
    MESSAGE_END();
}

bool CCareerTask::IsValidFor( CBasePlayer *pPlayer )
{
    return true;
}

void CCareerTask::SendPartialNotification( void )
{
    MESSAGE_BEGIN( MSG_ALL, gmsgCZCareer );
        WRITE_STRING( "TASKPART" );
        WRITE_BYTE( m_id );
        WRITE_SHORT( m_eventsSeen );
    MESSAGE_END();

    UTIL_LogPrintf( "Career Task Partial %d %d\n", m_id, m_eventsSeen );
}

bool CCareerTask::IsTaskCompletableThisRound( void )
{
    return true;
}



CCareerTaskManager::CCareerTaskManager()
{
    // ...
    m_finishedTaskTime = 0;

    Reset( true );
}

void CCareerTaskManager::Create( void )
{
    if( TheCareerTasks )
    {
        Reset( true );
        return;
    }
    
    TheCareerTasks = new CCareerTaskManager;
}

void CCareerTaskManager::HandleEvent( GameEventType event, CBasePlayer *pAttacker, CBasePlayer *pVictim )
{
    if( event == EVENT_ROUND_START )
    {
        m_roundStartTime = gpGlobals->time;
    }
    else if ( event <= EVENT_ROUND_LOSS && m_shouldLatchRoundEndMessage )
    {
        m_roundEndMessage = event;
    }
    else
    {
        for( std::list< CCareerTask* >::iterator iter = m_tasks.begin(); iter != m_tasks.end(); iter++ )
        {
            ( *iter )->OnEvent( event, pAttacker, pVictim );
        }
    }
}

void CCareerTaskManager::HandleEnemyKill( bool wasBlind, const char *weaponName, bool headshot, bool killerHasShield, CBasePlayer *pAttacker, CBasePlayer *pVictim )
{
    for( std::list< CCareerTask* >::iterator iter = m_tasks.begin(); iter != m_tasks.end(); iter++ )
    {
       // ...
    }
}

void CCareerTaskManager::HandleWeaponKill( int weaponId, int weaponClassId, bool headshot, bool killerHasShield, CBasePlayer *pAttacker, CBasePlayer *pVictim )
{
    for( std::list< CCareerTask* >::iterator iter = m_tasks.begin(); iter != m_tasks.end(); iter++ )
    {
        ( *iter )->OnWeaponKill( weaponId, weaponClassId, headshot, killerHasShield, pAttacker, pVictim );
    }
}

void CCareerTaskManager::HandleDeath( int team, CBasePlayer *pAttacker )
{
    // ...
}

void CCareerTaskManager::HandleWeaponInjury( int weaponId, int weaponClassId, bool attackerHasShield, CBasePlayer *pAttacker )
{
    for( std::list< CCareerTask* >::iterator iter = m_tasks.begin(); iter != m_tasks.end(); iter++ )
    {
        ( *iter )->OnWeaponInjury( weaponId, weaponClassId, attackerHasShield, pAttacker );
    }
}

void CCareerTaskManager::HandleEnemyInjury( const char *weaponName, bool attackerHasShield, CBasePlayer *pAttacker )
{
    for( std::list< CCareerTask* >::iterator iter = m_tasks.begin(); iter != m_tasks.end(); iter++ )
    {
        //( *iter )->OnWeaponInjury( AliasToWeaponID( weaponId ), AliasToWeaponClass( weaponClassId ), attackerHasShield, pAttacker );
        //( *iter )->OnEvent( EVENT_PLAYER_TOOK_DAMAGE, NULL, NULL );
    }
}

bool CCareerTaskManager::AreAllTasksComplete()
{
    for( std::list< CCareerTask* >::iterator iter = m_tasks.begin(); iter != m_tasks.end(); iter++ )
    {
        if( !( *iter )->IsComplete() )
        {
            return false;
        }
    }

    return true;
}

int CCareerTaskManager::GetNumRemainingTasks( void )
{
    int count = 0;

    for( std::list< CCareerTask* >::iterator iter = m_tasks.begin(); iter != m_tasks.end(); iter++ )
    {
        count += !( *iter )->IsComplete();
    }

    return count;
}

void CCareerTaskManager::LatchRoundEndMessage( void ) 
{
    m_shouldLatchRoundEndMessage = true; 
}

void CCareerTaskManager::UnlatchRoundEndMessage( void ) 
{ 
    m_shouldLatchRoundEndMessage = false; 

    if( m_roundEndMessage == EVENT_ROUND_START )
    {
        m_roundStartTime = gpGlobals->time;
    }

    for( std::list< CCareerTask* >::iterator iter = m_tasks.begin(); iter != m_tasks.end(); iter++ )
    {
        ( *iter )->OnEvent( m_roundEndMessage, NULL, NULL );
    }
};

void CCareerTaskManager::Reset( bool deleteTasks )
{
    std::list< CCareerTask* >::iterator iter = m_tasks.begin();

    CCareerTaskManager *pTask = ( CCareerTaskManager* )( *iter );

    if( deleteTasks )
    { 
        if( pTask != this )
        {
            for( ; pTask != this; iter++ )
            {
                delete ( *iter );
            }
        }
        else
        {
            // ... 
            m_nextId = 0;
        }
    }
    else
    {
        for( ; pTask != this; iter++ )
        {
            // ...
        }
    }

    m_finishedTaskTime  = 0;
    m_finishedTaskRound = 0;
    m_shouldLatchRoundEndMessage = 0;

    m_roundStartTime = gpGlobals->time + CVAR_GET_FLOAT( "mp_freezetime" );
}


CCareerTask *CPreventDefuseTask::NewTask( const char *taskName, GameEventType event, const char *weaponName, int n, bool mustLive, bool crossRounds, int id, bool isComplete )
{
    CCareerTask *pNewTask = new CCareerTask( taskName, event, weaponName, n, mustLive, crossRounds, id, isComplete );

    CPreventDefuseTask *pTask = ( CPreventDefuseTask* )pNewTask;

    pTask->m_bombPlantedThisRound   = false;
    pTask->m_defuseStartedThisRound = false;

    return pNewTask;
}

void CPreventDefuseTask::OnEvent( GameEventType event, CBasePlayer *pAttacker, CBasePlayer *pVictim )
{
    if( !IsComplete() )
    {
        if( event == EVENT_BOMB_PLANTED )
        {
            m_bombPlantedThisRound = true;
        }
        else if( event == EVENT_BOMB_DEFUSING )
        {
            m_defuseStartedThisRound = false;
        }
        else if( event <= EVENT_ROUND_LOSS )
        {
            m_bombPlantedThisRound   = false;
            m_defuseStartedThisRound = false;
        }

        CCareerTask::OnEvent( event, pAttacker, pVictim );
    }
}

void CPreventDefuseTask::Reset( void )
{
    m_bombPlantedThisRound   = false;
    m_defuseStartedThisRound = false;

    CCareerTask::Reset();
}

bool CPreventDefuseTask::IsTaskCompletableThisRound( void ) 
{
    return m_bombPlantedThisRound && !m_defuseStartedThisRound; 
}