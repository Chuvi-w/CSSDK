//=========== (C) Copyright 1999 Valve, L.L.C. All rights reserved. ===========
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Purpose: Functionality for the observer chase camera
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "pm_shared.h"
#include "game.h"

extern int gmsgCurWeapon;
extern int gmsgSetFOV;
extern int gmsgStatusIcon;
extern int gmsgSpecHealth2;

#define FORCECAMERA_SPECTATE_ANYONE    0
#define FORCECAMERA_SPECTATE_ONLY_TEAM 1
#define FORCECAMERA_ONLY_FRIST_PERSON  2

void UpdateClientEffects(CBasePlayer *pObserver, int oldMode);

int GetForceCamera(void)
{
	int retVal;

	if (!fadetoblack.value)
	{
		retVal = (int)CVAR_GET_FLOAT("mp_forcechasecam");

		if (retVal == FORCECAMERA_SPECTATE_ANYONE)
		{
			retVal = (int)CVAR_GET_FLOAT("mp_forcecamera");
		}
	}
	else
	{
		retVal = FORCECAMERA_ONLY_FRIST_PERSON;
	}

	return retVal;
}

CBaseEntity *CBasePlayer::Observer_IsValidTarget(int iPlayerIndex, bool bSameTeam)   // Last check: 2013, November 18.
{
	if (iPlayerIndex > gpGlobals->maxClients || iPlayerIndex < 1)
	{
		return NULL;
	}

	CBasePlayer *pPlayer = (CBasePlayer *)UTIL_PlayerByIndex(iPlayerIndex);

	if (!pPlayer || pPlayer == this || pPlayer->has_disconnected || pPlayer->IsObserver() || FBitSet(pPlayer->pev->effects, EF_NODRAW) || pPlayer->m_iTeam == UNASSIGNED)
	{
		return NULL;
	}

	if (bSameTeam && pPlayer->m_iTeam != m_iTeam)
	{
		return NULL;
	}

	return pPlayer;
}

void CBasePlayer::Observer_FindNextPlayer(bool bReverse, const char *name)   // Last check: 2013, November 18.
{
	if (m_flNextFollowTime && gpGlobals->time < m_flNextFollowTime)
	{
		return;
	}

	m_flNextFollowTime = gpGlobals->time + 0.25;

	int iStart = m_hObserverTarget ? m_hObserverTarget->entindex() : entindex();
	int iCurrent = iStart;

	m_hObserverTarget = NULL;

	int iDir = bReverse ? -1 : 1;
	bool bForceSameTeam = (GetForceCamera() != FORCECAMERA_SPECTATE_ANYONE && m_iTeam != SPECTATOR);

	do
	{
		iCurrent += iDir;

		if (iCurrent > gpGlobals->maxClients)
		{
			iCurrent = 1;
		}
		else if (iCurrent < 1)
		{
			iCurrent = gpGlobals->maxClients;
		}

		m_hObserverTarget = Observer_IsValidTarget(iCurrent, bForceSameTeam);

		if (m_hObserverTarget)
		{
			if (!name)
			{
				break;
			}

			CBasePlayer *pPlayer = (CBasePlayer *)UTIL_PlayerByIndex(m_hObserverTarget->entindex());

			if (!strcmp(name, STRING(pPlayer->pev->netname)))
			{
				break;
			}
		}
	} while (iCurrent != iStart);

	if (m_hObserverTarget)
	{
		UTIL_SetOrigin(pev, m_hObserverTarget->pev->origin);

		MESSAGE_BEGIN(MSG_ONE, gmsgSpecHealth2, NULL, pev);
			WRITE_BYTE(min(0, m_hObserverTarget->pev->health));
			WRITE_BYTE(m_hObserverTarget->entindex());
		MESSAGE_END();

		if (pev->iuser1 != OBS_ROAMING)
		{
			pev->iuser2 = m_hObserverTarget->entindex();
		}

		UpdateClientEffects(this, pev->iuser1);
	}
}

