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

enum ump45_e
{
    UMP45_IDLE1,
    UMP45_RELOAD,
    UMP45_DRAW,
    UMP45_SHOOT1,
    UMP45_SHOOT2,
    UMP45_SHOOT3
};

#define UMP45_MAX_SPEED      250
#define UMP45_RELOAD_TIME    3.5

LINK_ENTITY_TO_CLASS( weapon_ump45, CUMP45 );

void CUMP45::Spawn( void )
{
    pev->classname = MAKE_STRING( "weapon_ump45" );

    Precache();
    m_iId = WEAPON_UMP45;
    SET_MODEL( edict(), "models/w_ump45.mdl" );

    m_iDefaultAmmo = UMP45_DEFAULT_GIVE;
    m_flAccuracy   = 0;
    m_bDelayFire   = false;

    FallInit();
}

void CUMP45::Precache( void )
{
    PRECACHE_MODEL( "models/v_ump45.mdl" );
    PRECACHE_MODEL( "models/w_ump45.mdl" );

    PRECACHE_SOUND( "weapons/ump45-1.wav" );
    PRECACHE_SOUND( "weapons/ump45_clipout.wav" );
    PRECACHE_SOUND( "weapons/ump45_clipin.wav" );
    PRECACHE_SOUND( "weapons/ump45_boltslap.wav" );

    m_iShell = PRECACHE_MODEL( "models/pshell.mdl" );
    m_usFireUMP45 = PRECACHE_EVENT( 1, "events/ump45.sc" );
}

int CUMP45::GetItemInfo( ItemInfo *p )
{
    p->pszName   = STRING( pev->classname );
    p->pszAmmo1  = "45acp";
    p->iMaxAmmo1 = _45ACP_MAX_CARRY;
    p->pszAmmo2  = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip  = UMP45_MAX_CLIP;
    p->iSlot     = 0;
    p->iPosition = 15;
    p->iId       = m_iId = WEAPON_UMP45;
    p->iFlags    = 0;
    p->iWeight   = UMP45_WEIGHT;

    return 1;
}

int CUMP45::iItemSlot( void )
{
    return PRIMARY_WEAPON_SLOT;
}

BOOL CUMP45::Deploy( void )
{
    iShellOn = 1;

    m_flAccuracy = 0;
    m_bDelayFire = false;

    return DefaultDeploy( "models/v_ump45.mdl", "models/p_ump45.mdl", UMP45_DRAW, "carbine", UseDecrement() != FALSE );
}

void CUMP45::PrimaryAttack( void )
{
    if( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
    {
        UMP45Fire( 0.24 * m_flAccuracy, 0.1, FALSE );
    }
    else
    {
        UMP45Fire( 0.04 * m_flAccuracy, 0.1, FALSE );
    }
}

void CUMP45::UMP45Fire( float flSpread, float flCycleTime, BOOL fUseAutoAim)
{
    m_bDelayFire = true;
    m_iShotsFired++;

    m_flAccuracy = ( ( m_iShotsFired * m_iShotsFired * m_iShotsFired ) / 210 ) + 0.5;

    if( m_flAccuracy > 1 )
        m_flAccuracy = 1;

    if( m_iClip <= 0 )
    {
        if( m_fFireOnEmpty )
        {
            PlayEmptySound();
            m_flNextPrimaryAttack = GetNextAttackDelay( 0.2 );
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
    m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

    UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );

    Vector vecDir = m_pPlayer->FireBullets3( m_pPlayer->GetGunPosition(), gpGlobals->v_forward, flSpread, 
        TMP_DISTANCE, TMP_PENETRATION, BULLET_PLAYER_45ACP, TMP_DAMAGE, TMP_RANGE_MODIFER, m_pPlayer->pev, FALSE, m_pPlayer->random_seed );

    int flags;

    #if defined( CLIENT_WEAPONS )
        flags = FEV_NOTHOST;
    #else
        flags = 0;
    #endif

    PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usFireUMP45, 0, ( float* )&g_vecZero, ( float* )&g_vecZero, vecDir.x, vecDir.y, 
        ( int )( m_pPlayer->pev->punchangle.x * 100 ), ( int )( m_pPlayer->pev->punchangle.y * 100 ), FALSE, FALSE );

    m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
    m_pPlayer->m_iWeaponFlash  = DIM_GUN_FLASH;

    m_flNextPrimaryAttack = m_flNextSecondaryAttack = GetNextAttackDelay( flCycleTime );

    if( !m_iClip && m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] <= 0 )
    {
        m_pPlayer->SetSuitUpdate( "!HEV_AMO0", FALSE, 0 );
    }

    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0;

    if( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
    {
        KickBack( 0.125, 0.65, 0.55, 0.0475, 5.5, 4.0, 10 );
    }
    else if( m_pPlayer->pev->velocity.Length2D() > 0 )
    {
        KickBack( 0.55, 0.3, 0.225, 0.03, 3.5, 2.5, 10 );
    }
    else if( FBitSet( m_pPlayer->pev->flags, FL_DUCKING ) )
    {
        KickBack( 0.25, 0.175, 0.125, 0.02, 2.25, 1.25, 10 );
    }
    else
    {
        KickBack( 0.275, 0.2, 0.15, 0.0225, 2.5, 1.5, 10 );
    }
}

void CUMP45::Reload( void )
{
    if( m_pPlayer->ammo_45acp <= 0 )
    {
        return;
    }

    if( DefaultReload( UMP45_MAX_CLIP, UMP45_RELOAD, UMP45_RELOAD_TIME ) )
    {
        m_pPlayer->SetAnimation( PLAYER_RELOAD );

        m_flAccuracy  = 0;
        m_iShotsFired = 0;
    }
}

void CUMP45::WeaponIdle( void )
{
    ResetEmptySound();
    m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

    if( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
    {
        return;
    }

    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20.0;
    SendWeaponAnim( UMP45_IDLE1, UseDecrement() != FALSE );
}

BOOL CUMP45::UseDecrement( void )
{
    #if defined( CLIENT_WEAPONS )
        return TRUE;
    #else
        return FALSE;
    #endif
}

float CUMP45::GetMaxSpeed( void )
{
    return UMP45_MAX_SPEED;
}
