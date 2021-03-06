/***
*
*   Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*
*   This product contains software technology licensed from Id
*   Software, Inc. ( "Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*   All Rights Reserved.
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

enum p90_e
{
	P90_IDLE1,
	P90_RELOAD,
	P90_DRAW,
	P90_SHOOT1,
	P90_SHOOT2,
	P90_SHOOT3
};

#define P90_MAX_SPEED      240
#define P90_RELOAD_TIME    3.4

LINK_ENTITY_TO_CLASS(weapon_p90, CP90);

void CP90::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_p90");

	Precache();
	m_iId = WEAPON_P90;
	SET_MODEL(edict(), "models/w_p90.mdl");

	m_iDefaultAmmo = P90_DEFAULT_GIVE;
	m_flAccuracy   = 0.2;
	m_iShotsFired  = 0;
	m_bDelayFire   = false;

	FallInit();
}

void CP90::Precache(void)
{
	PRECACHE_MODEL("models/v_p90.mdl");
	PRECACHE_MODEL("models/w_p90.mdl");

	PRECACHE_SOUND("weapons/p90-1.wav");
	PRECACHE_SOUND("weapons/p90_clipout.wav");
	PRECACHE_SOUND("weapons/p90_clipin.wav");
	PRECACHE_SOUND("weapons/p90_boltpull.wav");
	PRECACHE_SOUND("weapons/p90_cliprelease.wav");

	m_iShell = PRECACHE_MODEL("models/rshell.mdl");
	m_usFireP90 = PRECACHE_EVENT(1, "events/p90.sc");
}

int CP90::GetItemInfo(ItemInfo *p)
{
	p->pszName   = STRING(pev->classname);
	p->pszAmmo1  = "57mm";
	p->iMaxAmmo1 = _57MM_MAX_CARRY;
	p->pszAmmo2  = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip  = P90_MAX_CLIP;
	p->iSlot     = 0;
	p->iPosition = 8;
	p->iId       = m_iId = WEAPON_P90;
	p->iFlags    = 0;
	p->iWeight   = P90_WEIGHT;

	return 1;
}

int CP90::iItemSlot(void)
{
	return PRIMARY_WEAPON_SLOT;
}

BOOL CP90::Deploy(void)
{
	iShellOn = 1;

	m_iShotsFired = 0;
	m_flAccuracy  = 0.2;
	m_bDelayFire  = false;

	return DefaultDeploy("models/v_p90.mdl", "models/p_p90.mdl", P90_DRAW, "carbine", UseDecrement() != FALSE);
}

void CP90::PrimaryAttack(void)
{
	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
	{
		P90Fire(0.3 * m_flAccuracy, 0.066, FALSE);
	}
	else if (m_pPlayer->pev->velocity.Length2D() > 170)
	{
		P90Fire(0.115 * m_flAccuracy, 0.066, FALSE);
	}
	else
	{
		P90Fire(0.045 * m_flAccuracy, 0.066, FALSE);
	}
}

void CP90::P90Fire(float flSpread, float flCycleTime, BOOL fUseAutoAim)
{
	m_bDelayFire = true;
	m_iShotsFired++;

	m_flAccuracy = ((m_iShotsFired * m_iShotsFired * m_iShotsFired) / 175) + 0.45;

	if (m_flAccuracy > 1)
		m_flAccuracy = 1;

	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = GetNextAttackDelay(0.2);
		}

		// TODO: Implement me.
		// if( TheBots )
		// {
		//     TheBots->OnEvent( EVENT_WEAPON_FIRED_ON_EMPTY, m_pPlayer, NULL );
		// }

		return;
	}

	m_iClip--;

	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash  = DIM_GUN_FLASH;

	Vector vecDir = m_pPlayer->FireBullets3(m_pPlayer->GetGunPosition(), gpGlobals->v_forward, flSpread,
		P90_DISTANCE, P90_PENETRATION, BULLET_PLAYER_57MM, P90_DAMAGE, P90_RANGE_MODIFER, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

	int flags;

#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFireP90, 0, (float*)&g_vecZero, (float*)&g_vecZero, vecDir.x, vecDir.y,
		(int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), 5, FALSE);

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = GetNextAttackDelay(flCycleTime);

	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
	{
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
	}

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0;

	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
	{
		KickBack(0.9, 0.45, 0.35, 0.04, 5.25, 3.5, 4);
	}
	else if (m_pPlayer->pev->velocity.Length2D() > 0)
	{
		KickBack(0.45, 0.3, 0.2, 0.0275, 4.0, 2.25, 7);
	}
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
	{
		KickBack(0.275, 0.2, 0.125, 0.02, 3.0, 1.0, 9);
	}
	else
	{
		KickBack(0.3, 0.225, 0.125, 0.02, 3.25, 1.25, 8);
	}
}

void CP90::Reload(void)
{
	if (m_pPlayer->ammo_57mm <= 0)
	{
		return;
	}

	if (DefaultReload(P90_MAX_CLIP, P90_RELOAD, P90_RELOAD_TIME))
	{
		m_pPlayer->SetAnimation(PLAYER_RELOAD);

		m_flAccuracy  = 0.2;
		m_iShotsFired = 0;
	}
}

void CP90::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
	{
		return;
	}

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20.0;
	SendWeaponAnim(P90_IDLE1, UseDecrement() != FALSE);
}

BOOL CP90::UseDecrement(void)
{
#if defined( CLIENT_WEAPONS )
	return TRUE;
#else
	return FALSE;
#endif
}

float CP90::GetMaxSpeed(void)
{
	return P90_MAX_SPEED;
}