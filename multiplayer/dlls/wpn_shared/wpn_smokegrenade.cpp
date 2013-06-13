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

enum smokegrenade_e
{
    SMOKEGRENADE_IDLE,
    SMOKEGRENADE_PINPULL,
    SMOKEGRENADE_THROW,
    SMOKEGRENADE_DRAW
};

#define SMOKEGRENADE_MAX_SPEED         250
#define SMOKEGRENADE_MAX_SPEED_SHIELD  180

LINK_ENTITY_TO_CLASS( weapon_smokegrenade, CSmokeGrenade );

void CSmokeGrenade::Spawn( void )
{
    pev->classname = MAKE_STRING( "weapon_smokegrenade" );

    Precache();
    m_iId = WEAPON_SMOKEGRENADE;
    SET_MODEL( edict(), "models/w_smokegrenade.mdl" );

    pev->dmg = 4;

    ClearBits( m_iWeaponState, WPNSTATE_SHIELD_DRAWN );

    m_iDefaultAmmo   = SMOKEGRENADE_DEFAULT_GIVE;
    m_flStartThrow   = 0;
    m_flReleaseThrow = -1;

    FallInit();
}

void CSmokeGrenade::Precache( void )
{
    PRECACHE_MODEL( "models/v_smokegrenade.mdl" );
    PRECACHE_MODEL( "models/shield/v_shield_smokegrenade.mdl" );

    PRECACHE_SOUND( "weapons/pinpull.wav" );
    PRECACHE_SOUND( "weapons/sg_explode.wav" );

    m_usCreateSmoke = PRECACHE_EVENT( 1, "events/createsmoke.sc" );
}

int CSmokeGrenade::GetItemInfo( ItemInfo *p )
{
    p->pszName   = STRING( pev->classname );
    p->pszAmmo1  = "SmokeGrenade";
    p->iMaxAmmo1 = SMOKEGRENADE_MAX_CARRY;
    p->pszAmmo2  = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip  = WEAPON_NOCLIP;
    p->iSlot     = 3;
    p->iPosition = 3;
    p->iId       = m_iId = WEAPON_SMOKEGRENADE;
    p->iWeight   = SMOKEGRENADE_WEIGHT;
    p->iFlags    = ITEM_FLAG_LIMITINWORLD | ITEM_FLAG_EXHAUSTIBLE;

    return 1;
}

int CSmokeGrenade::iItemSlot( void )
{
    return GRENADE_SLOT;
}

BOOL CSmokeGrenade::Deploy( void )
{
    ClearBits( m_iWeaponState, WPNSTATE_SHIELD_DRAWN );

    m_flReleaseThrow = -1;
    m_fMaxSpeed      = SMOKEGRENADE_MAX_SPEED;

    m_pPlayer->m_bShieldDrawn = false;

    if( m_pPlayer->HasShield() )
        return DefaultDeploy( "models/shield/v_shield_smokegrenade.mdl", "models/shield/p_shield_smokegrenade.mdl", SMOKEGRENADE_DRAW, "shieldgren", UseDecrement() != FALSE );
    else
        return DefaultDeploy( "models/v_smokegrenade.mdl", "models/p_smokegrenade.mdl", SMOKEGRENADE_DRAW, "grenade", UseDecrement() != FALSE );
}

void CSmokeGrenade::Holster( int skiplocal )
{
    m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;

    if( !m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] )
    {
        ClearBits( m_pPlayer->pev->weapons, 1 << WEAPON_SMOKEGRENADE );
        DestroyItem();
    }

    m_flStartThrow   = 0;
    m_flReleaseThrow = -1;
}

void CSmokeGrenade::PrimaryAttack( void )
{
    if( FBitSet( m_iWeaponState, WPNSTATE_SHIELD_DRAWN ) )
    {
        return;
    }

    if( !m_flStartThrow && m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] > 0 )
    {
        m_flStartThrow   = gpGlobals->time;
        m_flReleaseThrow = 0;

        SendWeaponAnim( SMOKEGRENADE_PINPULL, UseDecrement() != FALSE );

        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5;
    }
}
 
void CSmokeGrenade::SecondaryAttack( void )
{
    ShieldSecondaryFire( SHIELDGUN_DRAW, SHIELDGUN_DRAWN_IDLE );
}

