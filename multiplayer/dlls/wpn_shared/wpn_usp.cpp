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

enum usp_e
{
    USP_IDLE,
    USP_SHOOT1,
    USP_SHOOT2,
    USP_SHOOT3,
    USP_SHOOT_EMPTY,
    USP_RELOAD,
    USP_DRAW,
    USP_ATTACH_SILENCER,
    USP_UNSIL_IDLE,
    USP_UNSIL_SHOOT1,
    USP_UNSIL_SHOOT2,
    USP_UNSIL_SHOOT3,
    USP_UNSIL_SHOOT_EMPTY,
    USP_UNSIL_RELOAD,
    USP_UNSIL_DRAW,
    USP_DETACH_SILENCER
};

enum usp_shield_e
{
    USP_SHIELD_IDLE,
    USP_SHIELD_SHOOT1,
    USP_SHIELD_SHOOT2,
    USP_SHIELD_SHOOT_EMPTY,
    USP_SHIELD_RELOAD,
    USP_SHIELD_DRAW,
    USP_SHIELD_UP_IDLE,
    USP_SHIELD_UP,
    USP_SHIELD_DOWN
};

#define USP_MAX_SPEED     250
#define USP_RELOAD_TIME   2.7

LINK_ENTITY_TO_CLASS( weapon_usp, CUSP );

void CUSP::Spawn( void )
{
    pev->classname = MAKE_STRING( "weapon_usp" );

    Precache();
    m_iId = WEAPON_USP;
    SET_MODEL( edict(), "models/w_usp.mdl" );

    ClearBits( m_iWeaponState, WPNSTATE_SHIELD_DRAWN );

    m_iDefaultAmmo = USP_DEFAULT_GIVE;
    m_flAccuracy   = 0.92;
    
    FallInit();
}

void CUSP::Precache( void )
{
    PRECACHE_MODEL( "models/v_usp.mdl" );
    PRECACHE_MODEL( "models/w_usp.mdl" );
    PRECACHE_MODEL( "models/shield/v_shield_usp.mdl" );

    PRECACHE_SOUND( "weapons/usp1.wav" );
    PRECACHE_SOUND( "weapons/usp2.wav" );
    PRECACHE_SOUND( "weapons/usp_unsil-1.wav" );
    PRECACHE_SOUND( "weapons/usp_clipout.wav" );
    PRECACHE_SOUND( "weapons/usp_clipin.wav" );
    PRECACHE_SOUND( "weapons/usp_silencer_on.wav" );
    PRECACHE_SOUND( "weapons/usp_silencer_off.wav" );
    PRECACHE_SOUND( "weapons/usp_sliderelease.wav" );
    PRECACHE_SOUND( "weapons/usp_slideback.wav" );

    m_iShell = PRECACHE_MODEL( "models/pshell.mdl" );
    m_usFireUSP = PRECACHE_EVENT(1, "events/usp.sc" );
}

int CUSP::GetItemInfo( ItemInfo *p )
{
    p->pszName   = STRING( pev->classname );
    p->pszAmmo1  = "45acp";
    p->iMaxAmmo1 = _45ACP_MAX_CARRY;
    p->pszAmmo2  = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip  = USP_MAX_CLIP;
    p->iSlot     = 1;
    p->iPosition = 4;
    p->iId       = m_iId = WEAPON_USP;
    p->iFlags    = 0;
    p->iWeight   = USP_WEIGHT;

    return 1;
}

int CUSP::iItemSlot( void )
{
    return PISTOL_SLOT;
}

BOOL CUSP::Deploy( void )
{
    ClearBits( m_iWeaponState, WPNSTATE_SHIELD_DRAWN );

    m_flAccuracy = 0.92;
    m_fMaxSpeed  = USP_MAX_SPEED;

    m_pPlayer->m_bShieldDrawn = false;

    if( m_pPlayer->HasShield() )
        return DefaultDeploy( "models/shield/v_shield_usp.mdl", "models/shield/p_shield_usp.mdl", USP_SHIELD_DRAW, "shieldgun", UseDecrement() != FALSE );
    else
        return DefaultDeploy( "models/v_usp.mdl", "models/p_usp.mdl", FBitSet( m_iWeaponState, WPNSTATE_USP_SILENCED ) ? USP_DRAW : USP_UNSIL_DRAW, "onehanded", UseDecrement() != FALSE );
}

