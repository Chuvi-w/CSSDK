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

enum flashbang_e
{
    FLASHBANG_IDLE,
    FLASHBANG_PULLPIN,
    FLASHBANG_THROW,
    FLASHBANG_DRAW
};

#define FLASHBANG_MAX_SPEED         250
#define FLASHBANG_MAX_SPEED_SHIELD  180

LINK_ENTITY_TO_CLASS( weapon_flashbang, CFlashbang );

void CFlashbang::Spawn( void )
{
    pev->classname = MAKE_STRING( "weapon_flashbang" );

    Precache();
    m_iId = WEAPON_FLASHBANG;
    SET_MODEL( edict(), "models/w_flashbang.mdl" );

    pev->dmg = 4;

    m_iDefaultAmmo   = FLASHBANG_DEFAULT_GIVE;
    m_flStartThrow   = 0;
    m_flReleaseThrow = -1.0;

    ClearBits( m_iWeaponState, WPNSTATE_SHIELD_DRAWN );

    FallInit();
}

void CFlashbang::Precache( void )
{
    PRECACHE_MODEL( "models/v_flashbang.mdl" );
    PRECACHE_MODEL( "models/shield/v_shield_flashbang.mdl" );

    PRECACHE_SOUND( "weapons/flashbang-1.wav" );
    PRECACHE_SOUND( "weapons/flashbang-2.wav" );
    PRECACHE_SOUND( "weapons/pinpull.wav" );
}

int CFlashbang::GetItemInfo( ItemInfo *p )
{
    p->pszName   = STRING( pev->classname );
    p->pszAmmo1  = "Flashbang";
    p->iMaxAmmo1 = FLASHBANG_MAX_CARRY;
    p->pszAmmo2  = NULL;
    p->iMaxAmmo2 = -1;
    p->iMaxClip  = WEAPON_NOCLIP;
    p->iSlot     = 3;
    p->iPosition = 2;
    p->iId       = m_iId = WEAPON_FLASHBANG;
    p->iWeight   = FLASHBANG_WEIGHT;
    p->iFlags    = ITEM_FLAG_LIMITINWORLD | ITEM_FLAG_EXHAUSTIBLE;

    return 1;
}

int CFlashbang::iItemSlot( void )
{
    return GRENADE_SLOT;
}

BOOL CFlashbang::Deploy( void )
{
    m_flReleaseThrow = -1.0;
    m_fMaxSpeed      = FLASHBANG_MAX_SPEED;

    ClearBits( m_iWeaponState, WPNSTATE_SHIELD_DRAWN );

    m_pPlayer->m_bShieldDrawn = false;

    if( m_pPlayer->HasShield() )
        return DefaultDeploy( "models/shield/v_shield_flashbang.mdl", "models/shield/p_shield_flashbang.mdl", FLASHBANG_DRAW, "shieldgren", UseDecrement() != FALSE );
    else
        return DefaultDeploy( "models/v_flashbang.mdl", "models/p_flashbang.mdl", FLASHBANG_DRAW, "grenade", UseDecrement() != FALSE);
}

void CFlashbang::Holster( int skiplocal )
{
    m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;

    if( !m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] )
    {
        ClearBits( m_pPlayer->pev->weapons, 1 << WEAPON_FLASHBANG );
        DestroyItem();
    }

    m_flStartThrow   = 0;
    m_flReleaseThrow = -1.0;
}

void CFlashbang::PrimaryAttack( void )
{
    if( FBitSet( m_iWeaponState, WPNSTATE_SHIELD_DRAWN ) )
    {
        return;
    }

    if( !m_flStartThrow && m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] > 0 )
    {
        m_flStartThrow   = gpGlobals->time;
        m_flReleaseThrow = 0;

        SendWeaponAnim( FLASHBANG_PULLPIN, UseDecrement() != FALSE );

        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5;
    }
}

void CFlashbang::SecondaryAttack( void )
{
    ShieldSecondaryFire( SHIELDGUN_DRAW, SHIELDGUN_DRAWN_IDLE );
}

void CFlashbang::WeaponIdle( void )
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

        CGrenade::ShootTimed( m_pPlayer->pev, vecSrc, vecThrow, 1.5 );

        SendWeaponAnim( FLASHBANG_THROW, UseDecrement() != FALSE );
        SetPlayerShieldAnim();

        m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

        m_flStartThrow        = 0;
        m_flNextPrimaryAttack = GetNextAttackDelay( 0.5 );
        m_flTimeWeaponIdle    = UTIL_WeaponTimeBase() + 0.75;

        m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ]--;

        if( !m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
        {
            m_flTimeWeaponIdle = m_flNextSecondaryAttack = m_flNextPrimaryAttack = GetNextAttackDelay( 0.5 );
        }

        ResetPlayerShieldAnim();
    }
    else if( m_flReleaseThrow > 0 )
    {
        m_flStartThrow = 0;
        RetireWeapon();
    }
    else if( m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] )
    {
        float rand = RANDOM_FLOAT( 0, 1 );

        if( m_pPlayer->HasShield() )
        {
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20.0;

            if(FBitSet( m_iWeaponState, WPNSTATE_SHIELD_DRAWN ) )
            {
                SendWeaponAnim( SHIELDREN_IDLE, UseDecrement() != FALSE);
            }
        }
        else
        {
            if( rand > 0.75 )
                m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.5;
            else
                m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + RANDOM_FLOAT( 10, 15 );

            SendWeaponAnim( FLASHBANG_IDLE, UseDecrement() != FALSE );
        }
    }
}

void CFlashbang::SetPlayerShieldAnim( void )
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

bool CFlashbang::ShieldSecondaryFire( int up_anim, int down_anim )
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

        m_fMaxSpeed = FLASHBANG_MAX_SPEED;
        m_pPlayer->m_bShieldDrawn = false;
    }
    else
    {
        SetBits( m_iWeaponState, WPNSTATE_SHIELD_DRAWN );

        SendWeaponAnim( up_anim, UseDecrement() != FALSE );

        strcpy( m_pPlayer->m_szAnimExtention, "shielded" );

        m_fMaxSpeed = FLASHBANG_MAX_SPEED_SHIELD;
        m_pPlayer->m_bShieldDrawn = true;
    }

    m_pPlayer->UpdateShieldCrosshair( FBitSet( m_iWeaponState, WPNSTATE_SHIELD_DRAWN ) ? true : false );
    m_pPlayer->ResetMaxSpeed();

    m_flTimeWeaponIdle      = UTIL_WeaponTimeBase() + 0.6;
    m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.4;
    m_flNextPrimaryAttack   = GetNextAttackDelay( 0.4 );
    
    return true;
}

void CFlashbang::ResetPlayerShieldAnim( void )
{
    if( m_pPlayer->HasShield() )
    {
        if( FBitSet( m_iWeaponState, WPNSTATE_SHIELD_DRAWN ) )
        {
            strcpy( m_pPlayer->m_szAnimExtention, "shieldgren" );
        }
    }
}

float CFlashbang::GetMaxSpeed( void )
{
    return m_fMaxSpeed;
}

BOOL CFlashbang::CanDeploy( void )
{
    return m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] != 0;
}

BOOL CFlashbang::CanDrop( void )
{
    return FALSE;
}

BOOL CFlashbang::UseDecrement( void )
{
    #if defined( CLIENT_WEAPONS )
        return TRUE;
    #else
        return FALSE;
    #endif
}

BOOL CFlashbang::IsPistol( void )
{
    return TRUE;
}