void CSmokeGrenade::WeaponIdle( void )
{
    if( !m_flReleaseThrow && m_flStartThrow )
    {
        m_flReleaseThrow = gpGlobals->time;
    }

    if( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
    {
        return;
    }

    if( m_flStartThrow )
    {
        m_pPlayer->Radio( "%!MRAD_FIREINHOLE", "#Fire_in_the_hole" );
        Vector angleThrow = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;

        if( angleThrow.x < 0 )
            angleThrow.x = -10 + angleThrow.x * ( ( 90 - 10 ) / 90.0 );
        else
            angleThrow.x = -10 + angleThrow.x * ( ( 90 + 10 ) / 90.0 );

        float flVel = ( 90 - angleThrow.x ) * 6;

        if( flVel > 750 )
            flVel = 750;

        UTIL_MakeVectors( angleThrow );

        Vector vecSrc   = m_pPlayer->pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_forward * 16;
        Vector vecThrow = gpGlobals->v_forward * flVel + m_pPlayer->pev->velocity;

        CGrenade::ShootSmokeGrenade( m_pPlayer->pev, vecSrc, vecThrow, 1.5, m_usCreateSmoke );

        SendWeaponAnim( SMOKEGRENADE_THROW, UseDecrement() != FALSE );
        SetPlayerShieldAnim();

        m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

        m_flStartThrow        = 0;
        m_flNextPrimaryAttack = GetNextAttackDelay( 0.5 );
        m_flTimeWeaponIdle    = UTIL_WeaponTimeBase() + 0.75;

        m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ]--;

        if( !m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] )
        {
            m_flTimeWeaponIdle = m_flNextSecondaryAttack = m_flNextPrimaryAttack = GetNextAttackDelay( 0.5 );
        }

        ResetPlayerShieldAnim();
    }
    else if( m_flReleaseThrow > 0 )
    {
        m_flStartThrow = 0;

        if( m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] )
        {
            SendWeaponAnim( SMOKEGRENADE_DRAW, UseDecrement() != FALSE );

            m_flReleaseThrow   = -1;
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + RANDOM_FLOAT( 10, 15 );
        }
        else
        {
            RetireWeapon();
        }
    }
    else if( m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] )
    {
        if( m_pPlayer->HasShield() )
        {
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20.0;

            if( FBitSet( m_iWeaponState, WPNSTATE_SHIELD_DRAWN ) )
            {
                SendWeaponAnim( SHIELDREN_IDLE, UseDecrement() != FALSE );
            }
        }
        else
        {
            if( RANDOM_FLOAT(0, 1 ) <= 0.75 )
            {
                SendWeaponAnim( SMOKEGRENADE_IDLE, UseDecrement() != FALSE );
                m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + RANDOM_FLOAT( 10, 15 );
            }
            else
            {
                SendWeaponAnim( SMOKEGRENADE_IDLE, UseDecrement() != FALSE );
                m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 75.0 / 30.0;
            }
        }
    }
}

void CSmokeGrenade::SetPlayerShieldAnim( void )
{
    if( m_pPlayer->HasShield() )
    {
        if( FBitSet( m_iWeaponState, WPNSTATE_SHIELD_DRAWN ) )
        {
            strcpy( m_pPlayer->m_szAnimExtention, "shield" );
        }
        else
        {
            strcpy( m_pPlayer->m_szAnimExtention, "shieldgren" );
        }
    }
}

bool CSmokeGrenade::ShieldSecondaryFire( int up_anim, int down_anim )
{
    if( m_pPlayer->HasShield() || m_flStartThrow > 0 )
    {
        return false;
    }

    if( FBitSet( m_iWeaponState, WPNSTATE_SHIELD_DRAWN ) )
    {
        ClearBits( m_iWeaponState, WPNSTATE_SHIELD_DRAWN );

        SendWeaponAnim( down_anim, UseDecrement() != FALSE );

        strcpy( m_pPlayer->m_szAnimExtention, "shieldgren" );

        m_fMaxSpeed = SMOKEGRENADE_MAX_SPEED;
        m_pPlayer->m_bShieldDrawn = false;
    }
    else
    {
        SetBits( m_iWeaponState, WPNSTATE_SHIELD_DRAWN );

        SendWeaponAnim( up_anim, UseDecrement() != FALSE );

        strcpy( m_pPlayer->m_szAnimExtention, "shielded" );

        m_fMaxSpeed = SMOKEGRENADE_MAX_SPEED_SHIELD;
        m_pPlayer->m_bShieldDrawn = true;
    }

    m_pPlayer->UpdateShieldCrosshair( FBitSet( m_iWeaponState, WPNSTATE_SHIELD_DRAWN ) ? true : false );
    m_pPlayer->ResetMaxSpeed();

    m_flTimeWeaponIdle      = UTIL_WeaponTimeBase() + 0.6;
    m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.4;
    m_flNextPrimaryAttack   = GetNextAttackDelay( 0.4 );

    return true;
}

void CSmokeGrenade::ResetPlayerShieldAnim( void )
{
    if( m_pPlayer->HasShield() )
    {
        if( FBitSet( m_iWeaponState, WPNSTATE_SHIELD_DRAWN ) )
        {
            strcpy( m_pPlayer->m_szAnimExtention, "shieldgren" );
        }
    }
}

BOOL CSmokeGrenade::CanDeploy( void )
{
    return m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] != 0;
}

BOOL CSmokeGrenade::CanDrop( void )
{
    return FALSE;
}

BOOL CSmokeGrenade::UseDecrement( void )
{
    #if defined( CLIENT_WEAPONS )
        return TRUE;
    #else
        return FALSE;
    #endif
}
