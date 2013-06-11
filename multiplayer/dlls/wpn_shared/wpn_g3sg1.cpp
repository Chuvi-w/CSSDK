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

enum g3sg1_e
{
    G3SG1_IDLE,
    G3SG1_SHOOT,
    G3SG1_SHOOT2,
    G3SG1_RELOAD,
    G3SG1_DRAW
};

#define G3SG1_MAX_SPEED         210
#define G3SG1_MAX_SPEED_ZOOM    150
#define G3SG1_RELOAD_TIME       3.5

LINK_ENTITY_TO_CLASS( weapon_g3sg1, CG3SG1 );

void CG3SG1::Spawn( void )
{
    pev->classname = MAKE_STRING( "weapon_g3sg1" );

    Precache();
    m_iId = WEAPON_G3SG1;
    SET_MODEL( edict(), "models/w_g3sg1.mdl" );

    m_iDefaultAmmo = G3SG1_DEFAULT_GIVE;
    m_flLastFire   = 0;

    FallInit();
}

void CG3SG1::Precache( void )
{
    PRECACHE_MODEL( "models/v_g3sg1.mdl" );
    PRECACHE_MODEL( "models/w_g3sg1.mdl" );

    PRECACHE_SOUND( "weapons/g3sg1-1.wav" );
    PRECACHE_SOUND( "weapons/g3sg1_slide.wav" );
    PRECACHE_SOUND( "weapons/g3sg1_clipin.wav" );
    PRECACHE_SOUND( "weapons/g3sg1_clipout.wav" );
    PRECACHE_SOUND( "weapons/zoom.wav" );

    m_iShellId = m_iShell = PRECACHE_MODEL( "models/rshell.mdl" );
    m_usFireG3SG1 = PRECACHE_EVENT(1, "events/g3sg1.sc" );
}

int CG3SG1::GetItemInfo( ItemInfo *p )
{
    p->pszName   = STRING( pev->classname );
    p->pszAmmo1  = "762Nato";
    p->iMaxAmmo1 = _762NATO_MAX_CARRY;
    p->pszAmmo2  = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip  = G3SG1_MAX_CLIP;
    p->iSlot     = 0;
    p->iPosition = 3;
    p->iId       = m_iId = WEAPON_G3SG1;
    p->iFlags    = 0;
    p->iWeight   = WEAPON_G3SG1;

    return 1;
}

int CG3SG1::iItemSlot( void )
{
    return PRIMARY_WEAPON_SLOT;
}

BOOL CG3SG1::Deploy( void )
{
    m_flAccuracy = 0.2;

    return DefaultDeploy( "models/v_g3sg1.mdl", "models/p_g3sg1.mdl", G3SG1_DRAW, "mp5", UseDecrement() != FALSE );
}

void CG3SG1::PrimaryAttack( void )
{
    if( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
    {
        G3SG1Fire( 0.45, 0.25, FALSE );
    }
    else if( m_pPlayer->pev->velocity.Length2D() > 0 )
    {
        G3SG1Fire( 0.15, 0.25, FALSE );
    }
    else if( FBitSet( m_pPlayer->pev->flags, FL_DUCKING ) )
    {
        G3SG1Fire( 0.035, 0.25, FALSE );
    }
    else
    {
        G3SG1Fire( 0.055, 0.25, FALSE );
    }
}

void CG3SG1::SecondaryAttack( void )
{
    switch( m_pPlayer->m_iFOV )
    {
        case 90: m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 40; break;
        case 40: m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 15; break;
        default: m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 90; break;
    }

    m_pPlayer->ResetMaxSpeed();

    // TODO: Implement me.
    // if( TheBots )
    // {
    //     TheBots->OnEvent( EVENT_WEAPON_ZOOMED, m_pPlayer, NULL );
    // }

    EMIT_SOUND( edict(), CHAN_ITEM, "weapons/zoom.wav", 0.2, 2.4 );

    m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3;
}


void CG3SG1::G3SG1Fire( float flSpread, float flCycleTime, BOOL fUseAutoAim )
{
    if( m_pPlayer->pev->fov == 90 )
    {
        flSpread  += 0.025;
    }

    float spreadModifier = 0.02;

    if( m_flLastFire )
    {
        m_flAccuracy = ( gpGlobals->time - m_flLastFire ) * 0.3 + 0.55;

        if( m_flAccuracy > 0.98 )
        {
            m_flAccuracy = 0.98;
        }
        else
        {
            spreadModifier = 1 - m_flAccuracy;
        }
    }
    else
    {
        m_flAccuracy = 0.98;
    }

    m_flLastFire = gpGlobals->time;

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

    m_pPlayer->m_iWeaponVolume = BIG_EXPLOSION_VOLUME;
    m_pPlayer->m_iWeaponFlash  = NORMAL_GUN_FLASH;

    Vector vecDir = FireBullets3( m_pPlayer->GetGunPosition(), gpGlobals->v_forward, flSpread * spreadModifier, 
        G3SG1_DISTANCE, G3SG1_PENETRATION, BULLET_PLAYER_762MM, G3SG1_DAMAGE, G3SG1_RANGE_MODIFER, m_pPlayer->pev, TRUE, m_pPlayer->random_seed );

    int flags;

    #if defined( CLIENT_WEAPONS )
        flags = FEV_NOTHOST;
    #else
        flags = 0;
    #endif

    PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usFireG3SG1, 0, ( float* )&g_vecZero, ( float* )&g_vecZero, vecDir.x, vecDir.y, 
        ( int )( m_pPlayer->pev->punchangle.x * 100 ), ( int )( m_pPlayer->pev->punchangle.y * 100 ), TRUE, FALSE );
    
    m_flNextPrimaryAttack = m_flNextSecondaryAttack = GetNextAttackDelay( flCycleTime );

    if (!m_iClip && m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] <= 0 )
    {
        m_pPlayer->SetSuitUpdate( "!HEV_AMO0", FALSE, 0 );
    }

    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.8;

    m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomFloat( m_pPlayer->random_seed + 4, 2.75, 3.25 ) + m_pPlayer->pev->punchangle.x * 0.25;
    m_pPlayer->pev->punchangle.y += UTIL_SharedRandomFloat( m_pPlayer->random_seed + 5, -1.25, 1.5 );
}

void CG3SG1::Reload( void )
{
    if( m_pPlayer->ammo_762nato <= 0 ) 
    {
        return;
    }

    if( DefaultReload( G3SG1_MAX_CLIP, G3SG1_RELOAD, G3SG1_RELOAD_TIME ) )
    {
        m_flAccuracy = 0.2;
        m_pPlayer->SetAnimation( PLAYER_RELOAD );

        if( m_pPlayer->pev->fov != 90 )
        {
            m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 15;
            SecondaryAttack();
        }
    }
}

void CG3SG1::WeaponIdle( void )
{
    ResetEmptySound();
    m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

    if( m_flTimeWeaponIdle <= UTIL_WeaponTimeBase() )
    {
        if( m_iClip )
        {
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 60.0;
            SendWeaponAnim( G3SG1_IDLE, UseDecrement() != FALSE );
        }
    }
}

BOOL CDEAGLE::UseDecrement( void )
{
    #if defined( CLIENT_WEAPONS )
        return TRUE;
    #else
        return FALSE;
    #endif
}

float CG3SG1::GetMaxSpeed( void )
{
    return m_pPlayer->m_iFOV ? G3SG1_MAX_SPEED : G3SG1_MAX_SPEED_ZOOM;
}