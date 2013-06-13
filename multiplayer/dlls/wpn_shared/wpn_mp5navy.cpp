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

enum mp5n_e
{
    MP5N_IDLE1,
    MP5N_RELOAD,
    MP5N_DRAW,
    MP5N_SHOOT1,
    MP5N_SHOOT2,
    MP5N_SHOOT3
};

#define MP5N_MAX_SPEED      230
#define MP5N_RELOAD_TIME    2.63

LINK_ENTITY_TO_CLASS( weapon_mp5navy, CMP5N );

void CMP5N::Spawn( void )
{
    pev->classname = MAKE_STRING( "weapon_mp5navy" );

    Precache();
    m_iId = WEAPON_MP5N;
    SET_MODEL( edict(), "models/w_mp5.mdl" );

    m_iDefaultAmmo = MP5NAVY_DEFAULT_GIVE;
    m_flAccuracy   = 0;
    m_bDelayFire   = false;

    FallInit();
}

void CMP5N::Precache( void )
{
    PRECACHE_MODEL( "models/v_mp5.mdl" );
    PRECACHE_MODEL( "models/w_mp5.mdl" );

    PRECACHE_SOUND( "weapons/mp5-1.wav" );
    PRECACHE_SOUND( "weapons/mp5-2.wav" );
    PRECACHE_SOUND( "weapons/mp5_clipout.wav" );
    PRECACHE_SOUND( "weapons/mp5_clipin.wav" );
    PRECACHE_SOUND( "weapons/mp5_slideback.wav" );

    m_iShell = PRECACHE_MODEL( "models/pshell.mdl" );
    m_usFireMP5N = PRECACHE_EVENT(1, "events/mp5n.sc" );
}

int CMP5N::GetItemInfo( ItemInfo *p )
{
    p->pszName   = STRING( pev->classname );
    p->pszAmmo1  = "9mm";
    p->iMaxAmmo1 = _9MM_MAX_CARRY;
    p->pszAmmo2  = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip  = MP5N_MAX_CLIP;
    p->iSlot     = 0;
    p->iPosition = 7;
    p->iId       = m_iId = WEAPON_MP5N;
    p->iFlags    = 0;
    p->iWeight   = MP5NAVY_WEIGHT;

    return 1;
}

int CMP5N::iItemSlot( void )
{
    return PRIMARY_WEAPON_SLOT;
}

BOOL CMP5N::Deploy( void )
{
    iShellOn = 1;

    m_flAccuracy = 0;
    m_bDelayFire = false;

    return DefaultDeploy( "models/v_mp5.mdl", "models/p_mp5.mdl", MP5N_DRAW, "mp5", UseDecrement() != FALSE );
}

void CMP5N::PrimaryAttack( void )
{
    if( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
    {
        MP5NFire( 0.2 * m_flAccuracy, 0.075, FALSE );
    }
    else
    {
        MP5NFire( 0.04 * m_flAccuracy, 0.075, FALSE );
    }
}

void CMP5N::MP5NFire( float flSpread, float flCycleTime, BOOL fUseAutoAim ) 
{
    m_bDelayFire = true;
    m_iShotsFired++;

    m_flAccuracy = ( ( m_iShotsFired * m_iShotsFired * m_iShotsFired ) / 220 ) + 0.45;

    if( m_flAccuracy > 0.75 )
        m_flAccuracy = 0.75;

    if( m_iClip <= 0 )
    {
        if( m_fFireOnEmpty)
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
        MP5N_DISTANCE, MP5N_PENETRATION, BULLET_PLAYER_9MM, MP5N_DAMAGE, MP5N_RANGE_MODIFER, m_pPlayer->pev, FALSE, m_pPlayer->random_seed );

    int flags;

    #if defined( CLIENT_WEAPONS )
        flags = FEV_NOTHOST;
    #else
        flags = 0;
    #endif

    PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usFireMP5N, 0, ( float* )&g_vecZero, ( float* )&g_vecZero, vecDir.x, vecDir.y, 
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
        KickBack( 0.9, 0.475, 0.35, 0.0425, 5.0, 3.0, 6 );
    }
    else if( m_pPlayer->pev->velocity.Length2D() > 0 )
    {
        KickBack( 0.5, 0.275, 0.2, 0.03, 3.0, 2.0, 10 );
    }
    else if( FBitSet( m_pPlayer->pev->flags, FL_DUCKING ) )
    {
        KickBack( 0.225, 0.15, 0.1, 0.015, 2.0, 1.0, 10 );
    }
    else
    {
        KickBack( 0.25, 0.175, 0.125, 0.02, 2.25, 1.25, 10 );
    }
}

void CMP5N::Reload( void )
{
    if( m_pPlayer->ammo_9mm <= 0 )
    {
        return;
    }

    if( DefaultReload( MP5N_MAX_CLIP, MP5N_RELOAD, MP5N_RELOAD_TIME ) )
    {
        m_pPlayer->SetAnimation( PLAYER_RELOAD );

        m_flAccuracy  = 0;
        m_iShotsFired = 0;
    }
}

void CMP5N::WeaponIdle( void )
{
    ResetEmptySound();
    m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

    if( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
    {
        return;
    }

    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20.0;
    SendWeaponAnim( MP5N_IDLE1, UseDecrement() != FALSE );
}

BOOL CMP5N::UseDecrement( void )
{
    #if defined( CLIENT_WEAPONS )
        return TRUE;
    #else
        return FALSE;
    #endif
}

float CMP5N::GetMaxSpeed( void )
{
    return MP5N_MAX_SPEED;
}
