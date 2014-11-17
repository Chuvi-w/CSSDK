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
#include "weapons.h"
#include "gamerules.h"
#include "training_gamerules.h"
#include "game.h"
#include "GameEvent.h"

extern DLL_GLOBAL BOOL g_fGameOver;
extern int gmsgStatusIcon;
extern int gmsgBlinkAcct;

CHalfLifeTraining::CHalfLifeTraining(void) // Last check: 2013, September 16.
{
	PRECACHE_MODEL("models/w_weaponbox.mdl");
}

void CHalfLifeTraining::HostageDied(void) // Last check: 2013, September 16.
{
	CBasePlayer *pPlayer = (CBasePlayer *)UTIL_PlayerByIndex(1);

	if (pPlayer)
	{
		pPlayer->pev->radsuit_finished = gpGlobals->time + 3;
	}
}

bool CHalfLifeTraining::PlayerCanBuy(CBasePlayer *pPlayer) // Last check: 2013, September 16.
{
	return pPlayer->m_signals.GetState() & SIGNAL_BUY;
}
 
void CHalfLifeTraining::InitHUD(CBasePlayer *pl) // Last check: 2013, September 16.
{
	/* empty */
}

BOOL CHalfLifeTraining::IsMultiplayer(void) // Last check: 2013, September 16.
{
	return FALSE;
}

BOOL CHalfLifeTraining::IsDeathmatch(void) // Last check: 2013, September 16.
{
	return FALSE;
}

edict_t *CHalfLifeTraining::GetPlayerSpawnSpot(CBasePlayer *pPlayer) // Last check: 2013, September 16.
{
	CBaseEntity *pSpawnSpot = UTIL_FindEntityByClassname(NULL, "info_player_start");

	if (!pSpawnSpot)
	{
		ALERT(at_error, "PutClientInServer: no info_player_start on level");
		return INDEXENT(0);
	}

	edict_t *pentSpawnSpot = pSpawnSpot->edict();

	pPlayer->pev->origin     = VARS(pentSpawnSpot)->origin + Vector(0, 0, 1);
	pPlayer->pev->v_angle    = g_vecZero;
	pPlayer->pev->velocity   = g_vecZero;
	pPlayer->pev->angles     = VARS(pentSpawnSpot)->angles;
	pPlayer->pev->punchangle = g_vecZero;
	pPlayer->pev->fixangle   = TRUE;

	return pentSpawnSpot;
}

int CHalfLifeTraining::ItemShouldRespawn(CItem *pItem) // Last check: 2013, September 16.
{
	return GR_ITEM_RESPAWN_NO;
}

BOOL CHalfLifeTraining::FPlayerCanRespawn(CBasePlayer *pPlayer) // Last check: 2013, September 16.
{
	return TRUE;
}
 
