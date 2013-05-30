
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
#ifndef CAREER_TASKS_H
#define CAREER_TASKS_H

#include "bot.h"
#include <list>

class CCareerTask 
{
protected:

    CCareerTask( const char *taskName, GameEventType event, const char *weaponName, int n, bool mustLive, bool crossRounds, int id, bool isComplete );

    static CCareerTask *NewTask( const char  * taskName, enum GameEventType event, const char  * weaponName, int n, bool mustLive, bool crossRounds, int id, bool isComplete );

    virtual void OnEvent( GameEventType event, class CBasePlayer * pAttacker, class CBasePlayer * pVictim );

    void OnWeaponKill( int weaponId, int weaponClassId, bool headshot, bool killerHasShield, class CBasePlayer * pAttacker, class CBasePlayer * pVictim );
    void OnWeaponInjury( int weaponId, int weaponClassId, bool attackerHasShield, class CBasePlayer * pAttacker );

    bool IsComplete( void );
    const char GetTaskName( void );

    int GetWeaponId( void );
    int GetWeaponClassId( void );

    virtual void Reset( void );

    bool IsValidFor( CBasePlayer *pPlayer );
    void SendPartialNotification( void );

    virtual bool IsTaskCompletableThisRound( void );

	bool                        m_isComplete;         /*     4     1 */

	const char  *               m_name;               /*     8     4 */
	int                         m_id;                 /*    12     4 */
	GameEventType               m_event;              /*    16     4 */
	int                         m_eventsNeeded;       /*    20     4 */
	int                         m_eventsSeen;         /*    24     4 */
	bool                        m_mustLive;           /*    28     1 */
	bool                        m_crossRounds;        /*    29     1 */
	bool                        m_diedThisRound;      /*    30     1 */

	int                         m_weaponId;           /*    32     4 */
	int                         m_weaponClassId;      /*    36     4 */
	bool                        m_rescuer;            /*    40     1 */
	bool                        m_defuser;            /*    41     1 */
	bool                        m_vip;                /*    42     1 */

	/* vtable has 3 entries: 
    {
	   [0] = OnEvent(_ZN11CCareerTask7OnEventE13GameEventTypeP11CBasePlayerS2_), 
	   [1] = Reset(_ZN11CCareerTask5ResetEv), 
	   [2] = IsTaskCompletableThisRound(_ZNK11CCareerTask26IsTaskCompletableThisRoundEv), 
	} */
	/* size: 44, cachelines: 1, members: 15 */
	/* sum members: 39, holes: 2, sum holes: 4 */
	/* padding: 1 */
	/* last cacheline: 44 bytes */
};

typedef std::list< CCareerTask* > CareerTaskList;

class CCareerTaskManager 
{
    public:

        void Create( void );
        void Reset( bool deleteTasks );
        void AddTask( const char *taskName, const char *weaponName, int eventCount, bool mustLive, bool crossRounds, bool isComplete );

        void HandleEvent( GameEventType event, class CBasePlayer *pAttacker, CBasePlayer *pVictim );
        void HandleEnemyKill( bool wasBlind, const char *weaponName, bool headshot, bool killerHasShield, CBasePlayer *pAttacker, CBasePlayer *pVictim );
        void HandleWeaponKill( int weaponId, int weaponClassId, bool headshot, bool killerHasShield, CBasePlayer *pAttacker, CBasePlayer *pVictim );
        void HandleDeath( int team, CBasePlayer *pAttacker );
        void HandleWeaponInjury( int weaponId, int weaponClassId, bool attackerHasShield, CBasePlayer *pAttacker );
        void HandleEnemyInjury( const char *weaponName, bool attackerHasShield, CBasePlayer *pAttacker );

        bool AreAllTasksComplete( void );
        int GetNumRemainingTasks( void );
        float GetRoundElapsedTime( void );

        int GetTaskTime( void );
        void SetFinishedTaskTime( int val );
        int GetFinishedTaskTime( void );
        int GetFinishedTaskRound( void );

        CareerTaskList *GetTasks( void );

        void LatchRoundEndMessage( void );
        void UnlatchRoundEndMessage( void );

    private:

        CareerTaskList              m_tasks;                        /*     0     0 */
        int                         m_nextId;                       /*     8     4 */
        float                       m_roundStartTime;               /*    12     4 */
        int                         m_taskTime;                     /*    16     4 */
        int                         m_finishedTaskTime;             /*    20     4 */
        int                         m_finishedTaskRound;            /*    24     4 */
        GameEventType               m_roundEndMessage;              /*    28     4 */
        bool                        m_shouldLatchRoundEndMessage;   /*    32     1 */

        /* size: 36, cachelines: 1, members: 8 */
        /* sum members: 25, holes: 1, sum holes: 8 */
        /* padding: 3 */
        /* last cacheline: 36 bytes */
};

typedef CCareerTask* ( *TaskFactoryFunction )( const char  *, enum GameEventType, const char  *, int, bool, bool, int, bool );

struct TaskInfo 
{
	const char                 *taskName;             /*     0     4 */
	GameEventType               event;                /*     4     4 */
	TaskFactoryFunction         factory;              /*     8     4 */
};

class CPreventDefuseTask : public CCareerTask 
{
protected:

	CPreventDefuseTask( const char *taskName, GameEventType event, const char *weaponName, int n, bool mustLive, bool crossRounds, int id, bool isComplete );

	static CCareerTask *NewTask( const char  * taskName, enum GameEventType event, const char  * weaponName, int n, bool mustLive, bool crossRounds, int id, bool isComplete );

	virtual void OnEvent( GameEventType event, class CBasePlayer * pAttacker, class CBasePlayer * pVictim );
	virtual void Reset( void );
	virtual bool IsTaskCompletableThisRound( void );

    bool                        m_bombPlantedThisRound;      /*    43     1 */
    bool                        m_defuseStartedThisRound;    /*    44     1 */

	/* vtable has 3 entries: 
    {
	   [0] = OnEvent
	   [1] = Reset
	   [2] = IsTaskCompletableThisRound
	} */
	/* size: 48, cachelines: 1, members: 3 */
	/* padding: 3 */
	/* paddings: 1, sum paddings: 1 */
	/* last cacheline: 48 bytes */
};

extern CCareerTaskManager *TheCareerTasks;

#endif // CAREER_TASKS_H