void CBasePlayer::Observer_HandleButtons(void)   // Last check: 2013, November 18.
{
	if (m_flNextObserverInput > gpGlobals->time)
	{
		return;
	}

	if (m_afButtonPressed & IN_JUMP)
	{
		switch (pev->iuser1)
		{
			case OBS_CHASE_LOCKED:
				Observer_SetMode(OBS_CHASE_FREE);
				break;
			case OBS_CHASE_FREE:
				Observer_SetMode(OBS_IN_EYE);
				break;
			case OBS_IN_EYE:
				Observer_SetMode(OBS_ROAMING);
				break;
			case OBS_ROAMING:
				Observer_SetMode(OBS_MAP_FREE);
				break;
			case OBS_MAP_FREE:
				Observer_SetMode(OBS_MAP_CHASE);
				break;
			default:
				Observer_SetMode(m_bObserverAutoDirector? OBS_CHASE_LOCKED : OBS_CHASE_FREE);
				break;
		}

		m_flNextObserverInput = gpGlobals->time + 0.2;
	}

	if (m_afButtonPressed & IN_ATTACK)
	{
		Observer_FindNextPlayer(false);
		m_flNextObserverInput = gpGlobals->time + 0.2;
	}

	if (m_afButtonPressed & IN_ATTACK2)
	{
		Observer_FindNextPlayer(true);
		m_flNextObserverInput = gpGlobals->time + 0.2;
	}
}

void CBasePlayer::Observer_CheckTarget()  // Last check: 2013, November 18.
{
	if (pev->iuser1 == OBS_ROAMING && !m_bWasFollowing)
	{
		return;
	}

	if (m_bWasFollowing)
	{
		Observer_FindNextPlayer();

		if (m_hObserverTarget && m_iObserverLastMode != pev->iuser1)
		{
			Observer_SetMode(m_iObserverLastMode);
		}

		return;
	}

	if (!m_hObserverTarget)
	{
		Observer_FindNextPlayer();
	}

	if (m_hObserverTarget)
	{
		CBasePlayer *pTarget = (CBasePlayer *)UTIL_PlayerByIndex(m_hObserverTarget->entindex());

		if (!pTarget || pTarget->pev->deadflag == DEAD_RESPAWNABLE || FBitSet(pTarget->pev->effects, EF_NODRAW))
		{
			Observer_FindNextPlayer();
		}
		else
		{
			if (pTarget->pev->deadflag == DEAD_DEAD && gpGlobals->time > pTarget->m_fDeadTime + 2)
			{
				Observer_FindNextPlayer();

				if (!m_hObserverTarget)
				{
					int lastMode = pev->iuser1;

					if (lastMode != OBS_ROAMING)
					{
						Observer_SetMode(OBS_ROAMING);
					}

					m_iObserverLastMode = lastMode;
					m_bWasFollowing     = true;
				}
			}
		}
	}
	else
	{
		int lastMode = pev->iuser1;

		if (lastMode != OBS_ROAMING)
		{
			Observer_SetMode(OBS_ROAMING);
		}

		m_iObserverLastMode = lastMode;
	}
}

