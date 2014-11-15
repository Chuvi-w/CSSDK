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

enum hegrenade_e
{
    HEGRENADE_IDLE,
    HEGRENADE_PULLPIN,
    HEGRENADE_THROW,
    HEGRENADE_DRAW
};

#define HEGRENADE_MAX_SPEED         250
#define HEGRENADE_MAX_SPEED_SHIELD  180

LINK_ENTITY_TO_CLASS( weapon_hegrenade, CHEGrenade );

void CHEGrenade::Spawn( void )
{
    Precache();
    m_iId = WEAPON_HEGRENADE;
    SET_MODEL( edict(), "models/w_hegrenade.mdl" );

    pev->dmg = 4;

    ClearBits( m_iWeaponState, WPNSTATE_SHIELD_DRAWN );

    m_iDefaultAmmo   = HEGRENADE_DEFAULT_GIVE;
    m_flStartThrow   = 0;
    m_flReleaseThrow = -1;

    FallInit();
}

void CHEGrenade::Precache( void )
{
    PRECACHE_MODEL( "models/v_hegrenade.mdl" );
    PRECACHE_MODEL( "models/shield/v_shield_hegrenade.mdl" );

    PRECACHE_SOUND( "weapons/hegrenade-1.wav" );
    PRECACHE_SOUND( "weapons/hegrenade-2.wav" );
    PRECACHE_SOUND( "weapons/he_bounce-1.wav" );
    PRECACHE_SOUND( "weapons/pinpull.wav" );

    m_usCreateExplosion = PRECACHE_EVENT( 1, "events/createexplo.sc" );
}

int CHEGrenade::GetItemInfo( ItemInfo *p )
{
    p->pszName   = STRING( pev->classname );
    p->pszAmmo1  = "HEGrenade";
    p->iMaxAmmo1 = HEGRENADE_MAX_CARRY;
    p->pszAmmo2  = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip  = WEAPON_NOCLIP;
    p->iSlot     = 3;
    p->iPosition = 1;
    p->iId       = m_iId = WEAPON_HEGRENADE;
    p->iWeight   = HEGRENADE_WEIGHT;
    p->iFlags    = ITEM_FLAG_LIMITINWORLD | ITEM_FLAG_EXHAUSTIBLE;

    return 1;
}

int CHEGrenade::iItemSlot( void )
{
    return GRENADE_SLOT;
}

BOOL CHEGrenade::Deploy( void )
{
    ClearBits( m_iWeaponState, WPNSTATE_SHIELD_DRAWN );

    m_flReleaseThrow = -1;
    m_fMaxSpeed      = HEGRENADE_MAX_SPEED;

    m_pPlayer->m_bShieldDrawn = false;

    if( m_pPlayer->HasShield() )
        return DefaultDeploy( "models/shield/v_shield_hegrenade.mdl", "models/shield/p_shield_hegrenade.mdl", HEGRENADE_DRAW, "shieldgren", UseDecrement() != FALSE );
    else
        return DefaultDeploy( "models/v_hegrenade.mdl", "models/p_hegrenade.mdl", HEGRENADE_DRAW, "grenade", UseDecrement() != FALSE );
}

void CHEGrenade::Holster( int skiplocal )
{
    m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;

    if( !m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] )
    {
        ClearBits( m_pPlayer->pev->weapons, 1 << WEAPON_HEGRENADE );
        DestroyItem();
    }

    m_flStartThrow   = 0;
    m_flReleaseThrow = -1;
}

void CHEGrenade::PrimaryAttack( void )
{
    if( FBitSet( m_iWeaponState, WPNSTATE_SHIELD_DRAWN ) )
    {   
        return;
    }

    if( !m_flStartThrow && m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] > 0 )
    {
        m_flStartThrow   = gpGlobals->time;
        m_flReleaseThrow = 0;

        SendWeaponAnim( HEGRENADE_PULLPIN, UseDecrement() != FALSE );

        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5;
    }
}

