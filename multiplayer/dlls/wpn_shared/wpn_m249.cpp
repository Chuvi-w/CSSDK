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

enum m249_e
{
    M249_IDLE1,
    M249_SHOOT1,
    M249_SHOOT2,
    M249_RELOAD,
    M249_DRAW
};

#define M249_MAX_SPEED      220
#define M249_RELOAD_TIME    4.7

LINK_ENTITY_TO_CLASS( weapon_m249, CM249 );

void CM249::Spawn( void )
{
    pev->classname = MAKE_STRING( "weapon_m249" );

    Precache();
    m_iId = WEAPON_M249;
    SET_MODEL( edict(), "models/w_m249.mdl" );

    m_iDefaultAmmo = M249_DEFAULT_GIVE;
    m_flAccuracy   = 0.2;
    m_iShotsFired  = 0;

    FallInit();
}

void CM249::Precache( void )
{
    PRECACHE_MODEL( "models/v_m249.mdl" );
    PRECACHE_MODEL( "models/w_m249.mdl" );

    PRECACHE_SOUND( "weapons/m249-1.wav" );
    PRECACHE_SOUND( "weapons/m249-2.wav" );
    PRECACHE_SOUND( "weapons/m249_boxout.wav" );
    PRECACHE_SOUND( "weapons/m249_boxin.wav" );
    PRECACHE_SOUND( "weapons/m249_chain.wav" );
    PRECACHE_SOUND( "weapons/m249_coverup.wav" );
    PRECACHE_SOUND( "weapons/m249_coverdown.wav" );

    m_iShell = PRECACHE_MODEL( "models/rshell.mdl" );
    m_usFireM249 = PRECACHE_EVENT(1, "events/m249.sc" );
}

int CM249::GetItemInfo( ItemInfo *p )
{
    p->pszName    = STRING(pev->classname );
    p->pszAmmo1  = "556NatoBox";
    p->iMaxAmmo1 = _556NATOBOX_MAX_CARRY;
    p->pszAmmo2  = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip  = M249_MAX_CLIP;
    p->iSlot     = 0;
    p->iPosition = 4;
    p->iId       = m_iId = WEAPON_M249;
    p->iFlags    = 0;
    p->iWeight   = M249_WEIGHT;

    return 1;
}

int CM249::iItemSlot( void )
{
    return PRIMARY_WEAPON_SLOT;
}

BOOL CM249::Deploy( void )
{
    m_flAccuracy  = 0.2;
    m_iShotsFired = 0;
 
    iShellOn = 1;

    return DefaultDeploy( "models/v_m249.mdl", "models/p_m249.mdl", M249_DRAW, "m249", UseDecrement() != FALSE );
}

void CM249::PrimaryAttack( void )
{
    if( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
    {
        M249Fire( 0.045 + ( 0.5 * m_flAccuracy ), 0.1, FALSE );
    }
    else if( m_pPlayer->pev->velocity.Length2D() > 140 )
    {
        M249Fire( 0.045 + ( 0.095 * m_flAccuracy ), 0.1, FALSE );
    }
    else
    {
        M249Fire( 0.03 * m_flAccuracy, 0.1, FALSE );
    }
}

void CM249::M249Fire(float flSpread, float flCycleTime, BOOL fUseAutoAim)
{
    m_bDelayFire = true;
    m_iShotsFired++;

    m_flAccuracy = ( ( m_iShotsFired * m_iShotsFired * m_iShotsFired ) / 175 ) + 0.4;

    if( m_flAccuracy > 0.9 )
        m_flAccuracy = 0.9;

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

    m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
    m_pPlayer->m_iWeaponFlash  = BRIGHT_GUN_FLASH;

    Vector vecDir = m_pPlayer->FireBullets3( m_pPlayer->GetGunPosition(), gpGlobals->v_forward, flSpread, 
        M249_DISTANCE, M249_PENETRATION, BULLET_PLAYER_556MM, M249_DAMAGE, M249_RANGE_MODIFER, m_pPlayer->pev, FALSE, m_pPlayer->random_seed );

    int flags;

    #if defined( CLIENT_WEAPONS )
        flags = FEV_NOTHOST;
    #else
        flags = 0;
    #endif

    PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usFireM249, 0, ( float* )&g_vecZero, ( float* )&g_vecZero, vecDir.x, vecDir.y, 
        ( int )( m_pPlayer->pev->punchangle.x * 100 ), ( int )( m_pPlayer->pev->punchangle.y * 100 ), FALSE, FALSE );

    m_flNextPrimaryAttack = m_flNextSecondaryAttack = GetNextAttackDelay( flCycleTime );

    if( !m_iClip && m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] <= 0 )
    {
        m_pPlayer->SetSuitUpdate( "!HEV_AMO0", FALSE, 0 );
    }

    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.6;

    if( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
    {
        KickBack( 1.8, 0.65, 0.45, 0.125, 5.0, 3.5, 8 );
    }
    else if( m_pPlayer->pev->velocity.Length2D() > 0 )
    {
        KickBack( 1.1, 0.5, 0.3, 0.06, 4.0, 3.0, 8 );
    }
    else if( FBitSet( m_pPlayer->pev->flags, FL_DUCKING ) )
    {
        KickBack( 0.75, 0.325, 0.25, 0.025, 3.5, 2.5, 9 );
    }
    else
    {
        KickBack( 0.8, 0.35, 0.3, 0.03, 3.75, 3.0, 9 );
    }
}

void CM249::Reload( void )
{
    if( m_pPlayer->ammo_556natobox <= 0 )
    {
        return;
    }

    if( DefaultReload( M249_MAX_CLIP, M249_RELOAD, M249_RELOAD_TIME ) )
    {
        m_pPlayer->SetAnimation( PLAYER_RELOAD );

        m_flAccuracy  = 0.2;
        m_bDelayFire  = false;
        m_iShotsFired = 0;
    }
}

void CM249::WeaponIdle( void )
{
    ResetEmptySound();
    m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

    if( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
    {
        return;
    }
    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20.0;
    SendWeaponAnim(M249_IDLE1, UseDecrement() != FALSE );
}

BOOL CM249::UseDecrement( void )
{
    #if defined( CLIENT_WEAPONS )
        return TRUE;
    #else
        return FALSE;
    #endif
}

float CM249::GetMaxSpeed( void )
{
    return M249_MAX_SPEED;
}