void CBasePlayer::Observer_CheckProperties(void)  // Last check: 2013, November 18.
{
	if (pev->iuser1 == OBS_IN_EYE && m_hObserverTarget != NULL)
	{
		CBasePlayer *pTarget = (CBasePlayer *)UTIL_PlayerByIndex(m_hObserverTarget->entindex());

		if (!pTarget)
		{
			return;
		}

		int weapon = pTarget->m_pActiveItem ? pTarget->m_pActiveItem->m_iId : 0;

		if (m_iFOV != pTarget->m_iFOV || m_iObserverWeapon != weapon)
		{
			m_iClientFOV = m_iFOV = pTarget->m_iFOV;

			MESSAGE_BEGIN(MSG_ONE, gmsgSetFOV, NULL, pev);
				WRITE_BYTE(m_iFOV);
			MESSAGE_END();

			m_iObserverWeapon = weapon;

			MESSAGE_BEGIN(MSG_ONE, gmsgCurWeapon, NULL, pev);
				WRITE_BYTE(1);
				WRITE_BYTE(m_iObserverWeapon);
				WRITE_BYTE(0);
			MESSAGE_END();
		}

		int targetBombState = STATUSICON_HIDE;

		if (pTarget->m_bHasC4)
		{
			targetBombState = (pTarget->m_signals.GetState() & SIGNAL_BOMB) ? STATUSICON_FLASH : STATUSICON_SHOW;
		}

		if (m_iObserverC4State != targetBombState)
		{
			m_iObserverC4State = targetBombState;

			if (targetBombState)
			{
				MESSAGE_BEGIN(MSG_ONE, gmsgStatusIcon, NULL, pev);
					WRITE_BYTE(m_iObserverC4State);
					WRITE_STRING("c4");
					WRITE_BYTE(0);
					WRITE_BYTE(160);
					WRITE_BYTE(0);
				MESSAGE_END();
			}
			else
			{
				MESSAGE_BEGIN(MSG_ONE, gmsgStatusIcon, NULL, pev);
					WRITE_BYTE(STATUSICON_HIDE);
					WRITE_STRING("c4");
				MESSAGE_END();
			}
		}

		if (m_bObserverHasDefuser != pTarget->m_bHasDefuser)
		{
			m_bObserverHasDefuser = pTarget->m_bHasDefuser;

			if (pTarget->m_bHasDefuser)
			{
				MESSAGE_BEGIN(MSG_ONE, gmsgStatusIcon, NULL, pev);
					WRITE_BYTE(STATUSICON_SHOW);
					WRITE_STRING("defuser");
					WRITE_BYTE(0);
					WRITE_BYTE(160);
					WRITE_BYTE(0);
				MESSAGE_END();
			}
			else
			{
				MESSAGE_BEGIN(MSG_ONE, gmsgStatusIcon, NULL, pev);
					WRITE_BYTE(STATUSICON_HIDE);
					WRITE_STRING("defuser");
				MESSAGE_END();
			}
		}

		return;
	}

	m_iFOV = 90;

	if (m_iObserverWeapon)
	{
		m_iObserverWeapon = 0;

		MESSAGE_BEGIN(MSG_ONE, gmsgCurWeapon, NULL, pev);
			WRITE_BYTE(1);
			WRITE_BYTE(m_iObserverWeapon);
			WRITE_BYTE(0);
		MESSAGE_END();
	}

	if (m_iObserverC4State)
	{
		m_iObserverC4State = 0;

		MESSAGE_BEGIN(MSG_ONE, gmsgStatusIcon, NULL, pev);
			WRITE_BYTE(0);
			WRITE_STRING("c4");
		MESSAGE_END();
	}

	if (m_bObserverHasDefuser)
	{
		m_bObserverHasDefuser = false;

		MESSAGE_BEGIN(MSG_ONE, gmsgStatusIcon, NULL, pev);
			WRITE_BYTE(0);
			WRITE_STRING("defuser");
		MESSAGE_END();
	}
}

// Attempt to change the observer mode
void CBasePlayer::Observer_SetMode(int iMode)
{
	// Just abort if we're changing to the mode we're already in
	if (iMode == pev->iuser1)
		return;

	// is valid mode ?
	if (iMode < OBS_CHASE_LOCKED || iMode > OBS_MAP_CHASE)
		iMode = OBS_IN_EYE; // now it is
	// verify observer target again
	if (m_hObserverTarget != NULL)
	{
		CBaseEntity *pEnt = m_hObserverTarget;

		if ((pEnt == this) || (pEnt == NULL))
			m_hObserverTarget = NULL;
		else if ( /*((CBasePlayer*)pEnt)->IsObserver()|| */ (pEnt->pev->effects & EF_NODRAW))
			m_hObserverTarget = NULL;
	}

	// set spectator mode
	pev->iuser1 = iMode;

	// if we are not roaming, we need a valid target to track
	if ((iMode != OBS_ROAMING) && (m_hObserverTarget == NULL))
	{
		Observer_FindNextPlayer(false, NULL);

		// if we didn't find a valid target switch to roaming
		if (m_hObserverTarget == NULL)
		{
			ClientPrint(pev, HUD_PRINTCENTER, "#Spec_NoTarget");
			pev->iuser1 = OBS_ROAMING;
		}
	}

	// set target if not roaming
	if (pev->iuser1 == OBS_ROAMING)
	{
		pev->iuser2 = 0;
	}
	else
		pev->iuser2 = ENTINDEX(m_hObserverTarget->edict());

	pev->iuser3 = 0;	// clear second target from death cam

	// print spepctaor mode on client screen

	char modemsg[16];
	sprintf(modemsg, "#Spec_Mode%i", pev->iuser1);
	ClientPrint(pev, HUD_PRINTCENTER, modemsg);

	m_iObserverLastMode = iMode;
}