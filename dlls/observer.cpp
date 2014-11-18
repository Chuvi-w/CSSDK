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
#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"player.h"
#include	"weapons.h"
#include	"pm_shared.h"

extern int gmsgCurWeapon;
extern int gmsgSetFOV;
extern int gmsgStatusIcon;

// Find the next client in the game for this player to spectate
void CBasePlayer::Observer_FindNextPlayer(bool bReverse, const char *name)
{
	// MOD AUTHORS: Modify the logic of this function if you want to restrict the observer to watching
	//				only a subset of the players. e.g. Make it check the target's team.

	int		iStart;
	if (m_hObserverTarget)
		iStart = ENTINDEX(m_hObserverTarget->edict());
	else
		iStart = ENTINDEX(edict());
	int	    iCurrent = iStart;
	m_hObserverTarget = NULL;
	int iDir = bReverse ? -1 : 1;

	do
	{
		iCurrent += iDir;

		// Loop through the clients
		if (iCurrent > gpGlobals->maxClients)
			iCurrent = 1;
		if (iCurrent < 1)
			iCurrent = gpGlobals->maxClients;

		CBaseEntity *pEnt = UTIL_PlayerByIndex(iCurrent);
		if (!pEnt)
			continue;
		if (pEnt == this)
			continue;
		// Don't spec observers or players who haven't picked a class yet
		//if ( ((CBasePlayer*)pEnt)->IsObserver() || (pEnt->pev->effects & EF_NODRAW) )
		//	continue;

		// MOD AUTHORS: Add checks on target here.

		m_hObserverTarget = pEnt;
		break;
	} while (iCurrent != iStart);

	// Did we find a target?
	if (m_hObserverTarget)
	{
		// Move to the target
		UTIL_SetOrigin(pev, m_hObserverTarget->pev->origin);

		// ALERT( at_console, "Now Tracking %s\n", STRING( m_hObserverTarget->pev->netname ) );

		// Store the target in pev so the physics DLL can get to it
		if (pev->iuser1 != OBS_ROAMING)
			pev->iuser2 = ENTINDEX(m_hObserverTarget->edict());
	}
}

// Handle buttons in observer mode
void CBasePlayer::Observer_HandleButtons()
{
	// Slow down mouse clicks
	if (m_flNextObserverInput > gpGlobals->time)
		return;

	// Jump changes from modes: Chase to Roaming
	if (m_afButtonPressed & IN_JUMP)
	{
		if (pev->iuser1 == OBS_CHASE_LOCKED)
			Observer_SetMode(OBS_CHASE_FREE);

		else if (pev->iuser1 == OBS_CHASE_FREE)
			Observer_SetMode(OBS_IN_EYE);

		else if (pev->iuser1 == OBS_IN_EYE)
			Observer_SetMode(OBS_ROAMING);

		else if (pev->iuser1 == OBS_ROAMING)
			Observer_SetMode(OBS_MAP_FREE);

		else if (pev->iuser1 == OBS_MAP_FREE)
			Observer_SetMode(OBS_MAP_CHASE);

		else
			Observer_SetMode(OBS_CHASE_FREE);	// don't use OBS_CHASE_LOCKED anymore

		m_flNextObserverInput = gpGlobals->time + 0.2;
	}

	// Attack moves to the next player
	if (m_afButtonPressed & IN_ATTACK)//&& pev->iuser1 != OBS_ROAMING )
	{
		Observer_FindNextPlayer(false, NULL);

		m_flNextObserverInput = gpGlobals->time + 0.2;
	}

	// Attack2 moves to the prev player
	if (m_afButtonPressed & IN_ATTACK2)// && pev->iuser1 != OBS_ROAMING )
	{
		Observer_FindNextPlayer(true, NULL);

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