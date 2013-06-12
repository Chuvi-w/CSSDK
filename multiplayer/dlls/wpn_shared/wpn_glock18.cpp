/***
*
*   Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*
*   This product contains software technology licensed from Id
*   Software, Inc. ( "Id Technology" ).  Id Technology (c) 1996 Id Software, Inc.
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

enum glock18_e
{
    GLOCK18_IDLE1,
    GLOCK18_IDLE2,
    GLOCK18_IDLE3,
    GLOCK18_SHOOT,
    GLOCK18_SHOOT2,
    GLOCK18_SHOOT3,
    GLOCK18_SHOOT_EMPTY,
    GLOCK18_RELOAD,
    GLOCK18_DRAW,
    GLOCK18_HOLSTER,
    GLOCK18_ADD_SILENCER,
    GLOCK18_DRAW2,
    GLOCK18_RELOAD2
};

enum glock18_shield_e
{
    GLOCK18_SHIELD_IDLE1,
    GLOCK18_SHIELD_SHOOT,
    GLOCK18_SHIELD_SHOOT2,
    GLOCK18_SHIELD_SHOOT_EMPTY,
    GLOCK18_SHIELD_RELOAD,
    GLOCK18_SHIELD_DRAW,
    GLOCK18_SHIELD_IDLE,
    GLOCK18_SHIELD_UP,
    GLOCK18_SHIELD_DOWN
};

#define GLOCK18_MAX_SPEED     250
#define GLOCK18_RELOAD_TIME   2.2

LINK_ENTITY_TO_CLASS( weapon_glock18, CGLOCK18 );

void CGLOCK18::Spawn( void )
{
    pev->classname = MAKE_STRING( "weapon_glock18" );

    Precache();
    m_iId = WEAPON_GLOCK18;
    SET_MODEL( edict(), "models/w_glock18.mdl" );

    ClearBits( m_iWeaponState, WPNSTATE_SHIELD_DRAWN );

    m_iDefaultAmmo       = GLOCK18_DEFAULT_GIVE;
    m_bBurstFire         = false;
    m_iGlock18ShotsFired = 0;
    m_flGlock18Shoot     = 0;
    m_flAccuracy         = 0.9;

    FallInit();
}

void CGLOCK18::Precache( void )
{
    PRECACHE_MODEL( "models/v_glock18.mdl" );
    PRECACHE_MODEL( "models/w_glock18.mdl" );
    PRECACHE_MODEL( "models/shield/v_shield_glock18.mdl" );

    PRECACHE_SOUND( "weapons/glock18-1.wav" );
    PRECACHE_SOUND( "weapons/glock18-2.wav" );
    PRECACHE_SOUND( "weapons/clipout1.wav"  );
    PRECACHE_SOUND( "weapons/clipin1.wav"   );
    PRECACHE_SOUND( "weapons/sliderelease1.wav" );
    PRECACHE_SOUND( "weapons/slideback1.wav" );
    PRECACHE_SOUND( "weapons/357_cock1.wav"  );
    PRECACHE_SOUND( "weapons/de_clipin.wav"  );
    PRECACHE_SOUND( "weapons/de_clipout.wav" );

    m_iShellId = m_iShell = PRECACHE_MODEL( "models/pshell.mdl" );
    m_usFireGlock18 = PRECACHE_EVENT( 1, "events/glock18.sc" );
}

int CGLOCK18::GetItemInfo( ItemInfo *p )
{
    p->pszName   = STRING( pev->classname );
    p->pszAmmo1  = "9mm";
    p->iMaxAmmo1 = _9MM_MAX_CARRY;
    p->pszAmmo2  = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip  = GLOCK18_MAX_CLIP;
    p->iSlot     = 1;
    p->iPosition = 2;
    p->iId       = m_iId = WEAPON_GLOCK18;
    p->iFlags    = 0;
    p->iWeight   = GLOCK18_WEIGHT;

    return 1;
}

int CGLOCK18::iItemSlot( void )
{
    return PISTOL_SLOT;
}

BOOL CGLOCK18::Deploy( void )
{
    ClearBits( m_iWeaponState, WPNSTATE_SHIELD_DRAWN );

    m_bBurstFire         = 0;
    m_iGlock18ShotsFired = 0;
    m_flGlock18Shoot     = 0;
    m_flAccuracy         = 0.9;
    m_fMaxSpeed          = GLOCK18_MAX_SPEED;

    m_pPlayer->m_bShieldDrawn = false;

    if( m_pPlayer->HasShield() )
        return DefaultDeploy( "models/shield/v_shield_glock18.mdl", "models/shield/p_shield_glock18.mdl", GLOCK18_SHIELD_DRAW, "shieldgun", UseDecrement() != FALSE );
    else
        return DefaultDeploy( "models/v_glock18.mdl", "models/p_glock18.mdl", RANDOM_LONG( 0, 1 ) ? GLOCK18_DRAW : GLOCK18_DRAW2, "onehanded", UseDecrement() != FALSE );
}

void CGLOCK18::PrimaryAttack( void )
{
    if( FBitSet( m_iWeaponState, WPNSTATE_GLOCK18_BURST_MODE ) )
    {
        if( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
        {
            GLOCK18Fire( 1.2 * ( 1 - m_flAccuracy ), 0.5, TRUE );
        }
        else if( m_pPlayer->pev->velocity.Length2D() > 0)
        {
            GLOCK18Fire( 0.185 * ( 1 - m_flAccuracy ), 0.5, TRUE );
        }
        else if( FBitSet( m_pPlayer->pev->flags, FL_DUCKING ) )
        {
            GLOCK18Fire( 0.095 * ( 1 - m_flAccuracy ), 0.5, TRUE );
        }
        else
        {
            GLOCK18Fire( 0.3 * ( 1 - m_flAccuracy ), 0.5, TRUE );
        }
    }
    else
    {
        if( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
        {
            GLOCK18Fire( 1.0 * ( 1 - m_flAccuracy ), 0.2, FALSE );
        }
        else if( m_pPlayer->pev->velocity.Length2D() > 0 )
        {
            GLOCK18Fire( 0.165 * ( 1 - m_flAccuracy ), 0.2, FALSE );
        }
        else if( FBitSet( m_pPlayer->pev->flags, FL_DUCKING ) )
        {
            GLOCK18Fire( 0.075 * ( 1 - m_flAccuracy ), 0.2, FALSE );
        }
        else
        {
            GLOCK18Fire( 0.1 * ( 1 - m_flAccuracy ), 0.2, FALSE );
        }
    }
}

void CGLOCK18::SecondaryAttack( void )
{
    if( ShieldSecondaryFire( GLOCK18_SHIELD_UP, GLOCK18_SHIELD_DOWN ) )
    {
        return;
    }

    if( FBitSet( m_iWeaponState, WPNSTATE_GLOCK18_BURST_MODE ) )
    {
        ClientPrint( m_pPlayer->pev, HUD_PRINTCENTER, "#Switch_To_SemiAuto" );
        ClearBits( m_iWeaponState, WPNSTATE_GLOCK18_BURST_MODE );
    }
    else
    {
        ClientPrint( m_pPlayer->pev, HUD_PRINTCENTER, "#Switch_To_BurstFire" );
        SetBits( m_iWeaponState, WPNSTATE_GLOCK18_BURST_MODE );
    }

    m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3;
}

void CGLOCK18::GLOCK18Fire( float flSpread, float flCycleTime, BOOL fUseBurstMode )
{
    if( fUseBurstMode )
    {
        m_iGlock18ShotsFired = 0;
    }
    else
    {
        m_iShotsFired++;

        if( m_iShotsFired > 1 )
        {
            return;
        }

        flCycleTime -= 0.05;
    }

    if( m_flLastFire )
    {
        m_flAccuracy -= ( 0.325 - ( gpGlobals->time - m_flLastFire ) ) * 0.275;

        if( m_flAccuracy > 0.9 )
        {
            m_flAccuracy = 0.9;
        }
        else if( m_flAccuracy < 0.6 )
        {
            m_flAccuracy = 0.6;
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
    m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

    UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );

    m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
    m_pPlayer->m_iWeaponFlash  = BRIGHT_GUN_FLASH;

    Vector vecDir = m_pPlayer->FireBullets3( m_pPlayer->GetGunPosition(), gpGlobals->v_forward, flSpread, 
        GLOCK18_DISTANCE, GLOCK18_PENETRATION, BULLET_PLAYER_9MM, GLOCK18_DAMAGE, GLOCK18_RANGE_MODIFER, m_pPlayer->pev, TRUE, m_pPlayer->random_seed );

    int flags;

    #if defined( CLIENT_WEAPONS )
        flags = FEV_NOTHOST;
    #else
        flags = 0;
    #endif

    PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usFireGlock18, 0, ( float* )&g_vecZero, ( float* )&g_vecZero, vecDir.x, vecDir.y, 
        ( int )( m_pPlayer->pev->punchangle.x * 100 ), ( int )( m_pPlayer->pev->punchangle.y * 100 ), m_iClip != 0, FALSE );

    m_flNextPrimaryAttack = m_flNextSecondaryAttack = GetNextAttackDelay( flCycleTime );

    if( !m_iClip && m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] <= 0 )
    {
        m_pPlayer->SetSuitUpdate( "!HEV_AMO0", FALSE, 0 );
    }

    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.5;

    if( fUseBurstMode )
    {
        m_iGlock18ShotsFired++;
        m_flGlock18Shoot = gpGlobals->time + 0.1;
    }

    ResetPlayerShieldAnim();
}

void CGLOCK18::Reload( void )
{
    if( m_pPlayer->ammo_9mm <= 0 )
    {
        return;
    }

    int animation;

    if( m_pPlayer->HasShield() )
        animation = GLOCK18_SHIELD_RELOAD;
    else
        animation = RANDOM_LONG( 0, 1 ) ? GLOCK18_RELOAD : GLOCK18_RELOAD2;

    if( DefaultReload( GLOCK18_MAX_CLIP, animation, GLOCK18_RELOAD_TIME ) )
    {
        m_pPlayer->SetAnimation( PLAYER_RELOAD );
        m_flAccuracy = 0.9;
    }
}

void CGLOCK18::WeaponIdle( void )
{
    ResetEmptySound();
    m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

    if( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
    {
        return;
    }

    if( m_pPlayer->HasShield() )
    {
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20.0;

        if( FBitSet( m_iWeaponState, WPNSTATE_SHIELD_DRAWN ) )
        {
            SendWeaponAnim( GLOCK18_SHIELD_IDLE, UseDecrement() != FALSE );
        }
    }
    else if( m_iClip )
    {
        float rand = RANDOM_FLOAT( 0, 1 );

        if( rand <= 0.3 )
        {
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0625;
            SendWeaponAnim( GLOCK18_IDLE3, UseDecrement() != FALSE );
        }
        else if( rand <= 0.6 )
        {
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.75;
            SendWeaponAnim( GLOCK18_IDLE1, UseDecrement() != FALSE );
        }
        else
        {
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.5;
            SendWeaponAnim( GLOCK18_IDLE2, UseDecrement() != FALSE );
        }
    }
}

BOOL CGLOCK18::UseDecrement( void )
{
    #if defined( CLIENT_WEAPONS )
        return TRUE;
    #else
        return FALSE;
    #endif
}

float CGLOCK18::GetMaxSpeed( void )
{
    return m_fMaxSpeed;
}

BOOL CGLOCK18::IsPistol( void )
{
    return TRUE;
}