void CHEGrenade::SecondaryAttack( void )
{
    ShieldSecondaryFire( SHIELDGUN_DRAW, SHIELDGUN_DRAWN_IDLE );
}

void CHEGrenade::WeaponIdle( void )
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
        m_pPlayer->Radio( "%!MRAD_FIREINHOLE", "#Fire_in_the_hole", PITCH_NORM, true );

        Vector angleThrow = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;

        if( angleThrow.x < 0 )
            angleThrow.x = -10 + angleThrow.x * ( ( 90 - 10 ) / 90.0 );
        else
            angleThrow.x = -10 + angleThrow.x * ( ( 90 + 10 ) / 90.0 );

        float vel = ( 90 - angleThrow.x ) * 6;

        if( vel > 750 )
            vel = 750;

        UTIL_MakeVectors( angleThrow );

        Vector vecSrc   = m_pPlayer->pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_forward * 16;
        Vector vecThrow = gpGlobals->v_forward * vel + m_pPlayer->pev->velocity;

        CGrenade::ShootTimed2( m_pPlayer->pev, vecSrc, vecThrow, 1.5, m_pPlayer->m_iTeam, m_usCreateExplosion );

        SendWeaponAnim( HEGRENADE_THROW, UseDecrement() != FALSE );
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
            SendWeaponAnim( HEGRENADE_DRAW, UseDecrement() != FALSE );

            m_flReleaseThrow   = -1;
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + RANDOM_FLOAT( 10, 15 );
        }
        else
            RetireWeapon();
    }
    else if( m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] )
    {
        if( m_pPlayer->HasShield() )
        {
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20.0;

            if( FBitSet( m_iWeaponState, WPNSTATE_SHIELD_DRAWN ) )
            {
                SendWeaponAnim(SHIELDREN_IDLE, UseDecrement() != FALSE);
            }
        }
        else
        {
            SendWeaponAnim(HEGRENADE_IDLE, UseDecrement() != FALSE);
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + RANDOM_FLOAT( 10, 15 );
        }
    }
}

void CHEGrenade::SetPlayerShieldAnim( void )
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

bool CHEGrenade::ShieldSecondaryFire( int up_anim, int down_anim )
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

        m_fMaxSpeed = HEGRENADE_MAX_SPEED;
        m_pPlayer->m_bShieldDrawn = false;
    }
    else
    {
        SetBits( m_iWeaponState, WPNSTATE_SHIELD_DRAWN );

        SendWeaponAnim( up_anim, UseDecrement() != FALSE );

        strcpy( m_pPlayer->m_szAnimExtention, "shielded" );

        m_fMaxSpeed = HEGRENADE_MAX_SPEED_SHIELD;
        m_pPlayer->m_bShieldDrawn = true;
    }

    m_pPlayer->UpdateShieldCrosshair( FBitSet( m_iWeaponState, WPNSTATE_SHIELD_DRAWN ) ? true : false );
    m_pPlayer->ResetMaxSpeed();

    m_flTimeWeaponIdle      = UTIL_WeaponTimeBase() + 0.6;
    m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.4;
    m_flNextPrimaryAttack   = GetNextAttackDelay( 0.4 );

    return true;
}

void CHEGrenade::ResetPlayerShieldAnim( void )
{
    if( m_pPlayer->HasShield() )
    {
        if( FBitSet( m_iWeaponState, WPNSTATE_SHIELD_DRAWN ) )
        {
            strcpy( m_pPlayer->m_szAnimExtention, "shieldgren" );
        }
    }
}

float CHEGrenade::GetMaxSpeed( void )
{
    return m_fMaxSpeed;
}

BOOL CHEGrenade::CanDeploy( void )
{
    return m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] != 0;
}

BOOL CHEGrenade::CanDrop( void )
{
    return FALSE;
}

BOOL CHEGrenade::UseDecrement( void )
{
    #if defined( CLIENT_WEAPONS )
        return TRUE;
    #else
        return FALSE;
    #endif
}
