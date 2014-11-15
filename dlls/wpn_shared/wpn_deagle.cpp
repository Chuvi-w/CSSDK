/***
*
*   Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*
*   This product contains software technology licensed from Id
*   Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
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

enum deagle_e
{
    DEAGLE_IDLE1,
    DEAGLE_SHOOT1,
    DEAGLE_SHOOT2,
    DEAGLE_SHOOT_EMPTY,
    DEAGLE_RELOAD,
    DEAGLE_DRAW
};

#define DEAGLE_MAX_SPEED    250
#define DEAGLE_RELOAD_TIME  2.2

LINK_ENTITY_TO_CLASS( weapon_deagle, CDEAGLE );

void CDEAGLE::Spawn( void )
{
    pev->classname = MAKE_STRING( "weapon_deagle" );

    Precache();
    m_iId = WEAPON_DEAGLE;
    SET_MODEL( edict(), "models/w_deagle.mdl" );

    m_iDefaultAmmo  = DEAGLE_DEFAULT_GIVE;
    m_flAccuracy    = 0.9;
    m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;
    m_fMaxSpeed     = DEAGLE_MAX_SPEED;

    FallInit();
}

void CDEAGLE::Precache( void )
{
    PRECACHE_MODEL( "models/v_deagle.mdl" );
    PRECACHE_MODEL( "models/shield/v_shield_deagle.mdl" );
    PRECACHE_MODEL( "models/w_deagle.mdl" );

    PRECACHE_SOUND( "weapons/deagle-1.wav"   );
    PRECACHE_SOUND( "weapons/deagle-2.wav"   );
    PRECACHE_SOUND( "weapons/de_clipout.wav" );
    PRECACHE_SOUND( "weapons/de_clipin.wav"  );
    PRECACHE_SOUND( "weapons/de_deploy.wav"  );

    m_iShell = PRECACHE_MODEL( "models/pshell.mdl" );
    m_usFireDeagle = PRECACHE_EVENT( 1, "events/deagle.sc" );
}

int CDEAGLE::GetItemInfo( ItemInfo *p )
{
    p->pszName   = STRING(pev->classname);
    p->pszAmmo1  = "50AE";
    p->iMaxAmmo1 = _50AE_MAX_CARRY;
    p->pszAmmo2  = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip  = DEAGLE_MAX_CLIP;
    p->iSlot     = 1;
    p->iPosition = 1;
    p->iId       = m_iId = WEAPON_DEAGLE;
    p->iFlags    = 0;
    p->iWeight   = DEAGLE_WEIGHT;

    return 1;
}

int CDEAGLE::iItemSlot( void )
{
    return PISTOL_SLOT;
}

BOOL CDEAGLE::Deploy( void )
{
    m_flAccuracy    = 0.9;
    m_fMaxSpeed     = DEAGLE_MAX_SPEED;

    ClearBits( m_iWeaponState, WPNSTATE_SHIELD_DRAWN );

    m_pPlayer->m_bShieldDrawn = false;

    if( m_pPlayer->HasShield() )
        return DefaultDeploy( "models/shield/v_shield_deagle.mdl", "models/shield/p_shield_deagle.mdl", DEAGLE_DRAW, "shieldgun", UseDecrement() != FALSE );
    else
        return DefaultDeploy( "models/v_deagle.mdl", "models/p_deagle.mdl", DEAGLE_DRAW, "onehanded", UseDecrement() != FALSE );
}

void CDEAGLE::PrimaryAttack( void )
{
    if( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
    {
        DEAGLEFire( 1.5 * ( 1 - m_flAccuracy ), 0.3, FALSE );
    }
    else if( m_pPlayer->pev->velocity.Length2D() > 0 )
    {
        DEAGLEFire( 0.25 * ( 1 - m_flAccuracy ), 0.3, FALSE );
    }
    else if( FBitSet( m_pPlayer->pev->flags, FL_DUCKING ) )
    {
        DEAGLEFire( 0.115 * ( 1 - m_flAccuracy ), 0.3, FALSE );
    }
    else
    {
        DEAGLEFire( 0.13 * ( 1 - m_flAccuracy ), 0.3, FALSE );
    } 
}

void CDEAGLE::SecondaryAttack( void )
{
    ShieldSecondaryFire( SHIELDGUN_UP, SHIELDGUN_DOWN );
}

void CDEAGLE::DEAGLEFire( float flSpread, float flCycleTime, BOOL fUseAutoAim )
{
    m_iShotsFired++;

    if( m_iShotsFired > 1 )
    {
        return;
    }

    if( m_flLastFire )
    {
        m_flAccuracy -= ( 0.4 - ( gpGlobals->time - m_flLastFire    ) ) * 0.35;

        if( m_flAccuracy > 0.9 )
        {
            m_flAccuracy = 0.9;
        }
        else if( m_flAccuracy < 0.55 )
        {
            m_flAccuracy = 0.55;
        }
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

    SetPlayerShieldAnim();
    m_pPlayer->SetAnimation(PLAYER_ATTACK1);

    UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );

    m_pPlayer->m_iWeaponVolume = BIG_EXPLOSION_VOLUME;
    m_pPlayer->m_iWeaponFlash  = BRIGHT_GUN_FLASH;

    Vector vecSrc = m_pPlayer->GetGunPosition();
    Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 
        DEAGLE_DISTANCE, DEAGLE_PENETRATION, BULLET_PLAYER_50AE, DEAGLE_DAMAGE, DEAGLE_RANGE_MODIFER, m_pPlayer->pev, TRUE, m_pPlayer->random_seed );

    int flags;

    #if defined( CLIENT_WEAPONS )
        flags = FEV_NOTHOST;
    #else
        flags = 0;
    #endif

    PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usFireDeagle, 0, ( float* )&g_vecZero, ( float* )&g_vecZero, vecDir.x, vecDir.y, 
        ( int )(m_pPlayer->pev->punchangle.x * 100 ), ( int )( m_pPlayer->pev->punchangle.y * 100 ), m_iClip != 0, FALSE );

    flCycleTime -= 0.075;
    m_flNextPrimaryAttack = m_flNextSecondaryAttack = GetNextAttackDelay( flCycleTime );

    if( !m_iClip && m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] <= 0 )
    {
        m_pPlayer->SetSuitUpdate( "!HEV_AMO0", FALSE, 0 );
    }

    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.8;
    m_pPlayer->pev->punchangle.x -= 2;

    ResetPlayerShieldAnim();
}

void CDEAGLE::Reload( void )
{
    if( m_pPlayer->ammo_50ae <= 0 )
    {
        return;
    }

    if( DefaultReload( DEAGLE_MAX_CLIP, DEAGLE_RELOAD, DEAGLE_RELOAD_TIME ) )
    {
        m_pPlayer->SetAnimation( PLAYER_RELOAD );
        m_flAccuracy = 0.9;
    }
}

void CDEAGLE::WeaponIdle( void )
{
    ResetEmptySound();
    m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

    if( m_flTimeWeaponIdle <= UTIL_WeaponTimeBase() )
    {
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20.0;

        if( FBitSet( m_iWeaponState, WPNSTATE_SHIELD_DRAWN ) )
        {
            SendWeaponAnim( SHIELDGUN_DRAWN_IDLE, UseDecrement() != FALSE );
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

float CDEAGLE::GetMaxSpeed( void )
{
    return m_fMaxSpeed;
}

BOOL CDEAGLE::IsPistol( void )
{
    return TRUE;
}
