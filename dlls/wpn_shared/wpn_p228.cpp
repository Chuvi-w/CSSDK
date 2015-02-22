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

enum p228_e
{
	P228_IDLE,
	P228_SHOOT1,
	P228_SHOOT2,
	P228_SHOOT3,
	P228_SHOOT_EMPTY,
	P228_RELOAD,
	P228_DRAW
};

enum p228_shield_e
{
	P228_SHIELD_IDLE,
	P228_SHIELD_SHOOT1,
	P228_SHIELD_SHOOT2,
	P228_SHIELD_SHOOT_EMPTY,
	P228_SHIELD_RELOAD,
	P228_SHIELD_DRAW,
	P228_SHIELD_IDLE_UP,
	P228_SHIELD_UP,
	P228_SHIELD_DOWN
};

#define P228_MAX_SPEED     250
#define P228_RELOAD_TIME   2.7

LINK_ENTITY_TO_CLASS(weapon_p228, CP228);

void CP228::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_p228");

	Precache();
	m_iId = WEAPON_P228;
	SET_MODEL(ENT(pev), "models/w_p228.mdl");

	ClearBits(m_iWeaponState, WPNSTATE_SHIELD_DRAWN);

	m_iDefaultAmmo = P228_DEFAULT_GIVE;
	m_flAccuracy   = 0.9;

	FallInit();
}

void CP228::Precache(void)
{
	PRECACHE_MODEL("models/v_p228.mdl");
	PRECACHE_MODEL("models/w_p228.mdl");
	PRECACHE_MODEL("models/shield/v_shield_p228.mdl");

	PRECACHE_SOUND("weapons/p228-1.wav");
	PRECACHE_SOUND("weapons/p228_clipout.wav");
	PRECACHE_SOUND("weapons/p228_clipin.wav");
	PRECACHE_SOUND("weapons/p228_sliderelease.wav");
	PRECACHE_SOUND("weapons/p228_slidepull.wav");

	m_iShell = PRECACHE_MODEL("models/pshell.mdl");
	m_usFireP228 = PRECACHE_EVENT(1, "events/p228.sc");
}

int CP228::GetItemInfo(ItemInfo *p)
{
	p->pszName   = STRING(pev->classname);
	p->pszAmmo1  = "357SIG";
	p->iMaxAmmo1 = _357SIG_MAX_CARRY;
	p->pszAmmo2  = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip  = P228_MAX_CLIP;
	p->iSlot     = 1;
	p->iPosition = 3;
	p->iId       = m_iId = WEAPON_P228;
	p->iFlags    = 0;
	p->iWeight   = P228_WEIGHT;

	return 1;
}

int CP228::iItemSlot(void)
{
	return PISTOL_SLOT;
}

BOOL CP228::Deploy(void)
{
	m_flAccuracy = 0.9;
	m_fMaxSpeed  = P228_MAX_SPEED;

	ClearBits(m_iWeaponState, WPNSTATE_SHIELD_DRAWN);

	m_pPlayer->m_bShieldDrawn = false;

	if (m_pPlayer->HasShield())
		return DefaultDeploy("models/shield/v_shield_p228.mdl", "models/shield/p_shield_p228.mdl", P228_SHIELD_DRAW, "shieldgun", UseDecrement() != FALSE);
	else
		return DefaultDeploy("models/v_p228.mdl", "models/p_p228.mdl", P228_DRAW, "onehanded", UseDecrement() != FALSE);
}

void CP228::PrimaryAttack(void)
{
	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
	{
		P228Fire(1.5  * (1 - m_flAccuracy), 0.2, FALSE);
	}
	else if (m_pPlayer->pev->velocity.Length2D() > 0)
	{
		P228Fire(0.255 * (1 - m_flAccuracy), 0.2, FALSE);
	}
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
	{
		P228Fire(0.075 * (1 - m_flAccuracy), 0.2, FALSE);
	}
	else
	{
		P228Fire(0.15 * (1 - m_flAccuracy), 0.2, FALSE);
	}
}

void CP228::SecondaryAttack(void)
{
	ShieldSecondaryFire(SHIELDGUN_UP, SHIELDGUN_DOWN);
}

void CP228::P228Fire(float flSpread, float flCycleTime, BOOL fUseAutoAim)
{
	m_iShotsFired++;

	if (m_iShotsFired > 1)
	{
		return;
	}

	if (m_flLastFire)
	{
		m_flAccuracy -= (0.325 - (gpGlobals->time - m_flLastFire)) * 0.3;

		if (m_flAccuracy > 0.9)
		{
			m_flAccuracy = 0.9;
		}
		else if (m_flAccuracy < 0.6)
		{
			m_flAccuracy = 0.6;
		}
	}

	m_flLastFire = gpGlobals->time;

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

	SetPlayerShieldAnim();
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	m_pPlayer->m_iWeaponVolume = BIG_EXPLOSION_VOLUME;
	m_pPlayer->m_iWeaponFlash  = DIM_GUN_FLASH;

	Vector vecDir = m_pPlayer->FireBullets3(m_pPlayer->GetGunPosition(), gpGlobals->v_forward, flSpread,
		P228_DISTANCE, 1, BULLET_PLAYER_357SIG, P228_DAMAGE, P228_RANGE_MODIFER, m_pPlayer->pev, TRUE, m_pPlayer->random_seed);

	int flags;

#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFireP228, 0, (float*)&g_vecZero, (float*)&g_vecZero, vecDir.x, vecDir.y,
		(int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), m_iClip != 0, FALSE);

	flCycleTime -= 0.05;
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = GetNextAttackDelay(flCycleTime);

	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
	{
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
	}

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0;
	m_pPlayer->pev->punchangle.x -= 2;

	ResetPlayerShieldAnim();
}

void CP228::Reload(void)
{
	if (m_pPlayer->ammo_357sig <= 0)
	{
		return;
	}

	if (DefaultReload(P228_MAX_CLIP, m_pPlayer->HasShield() ? P228_SHIELD_RELOAD : P228_RELOAD, P228_RELOAD_TIME))
	{
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
		m_flAccuracy = 0.9;
	}
}

void CP228::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
	{
		return;
	}

	if (m_pPlayer->HasShield())
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20.0;

		if (FBitSet(m_iWeaponState, WPNSTATE_SHIELD_DRAWN))
		{
			SendWeaponAnim(P228_SHIELD_IDLE_UP, UseDecrement() != FALSE);
		}
	}
	else if (m_iClip)
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0625;
		SendWeaponAnim(P228_IDLE, UseDecrement() != FALSE);
	}
}

BOOL CP228::UseDecrement(void)
{
#if defined( CLIENT_WEAPONS )
	return TRUE;
#else
	return FALSE;
#endif
}

float CP228::GetMaxSpeed(void)
{
	return m_fMaxSpeed;
}

BOOL CP228::IsPistol(void)
{
	return TRUE;
}