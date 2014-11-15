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

#include "GameEvent.h"
#include <list>

class CCareerTask
{
public:

	CCareerTask(const char *taskName, GameEventType event, const char *weaponName, int n, bool mustLive, bool crossRounds, int id, bool isComplete);

public:

	static CCareerTask *NewTask(const char *taskName, GameEventType event, const char *weaponName, int n, bool mustLive, bool crossRounds, int id, bool isComplete);

	virtual void OnEvent(GameEventType event, CBasePlayer *pAttacker, CBasePlayer *pVictim);

	void OnWeaponKill(int weaponId, int weaponClassId, bool headshot, bool killerHasShield, CBasePlayer *pAttacker, CBasePlayer *pVictim);
	void OnWeaponInjury(int weaponId, int weaponClassId, bool attackerHasShield, CBasePlayer *pAttacker);

	bool IsComplete(void) { return m_isComplete; }
	const char *GetTaskName(void) { return m_name; }

	int GetWeaponId(void) { return m_weaponId; }
	int GetWeaponClassId(void) { return m_weaponClassId; }

	virtual void Reset(void);

	bool IsValidFor(CBasePlayer *pPlayer);
	void SendPartialNotification(void);

	virtual bool IsTaskCompletableThisRound(void);

private:

	bool                        m_isComplete;         /*     4     1 */
	const char                 *m_name;               /*     8     4 */
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
	[0] = OnEvent
	[1] = Reset
	[2] = IsTaskCompletableThisRound
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

	CCareerTaskManager(void);

public:

	void Create(void);
	void Reset(bool deleteTasks);
	void AddTask(const char *taskName, const char *weaponName, int eventCount, bool mustLive, bool crossRounds, bool isComplete);

	void HandleEvent(GameEventType event, class CBasePlayer *pAttacker, CBasePlayer *pVictim);
	void HandleEnemyKill(bool wasBlind, const char *weaponName, bool headshot, bool killerHasShield, CBasePlayer *pAttacker, CBasePlayer *pVictim);
	void HandleWeaponKill(int weaponId, int weaponClassId, bool headshot, bool killerHasShield, CBasePlayer *pAttacker, CBasePlayer *pVictim);
	void HandleDeath(int team, CBasePlayer *pAttacker);
	void HandleWeaponInjury(int weaponId, int weaponClassId, bool attackerHasShield, CBasePlayer *pAttacker);
	void HandleEnemyInjury(const char *weaponName, bool attackerHasShield, CBasePlayer *pAttacker);

	bool AreAllTasksComplete(void);
	int GetNumRemainingTasks(void);
	float GetRoundElapsedTime(void) { return gpGlobals->time - m_taskTime; }

	int GetTaskTime(void) { return m_taskTime; }
	void SetFinishedTaskTime(int val) { m_finishedTaskTime = val;/* m_finishedTaskRound = g_pGameRules->m_iTotalRoundsPlayed;*/ }
	int GetFinishedTaskTime(void) { return m_finishedTaskTime; }
	int GetFinishedTaskRound(void) { return m_finishedTaskRound; }

	CareerTaskList *GetTasks(void) { return &m_tasks; }

	void LatchRoundEndMessage(void);
	void UnlatchRoundEndMessage(void);

private:

	CareerTaskList              m_tasks;                        /*     0     0 */
	int                         m_nextId;                       /*     8     4 */
	float                       m_roundStartTime;               /*    12     4 */
	int                         m_taskTime;                     /*    16     4 */
	int                         m_finishedTaskTime;             /*    20     4 */
	int                         m_finishedTaskRound;            /*    24     4 */
	GameEventType               m_roundEndMessage;              /*    28     4 */
	bool                        m_shouldLatchRoundEndMessage;   /*    32     1 */

	/* size: 36, cachelines: 1, members: 8      */
	/* sum members: 25, holes: 1, sum holes: 8  */
	/* padding: 3                               */
	/* last cacheline: 36 bytes                 */
};

typedef CCareerTask* (*TaskFactoryFunction)(const char*, GameEventType, const char*, int, bool, bool, int, bool);

struct TaskInfo
{
	const char                 *taskName;             /*     0     4 */
	GameEventType               event;                /*     4     4 */
	TaskFactoryFunction         factory;              /*     8     4 */
};

class CPreventDefuseTask : public CCareerTask
{
public:

	CPreventDefuseTask(const char *taskName, GameEventType event, const char *weaponName, int n, bool mustLive, bool crossRounds, int id, bool isComplete);

public:

	static CCareerTask *NewTask(const char *taskName, GameEventType event, const char *weaponName, int n, bool mustLive, bool crossRounds, int id, bool isComplete);

	virtual void OnEvent(GameEventType event, CBasePlayer *pAttacker, CBasePlayer *pVictim);
	virtual void Reset(void);
	virtual bool IsTaskCompletableThisRound(void);

protected:

	bool                        m_bombPlantedThisRound;      /*    43     1 */
	bool                        m_defuseStartedThisRound;    /*    44     1 */

	/* vtable has 3 entries:
	{
	[0] = OnEvent
	[1] = Reset
	[2] = IsTaskCompletableThisRound
	} */
	/* size: 48, cachelines: 1, members: 3 */
	/* padding: 3                          */
	/* paddings: 1, sum paddings: 1        */
	/* last cacheline: 48 bytes            */
};

extern CCareerTaskManager *TheCareerTasks;

#endif // CAREER_TASKS_H