void CUSP::PrimaryAttack( void )
{
    if( FBitSet( m_iWeaponState, WPNSTATE_USP_SILENCED ) )
    {
        if( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
        {
            USPFire( 1.3 * ( 1 - m_flAccuracy ), 0.225, FALSE );
        }
        else if( m_pPlayer->pev->velocity.Length2D() > 0 )
        {
            USPFire( 0.25 * ( 1 - m_flAccuracy ), 0.225, FALSE );
        }
        else if( FBitSet( m_pPlayer->pev->flags, FL_DUCKING ) )
        {
            USPFire( 0.125 * ( 1 - m_flAccuracy ), 0.225, FALSE );
        }
        else
        {
            USPFire( 0.15 * ( 1 - m_flAccuracy ), 0.225, FALSE );
        }
    }
    else
    {
        if( !FBitSet( m_pPlayer->pev->flags, FL_ONGROUND ) )
        {
            USPFire( 1.2 * ( 1  - m_flAccuracy ), 0.225, FALSE );
        }
        else if( m_pPlayer->pev->velocity.Length2D() > 0 )
        {
            USPFire( 0.225 * ( 1 - m_flAccuracy ), 0.225, FALSE );
        }
        else if( FBitSet( m_pPlayer->pev->flags, FL_DUCKING ) )
        {
            USPFire( 0.08 * ( 1 - m_flAccuracy ), 0.225, FALSE );
        }
        else
        {
            USPFire( 0.1 * ( 1 - m_flAccuracy ), 0.225, FALSE );
        }
    }
}

void CUSP::SecondaryAttack( void )
{
    if( ShieldSecondaryFire( USP_SHIELD_UP, USP_SHIELD_DOWN ) )
    {
        return;
    }

    if( FBitSet( m_iWeaponState, WPNSTATE_USP_SILENCED ) )
    {
        ClearBits( m_iWeaponState, WPNSTATE_USP_SILENCED );

        SendWeaponAnim(USP_DETACH_SILENCER, UseDecrement() != FALSE );
        strcpy( m_pPlayer->m_szAnimExtention, "onehanded" );
    }
    else
    {
        SetBits( m_iWeaponState, WPNSTATE_USP_SILENCED );

        SendWeaponAnim( USP_ATTACH_SILENCER, UseDecrement() != FALSE );
        strcpy( m_pPlayer->m_szAnimExtention, "onehanded" );
    }

    m_flTimeWeaponIdle      = UTIL_WeaponTimeBase() + 3.0;
    m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 3.0;
    m_flNextPrimaryAttack   = GetNextAttackDelay( 3.0 );
}

void CUSP::USPFire( float flSpread, float flCycleTime, BOOL fUseAutoAim )
{
    m_iShotsFired++;

    if( m_iShotsFired > 1 )
    {
        return;
    }

    if( m_flLastFire )
    {
        m_flAccuracy -= ( 0.3 - ( gpGlobals->time - m_flLastFire ) ) * 0.275;

        if( m_flAccuracy > 0.92 )
        {
            m_flAccuracy = 0.92;
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

    flCycleTime -= 0.075;
    m_flNextPrimaryAttack = m_flNextSecondaryAttack = GetNextAttackDelay( flCycleTime );

    m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

    m_pPlayer->m_iWeaponVolume = BIG_EXPLOSION_VOLUME;
    m_pPlayer->m_iWeaponFlash  = DIM_GUN_FLASH;

    UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );

    if( !FBitSet( m_iWeaponState, WPNSTATE_USP_SILENCED ) )
    {
        m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
    }

    int damage = FBitSet( m_iWeaponState, WPNSTATE_USP_SILENCED ) ? USP_DAMAGE_SIL : USP_DAMAGE;

    Vector vecDir = FireBullets3( m_pPlayer->GetGunPosition(), gpGlobals->v_forward, flSpread, 
        USP_DISTANCE, USP_PENETRATION, BULLET_PLAYER_45ACP, damage, USP_RANGE_MODIFER, m_pPlayer->pev, TRUE, m_pPlayer->random_seed );

    int flags;

    #if defined( CLIENT_WEAPONS )
        flags = FEV_NOTHOST;
    #else
        flags = 0;
    #endif

    PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usFireUSP, 0, ( float* )&g_vecZero,  (float* )&g_vecZero, vecDir.x, vecDir.y, 
        ( int )( m_pPlayer->pev->punchangle.x * 100 ), 0, m_iClip != 0, FBitSet( m_iWeaponState, WPNSTATE_USP_SILENCED ) );
    
    if( !m_iClip && m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] <= 0 )
    {
        m_pPlayer->SetSuitUpdate( "!HEV_AMO0", FALSE, 0 );
    }

    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0;
    m_pPlayer->pev->punchangle.x -= 2;

    ResetPlayerShieldAnim();
}

void CUSP::Reload( void )
{
    if( m_pPlayer->ammo_45acp <= 0 )
    {
        return;
    }

    int anim;

    if( m_pPlayer->HasShield() )
    {
        anim = USP_SHIELD_RELOAD;
    }
    else if( FBitSet( m_iWeaponState, WPNSTATE_USP_SILENCED ) )
    {
        anim = USP_RELOAD;
    }
    else
    {
        anim = USP_UNSIL_RELOAD;
    }

    if( DefaultReload( USP_MAX_CLIP, anim, USP_RELOAD_TIME ) )
    {
        m_pPlayer->SetAnimation( PLAYER_RELOAD );
        m_flAccuracy = 0.92;
    }
}

void CUSP::WeaponIdle( void )
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
            SendWeaponAnim( USP_DRAW, UseDecrement() != FALSE );
        }
    }
    else if( m_iClip )
    {
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 60.0;
        SendWeaponAnim( FBitSet( m_iWeaponState, WPNSTATE_USP_SILENCED ) ? USP_IDLE : USP_UNSIL_IDLE, UseDecrement() != FALSE );
    }
}

BOOL CUSP::UseDecrement( void )
{
    #if defined( CLIENT_WEAPONS )
        return TRUE;
    #else
        return FALSE;
    #endif
}

float CUSP::GetMaxSpeed( void )
{
    return m_fMaxSpeed;
}

BOOL CUSP::IsPistol( void )
{
    return TRUE;
}