void CHalfLifeTraining::PlayerThink(CBasePlayer *pPlayer) // Last check: 2013, September 16.
{
	if (pPlayer->pev->radsuit_finished != 0 && gpGlobals->time > pPlayer->pev->radsuit_finished)
	{
		SERVER_COMMAND("reload\n");
	}

	if (!pPlayer->m_iAccount)
	{
		if (pPlayer->pev->scale)
		{
			pPlayer->m_iAccount = (int)pPlayer->pev->scale;
		}
	}

	if (pPlayer->m_iTeam == UNASSIGNED)
	{
		pPlayer->SetProgressBarTime(0);
		pPlayer->m_bHasDefuser = pPlayer->pev->ideal_yaw != 0;
	}

	m_iHostagesRescued = 0;
	m_bFreezePeriod    = FALSE;
	m_iRoundTimeSecs   = gpGlobals->time + 1;

	g_fGameOver = FALSE;

	pPlayer->m_iTeam         = CT;
	pPlayer->m_bCanShoot     = true;
	pPlayer->m_fLastMovement = gpGlobals->time;

	if (pPlayer->m_pActiveItem)
	{
		pPlayer->m_iHideHUD &= ~HIDEHUD_WEAPONS;
	}
	else
	{
		pPlayer->m_iHideHUD |= HIDEHUD_WEAPONS;
	}

	if (pPlayer->HasNamedPlayerItem("weapon_c4"))
	{
		if (pPlayer->m_rgAmmo[pPlayer->GetAmmoIndex("C4")] <= 0)
		{
			pPlayer->m_bHasC4 = false;

			if (FClassnameIs(pPlayer->m_pActiveItem->pev, "weapon_c4"))
			{
				pPlayer->pev->weapons &= ~(1 << pPlayer->m_pActiveItem->m_iId);
				pPlayer->RemovePlayerItem(pPlayer->m_pActiveItem);
				pPlayer->m_pActiveItem->Drop();
			}
		}
		else
		{
			pPlayer->m_bHasC4 = true;
		}
	}

	if (!pPlayer->m_bVGUIMenus && fVGUIMenus)
	{
		pPlayer->m_bVGUIMenus = true;
	}

	CGrenade *pGrenade = NULL;

	while ((pGrenade = (CGrenade *)UTIL_FindEntityByClassname(pGrenade, "grenade")) != NULL)
	{
		if (pGrenade->m_pentCurBombTarget != NULL)
		{
			pGrenade->m_bStartDefuse = true;
		}
	}

	if (pPlayer->m_signals.GetState() & SIGNAL_BUY)
	{
		if (!fInBuyArea)
		{
			FillAccountTime = 1;

			if (!fVisitedBuyArea)
			{
				MESSAGE_BEGIN(MSG_ONE, gmsgStatusIcon, NULL, pPlayer->pev);
					WRITE_BYTE(STATUSICON_FLASH);
					WRITE_STRING("buyzone");
					WRITE_BYTE(0);
					WRITE_BYTE(160);
					WRITE_BYTE(0);
				MESSAGE_END();
			}
		}

		fInBuyArea = TRUE;

		if (pPlayer->m_iAccount < 16000 && !FillAccountTime)
		{
			FillAccountTime = gpGlobals->time + 5;
		}

		if (FillAccountTime != 0 && gpGlobals->time > FillAccountTime)
		{
			if (!fVisitedBuyArea)
			{
				MESSAGE_BEGIN(MSG_ONE, gmsgBlinkAcct, NULL, pPlayer->pev);
					WRITE_BYTE(3);
				MESSAGE_END();

				MESSAGE_BEGIN(MSG_ONE, gmsgStatusIcon, NULL, pPlayer->pev);
					WRITE_BYTE(STATUSICON_SHOW);
					WRITE_STRING("buyzone");
					WRITE_BYTE(0);
					WRITE_BYTE(160);
					WRITE_BYTE(0);
				MESSAGE_END();

				fVisitedBuyArea = TRUE;
			}

			pPlayer->AddAccount(16000 - pPlayer->m_iAccount, true);
			FillAccountTime = 0;
		}
	}
	else
	{
		if (fInBuyArea && fVisitedBuyArea)
		{
			fInBuyArea = FALSE;
		}
	}

	pPlayer->pev->scale     = pPlayer->m_iAccount;
	pPlayer->pev->ideal_yaw = pPlayer->m_bHasDefuser;

	// TODO: Implement me.
	// TheBots->OnEvent(EVENT_PLAYER_CHANGED_TEAM, pPlayer, NULL);
}

