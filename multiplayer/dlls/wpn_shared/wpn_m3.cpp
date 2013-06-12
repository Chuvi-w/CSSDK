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

enum m3_e
{
    M3_IDLE,
    M3_FIRE1,
    M3_FIRE2,
    M3_RELOAD,
    M3_PUMP,
    M3_START_RELOAD,
    M3_DRAW,
    M3_HOLSTER
};

#define M3_MAX_SPEED    230
#define VECTOR_CONE_M3	Vector( 0.06750, 0.06850, 0.00 )

LINK_ENTITY_TO_CLASS( weapon_m3, CM3  );

void CM3::Spawn( void )
{
    pev->classname = MAKE_STRING( "weapon_m3" );

    Precache();
    m_iId = WEAPON_M3;
    SET_MODEL( edict(), "models/w_m3.mdl" );

    m_iDefaultAmmo = M3_DEFAULT_GIVE;

    FallInit();
}

void CM3::Precache( void )
{
    PRECACHE_MODEL( "models/v_m3.mdl" );
    PRECACHE_MODEL( "models/w_m3.mdl" );

    m_iShellId = m_iShell = PRECACHE_MODEL( "models/shotgunshell.mdl" );

    PRECACHE_SOUND( "weapons/m3-1.wav" );
    PRECACHE_SOUND( "weapons/m3_insertshell.wav" );
    PRECACHE_SOUND( "weapons/m3_pump.wav" );
    PRECACHE_SOUND( "weapons/reload1.wav" );
    PRECACHE_SOUND( "weapons/reload3.wav" );

    m_usFireM3 = PRECACHE_EVENT(1, "events/m3.sc" );
}

int CM3::GetItemInfo( ItemInfo *p )
{
    p->pszName   = STRING( pev->classname );
    p->pszAmmo1  = "buckshot";
    p->iMaxAmmo1 = BUCKSHOT_MAX_CARRY;
    p->pszAmmo2  = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip  = M3_MAX_CLIP;
    p->iSlot     = 0;
    p->iPosition = 5;
    p->iId       = m_iId = WEAPON_M3;
    p->iFlags    = 0;
    p->iWeight   = M3_WEIGHT;

    return 1;
}

int CM3::iItemSlot( void )
{
    return PRIMARY_WEAPON_SLOT;
}

BOOL CM3::Deploy( void )
{
    return DefaultDeploy( "models/v_m3.mdl", "models/p_m3.mdl", M3_DRAW, "shotgun", UseDecrement() != FALSE );
}

void CM3::PrimaryAttack( void )
{
    if( m_pPlayer->pev->waterlevel == 3 )
    {
        PlayEmptySound();
        m_flNextPrimaryAttack = GetNextAttackDelay( 0.15 );
        return;
    }

    if( m_iClip <= 0 )
    {
        Reload();

        if( !m_iClip )
        {
            PlayEmptySound();
        }

        // TODO: Implement me.
        // if( TheBots )
        // {
        //     TheBots->OnEvent( EVENT_WEAPON_FIRED_ON_EMPTY, m_pPlayer, NULL );
        // }

        return;
    }

    m_iClip--;

    m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
    m_pPlayer->m_iWeaponFlash  = BRIGHT_GUN_FLASH;

    m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
    m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

    UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );

    m_pPlayer->FireBullets( 9, m_pPlayer->GetGunPosition(), gpGlobals->v_forward, VECTOR_CONE_M3, 3000, BULLET_PLAYER_BUCKSHOT, 0 );

    int flags;

    #if defined( CLIENT_WEAPONS )
        flags = FEV_NOTHOST;
    #else
        flags = 0;
    #endif

    PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usFireM3, 0, ( float* )&g_vecZero, ( float* )&g_vecZero, 0, 0, 0, 0, FALSE, FALSE );

    if( !m_iClip && m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] <= 0 )
    {
        m_pPlayer->SetSuitUpdate( "!HEV_AMO0", FALSE, 0 );
    }

    if( m_iClip)
    {
        m_flPumpTime = UTIL_WeaponTimeBase() + 0.5;
    }

    m_flNextPrimaryAttack  =  GetNextAttackDelay( 0.875 );
    m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.875;

    if( m_iClip )
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.5;
    else
        m_flTimeWeaponIdle = 0.875;

    m_fInSpecialReload = 0;

    if( FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
        m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong( m_pPlayer->random_seed + 1, 4, 6 );
    else
        m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong( m_pPlayer->random_seed + 1, 8, 11 );

    m_pPlayer->m_flEjectBrass = gpGlobals->time + 0.45;
}

void CM3::Reload( void )
{
    if( m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] <= 0 || m_iClip == M3_MAX_CLIP )
    {
        return;
    }

    if( m_flNextPrimaryAttack > UTIL_WeaponTimeBase() )
    {
        return;
    }

    if( !m_fInSpecialReload )
    {
        m_pPlayer->SetAnimation( PLAYER_RELOAD );
        SendWeaponAnim( M3_START_RELOAD, UseDecrement() != FALSE );

        m_fInSpecialReload = 1;

        m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.55;
        m_flTimeWeaponIdle        = UTIL_WeaponTimeBase() + 0.55;
        m_flNextPrimaryAttack     = GetNextAttackDelay( 0.55 );
        m_flNextSecondaryAttack   = UTIL_WeaponTimeBase() + 0.55;
    }
    else if( m_fInSpecialReload == 1 )
    {
        if( m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
        {
            return;
        }

        m_fInSpecialReload = 2;
        SendWeaponAnim( M3_RELOAD, UseDecrement() != FALSE );

        m_flNextReload     = UTIL_WeaponTimeBase() + 0.45;
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.45;
    }
    else
    {
        m_iClip++;

        m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ]--;   
        m_pPlayer->ammo_buckshot--;

        m_fInSpecialReload = 1;
    }
}

void CM3::WeaponIdle( void )
{
    ResetEmptySound();
    m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );

    if( m_flPumpTime && m_flPumpTime < UTIL_WeaponTimeBase() )
    {
        m_flPumpTime = 0;
    }

    if( m_flTimeWeaponIdle < UTIL_WeaponTimeBase() )
    {
        if( m_iClip == 0 && m_fInSpecialReload == 0 && m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] )
        {
            Reload();
        }
        else if( m_fInSpecialReload != 0 )
        {
            if( m_iClip != M3_MAX_CLIP && m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] )
            {
                Reload();
            }
            else
            {
                SendWeaponAnim( M3_PUMP, UseDecrement() != FALSE );

                m_fInSpecialReload = 0;
                m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5;
            }
        }
        else
        {
            SendWeaponAnim(M3_IDLE, UseDecrement() != FALSE );
        }
    }
}

BOOL CM3::UseDecrement( void )
{
    #if defined( CLIENT_WEAPONS )
        return TRUE;
    #else
        return FALSE;
    #endif
}

float CM3::GetMaxSpeed( void )
{
    return M3_MAX_SPEED;
}