void CHalfLifeTraining::PlayerSpawn(CBasePlayer *pPlayer) // Last check: 2013, September 16.
{
	if (pPlayer->m_bNotKilled)
	{
		return;
	}

	fInBuyArea      = TRUE;
	fVisitedBuyArea = FALSE;
	fVGUIMenus      = pPlayer->m_bVGUIMenus;
	FillAccountTime = 0;

	pPlayer->m_iJoiningState = JOINED;
	pPlayer->m_iTeam         = CT;
	pPlayer->m_bNotKilled    = true;
	pPlayer->pev->body       = 0;
	pPlayer->m_iModelName    = MODEL_URBAN;

	SET_MODEL(pPlayer->edict(), "models/player.mdl");

	CBaseEntity *pWeaponEntity = NULL;

	while ((pWeaponEntity = UTIL_FindEntityByClassname(pWeaponEntity, "game_player_equip")) != NULL)
	{
		pWeaponEntity->Touch(pPlayer);
	}

	pPlayer->SetPlayerModel(FALSE);
	pPlayer->Spawn();
	pPlayer->m_iHideHUD |= (HIDEHUD_WEAPONS | HIDEHUD_HEALTH | HIDEHUD_TIMER | HIDEHUD_MONEY);
}

void CHalfLifeTraining::PlayerKilled(CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor) // Last check: 2013, September 16.
{
	SET_VIEW(pVictim->edict(), pVictim->edict());
	FireTargets("game_playerdie", pVictim, pVictim, USE_TOGGLE, 0);
}

void CHalfLifeTraining::CheckMapConditions(void) // Last check: 2013, September 16.
{
	/* empty */
}

void CHalfLifeTraining::CheckWinConditions(void) // Last check: 2013, September 16.
{
	CGrenade *pBomb;

	if (m_bBombDefused)
	{
		pBomb = NULL;

		while ((pBomb = (CGrenade *)UTIL_FindEntityByClassname(pBomb, "grenade")) != NULL)
		{
			if (!pBomb->m_bIsC4 || !pBomb->m_bJustBlew)
			{
				continue;
			}

			pBomb->m_bJustBlew = false;
			m_bBombDefused     = false;

			FireTargets(STRING(pBomb->pev->target), CBaseEntity::Instance(pBomb->pev->owner), CBaseEntity::Instance(pBomb->pev->owner), USE_TOGGLE, 0);
			break;
		}
	}
	else if (m_bTargetBombed)
	{
		pBomb = NULL;

		while ((pBomb = (CGrenade *)UTIL_FindEntityByClassname(pBomb, "grenade")) != NULL)
		{
			if (!pBomb->m_bIsC4 || !pBomb->m_bJustBlew)
			{
				continue;
			}

			if (FStringNull(pBomb->pev->noise1))
			{
				continue;
			}

			pBomb->m_bJustBlew = false;
			m_bTargetBombed    = false;

			FireTargets(STRING(pBomb->pev->noise1), CBaseEntity::Instance(pBomb->pev->owner), CBaseEntity::Instance(pBomb->pev->owner), USE_TOGGLE, 0);
			break;
		}
	}

	// TODO: Fix me, CHostage here.
	edict_t *pentHostage = FIND_ENTITY_BY_CLASSNAME(NULL, "hostage_entity");

	if (!FNullEnt(pentHostage))
	{
		CBaseEntity *pHostage = CBaseEntity::Instance(pentHostage);

		if (pHostage)
		{
			while (pHostage)
			{
				if (pHostage->pev->deadflag = DEAD_RESPAWNABLE && !FStringNull(pHostage->pev->noise1))
				{
					break;
				}

				pHostage = UTIL_FindEntityByClassname(pHostage, "hostage_entity");
				
				if (!pHostage)
				{
					return;
				}
			}

			UTIL_SetSize(pHostage->pev, Vector(-16, -16, 0), Vector(16, 16, 72));

			edict_t *pentRescueArea = FIND_ENTITY_BY_CLASSNAME(NULL, "func_hostage_rescue");

			if (!FNullEnt(pentRescueArea))
			{
				CBaseEntity *pRescueArea = CBaseEntity::Instance(pentRescueArea);

				if (pRescueArea)
				{
					pHostage->pev->noise1 = 1;
					FireTargets(STRING(pRescueArea->pev->target), NULL, NULL, USE_TOGGLE, 0);
				}
			}
		}
	}
}