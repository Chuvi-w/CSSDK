/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
/*

===== generic grenade.cpp ========================================================

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "soundent.h"
#include "decals.h"
#include "gamerules.h"
#include "player.h"
#include "hltv.h"
#include "career_tasks.h"


LINK_ENTITY_TO_CLASS( grenade, CGrenade );

TYPEDESCRIPTION	CGrenade::m_SaveData[] = 
{
	DEFINE_FIELD( CGrenade, m_flNextFreqInterval, FIELD_TIME	),
	DEFINE_FIELD( CGrenade, m_flNextBeep		, FIELD_TIME	),
	DEFINE_FIELD( CGrenade, m_flNextFreq		, FIELD_TIME	),
	DEFINE_FIELD( CGrenade, m_sBeepName			, FIELD_STRING	),
	DEFINE_FIELD( CGrenade, m_fAttenu			, FIELD_FLOAT	),
	DEFINE_FIELD( CGrenade, m_flNextBlink		, FIELD_TIME	),
	DEFINE_FIELD( CGrenade, m_bJustBlew			, FIELD_BOOLEAN ),
	DEFINE_FIELD( CGrenade, m_pentCurBombTarget	, FIELD_ENTITY	),
	DEFINE_FIELD( CGrenade, m_SGSmoke			, FIELD_INTEGER ),
	DEFINE_FIELD( CGrenade, m_bLightSmoke		, FIELD_BOOLEAN ),
	DEFINE_FIELD( CGrenade, m_usEvent			, FIELD_SHORT	),
};

IMPLEMENT_SAVERESTORE( CGrenade, CBaseEntity );


// Grenades flagged with this will be triggered when the owner calls detonateSatchelCharges
#define SF_DETONATE		0x0001

// CS
extern int gmsgScenarioIcon;
extern int gmsgBombPickup;

//
// Grenade Explode
//
void CGrenade::Explode( Vector vecSrc, Vector vecAim )
{
	TraceResult tr;
	UTIL_TraceLine ( pev->origin, pev->origin + Vector ( 0, 0, -32 ),  ignore_monsters, ENT(pev), & tr);

	Explode( &tr, DMG_BLAST );
}

// UNDONE: temporary scorching for PreAlpha - find a less sleazy permenant solution.
void CGrenade::Explode( TraceResult *pTrace, int bitsDamageType )
{
	float		flRndSound;// sound randomizer

	pev->model = iStringNull;//invisible
	pev->solid = SOLID_NOT;// intangible

	pev->takedamage = DAMAGE_NO;

	// Pull out of the wall a bit
	if ( pTrace->flFraction != 1.0 )
	{
		pev->origin = pTrace->vecEndPos + (pTrace->vecPlaneNormal * (pev->dmg - 24) * 0.6);
	}

	int iContents = UTIL_PointContents ( pev->origin );
	
	MESSAGE_BEGIN( MSG_PAS, SVC_TEMPENTITY, pev->origin );
		WRITE_BYTE( TE_EXPLOSION );		// This makes a dynamic light and the explosion sprites/sound
		WRITE_COORD( pev->origin.x );	// Send to PAS because of the sound
		WRITE_COORD( pev->origin.y );
		WRITE_COORD( pev->origin.z );
		if (iContents != CONTENTS_WATER)
		{
			WRITE_SHORT( g_sModelIndexFireball );
		}
		else
		{
			WRITE_SHORT( g_sModelIndexWExplosion );
		}
		WRITE_BYTE( static_cast<int>((pev->dmg - 50) * .60)  ); // scale * 10
		WRITE_BYTE( 15  ); // framerate
		WRITE_BYTE( TE_EXPLFLAG_NONE );
	MESSAGE_END();

	CSoundEnt::InsertSound ( bits_SOUND_COMBAT, pev->origin, NORMAL_EXPLOSION_VOLUME, 3.0 );
	entvars_t *pevOwner;
	if ( pev->owner )
		pevOwner = VARS( pev->owner );
	else
		pevOwner = NULL;

	pev->owner = NULL; // can't traceline attack owner if this is set

	RadiusDamage ( pev, pevOwner, pev->dmg, CLASS_NONE, bitsDamageType );

	if ( RANDOM_FLOAT( 0 , 1 ) < 0.5 )
	{
		UTIL_DecalTrace( pTrace, DECAL_SCORCH1 );
	}
	else
	{
		UTIL_DecalTrace( pTrace, DECAL_SCORCH2 );
	}

	flRndSound = RANDOM_FLOAT( 0 , 1 );

	switch ( RANDOM_LONG( 0, 2 ) )
	{
		case 0:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/debris1.wav", 0.55, ATTN_NORM);	break;
		case 1:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/debris2.wav", 0.55, ATTN_NORM);	break;
		case 2:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/debris3.wav", 0.55, ATTN_NORM);	break;
	}

	pev->effects |= EF_NODRAW;
	SetThink( &CGrenade::Smoke );
	pev->velocity = g_vecZero;
	pev->nextthink = gpGlobals->time + 0.3;

	if (iContents != CONTENTS_WATER)
	{
		int sparkCount = RANDOM_LONG(0,3);
		for ( int i = 0; i < sparkCount; i++ )
			Create( "spark_shower", pev->origin, pTrace->vecPlaneNormal, NULL );
	}
}


void CGrenade::Smoke( void )
{
	if (UTIL_PointContents ( pev->origin ) == CONTENTS_WATER)
	{
		UTIL_Bubbles( pev->origin - Vector( 64, 64, 64 ), pev->origin + Vector( 64, 64, 64 ), 100 );
	}
	else
	{
		MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, pev->origin );
			WRITE_BYTE( TE_SMOKE );
			WRITE_COORD( pev->origin.x );
			WRITE_COORD( pev->origin.y );
			WRITE_COORD( pev->origin.z );
			WRITE_SHORT( g_sModelIndexSmoke );
			WRITE_BYTE( static_cast<int>((pev->dmg - 50) * 0.80) ); // scale * 10
			WRITE_BYTE( 12  ); // framerate
		MESSAGE_END();
	}
	UTIL_Remove( this );
}

void CGrenade::Killed( entvars_t *pevAttacker, int iGib )
{
	Detonate( );
}


// Timed grenade, this think is called when time runs out.
void CGrenade::DetonateUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	SetThink( &CGrenade::Detonate );
	pev->nextthink = gpGlobals->time;
}

void CGrenade::PreDetonate( void )
{
	CSoundEnt::InsertSound ( bits_SOUND_DANGER, pev->origin, 400, 0.3 );

	SetThink( &CGrenade::Detonate );
	pev->nextthink = gpGlobals->time + 1;
}


void CGrenade::Detonate( void )
{
	TraceResult tr;
	Vector		vecSpot;// trace starts here!

	vecSpot = pev->origin + Vector ( 0 , 0 , 8 );
	UTIL_TraceLine ( vecSpot, vecSpot + Vector ( 0, 0, -40 ),  ignore_monsters, ENT(pev), & tr);

	Explode( &tr, DMG_BLAST );
}


//
// Contact grenade, explode when it touches something
// 
void CGrenade::ExplodeTouch( CBaseEntity *pOther )
{
	TraceResult tr;
	Vector		vecSpot;// trace starts here!

	pev->enemy = pOther->edict();

	vecSpot = pev->origin - pev->velocity.Normalize() * 32;
	UTIL_TraceLine( vecSpot, vecSpot + pev->velocity.Normalize() * 64, ignore_monsters, ENT(pev), &tr );

	Explode( &tr, DMG_BLAST );
}


void CGrenade::DangerSoundThink( void )
{
	if (!IsInWorld())
	{
		UTIL_Remove( this );
		return;
	}

	CSoundEnt::InsertSound ( bits_SOUND_DANGER, pev->origin + pev->velocity * 0.5, static_cast<int>(pev->velocity.Length( )), 0.2 );
	pev->nextthink = gpGlobals->time + 0.2;

	if (pev->waterlevel != 0)
	{
		pev->velocity = pev->velocity * 0.5;
	}
}


void CGrenade::BounceTouch( CBaseEntity *pOther )
{
	// don't hit the guy that launched this grenade
	if ( pOther->edict() == pev->owner )
		return;

	// only do damage if we're moving fairly fast
	if (m_flNextAttack < gpGlobals->time && pev->velocity.Length() > 100)
	{
		entvars_t *pevOwner = VARS( pev->owner );
		if (pevOwner)
		{
			TraceResult tr = UTIL_GetGlobalTrace( );
			ClearMultiDamage( );
			pOther->TraceAttack(pevOwner, 1, gpGlobals->v_forward, &tr, DMG_CLUB ); 
			ApplyMultiDamage( pev, pevOwner);
		}
		m_flNextAttack = gpGlobals->time + 1.0; // debounce
	}

	Vector vecTestVelocity;
	// pev->avelocity = Vector (300, 300, 300);

	// this is my heuristic for modulating the grenade velocity because grenades dropped purely vertical
	// or thrown very far tend to slow down too quickly for me to always catch just by testing velocity. 
	// trimming the Z velocity a bit seems to help quite a bit.
	vecTestVelocity = pev->velocity; 
	vecTestVelocity.z *= 0.45;

	if ( !m_fRegisteredSound && vecTestVelocity.Length() <= 60 )
	{
		//ALERT( at_console, "Grenade Registered!: %f\n", vecTestVelocity.Length() );

		// grenade is moving really slow. It's probably very close to where it will ultimately stop moving. 
		// go ahead and emit the danger sound.
		
		// register a radius louder than the explosion, so we make sure everyone gets out of the way
		CSoundEnt::InsertSound ( bits_SOUND_DANGER, pev->origin, static_cast<int>(pev->dmg / 0.4), 0.3 );
		m_fRegisteredSound = TRUE;
	}

	if (pev->flags & FL_ONGROUND)
	{
		// add a bit of static friction
		pev->velocity = pev->velocity * 0.8;

		pev->sequence = RANDOM_LONG( 1, 1 );
	}
	else
	{
		// play bounce sound
		BounceSound();
	}
	pev->framerate = pev->velocity.Length() / 200.0;
	if (pev->framerate > 1.0)
		pev->framerate = 1;
	else if (pev->framerate < 0.5)
		pev->framerate = 0;

}



void CGrenade::SlideTouch( CBaseEntity *pOther )
{
	// don't hit the guy that launched this grenade
	if ( pOther->edict() == pev->owner )
		return;

	// pev->avelocity = Vector (300, 300, 300);

	if (pev->flags & FL_ONGROUND)
	{
		// add a bit of static friction
		pev->velocity = pev->velocity * 0.95;

		if (pev->velocity.x != 0 || pev->velocity.y != 0)
		{
			// maintain sliding sound
		}
	}
	else
	{
		BounceSound();
	}
}

void CGrenade :: BounceSound( void )
{
	switch ( RANDOM_LONG( 0, 2 ) )
	{
	case 0:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/grenade_hit1.wav", 0.25, ATTN_NORM);	break;
	case 1:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/grenade_hit2.wav", 0.25, ATTN_NORM);	break;
	case 2:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/grenade_hit3.wav", 0.25, ATTN_NORM);	break;
	}
}

void CGrenade :: TumbleThink( void )
{
	if (!IsInWorld())
	{
		UTIL_Remove( this );
		return;
	}

	StudioFrameAdvance( );
	pev->nextthink = gpGlobals->time + 0.1;

	if (pev->dmgtime - 1 < gpGlobals->time)
	{
		CSoundEnt::InsertSound ( bits_SOUND_DANGER, pev->origin + pev->velocity * (pev->dmgtime - gpGlobals->time), 400, 0.1 );
	}

	if (pev->dmgtime <= gpGlobals->time)
	{
		SetThink( &CGrenade::Detonate );
	}
	if (pev->waterlevel != 0)
	{
		pev->velocity = pev->velocity * 0.5;
		pev->framerate = 0.2;
	}
}


void CGrenade:: Spawn( void )
{
	pev->movetype = MOVETYPE_BOUNCE;
	pev->classname = MAKE_STRING( "grenade" );
	
	pev->solid = SOLID_BBOX;

	SET_MODEL(ENT(pev), "models/grenade.mdl");
	UTIL_SetSize(pev, Vector( 0, 0, 0), Vector(0, 0, 0));

	pev->dmg = 100;
	m_fRegisteredSound = FALSE;
}


CGrenade *CGrenade::ShootContact( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity )
{
	CGrenade *pGrenade = GetClassPtr( (CGrenade *)NULL );
	pGrenade->Spawn();
	// contact grenades arc lower
	pGrenade->pev->gravity = 0.5;// lower gravity since grenade is aerodynamic and engine doesn't know it.
	UTIL_SetOrigin( pGrenade->pev, vecStart );
	pGrenade->pev->velocity = vecVelocity;
	pGrenade->pev->angles = UTIL_VecToAngles (pGrenade->pev->velocity);
	pGrenade->pev->owner = ENT(pevOwner);
	
	// make monsters afaid of it while in the air
	pGrenade->SetThink( &CGrenade::DangerSoundThink );
	pGrenade->pev->nextthink = gpGlobals->time;
	
	// Tumble in air
	pGrenade->pev->avelocity.x = RANDOM_FLOAT ( -100, -500 );
	
	// Explode on contact
	pGrenade->SetTouch( &CGrenade::ExplodeTouch );

	pGrenade->pev->dmg = gSkillData.plrDmgM203Grenade;

	return pGrenade;
}


CGrenade * CGrenade:: ShootTimed( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float time )
{
	CGrenade *pGrenade = GetClassPtr( (CGrenade *)NULL );
	pGrenade->Spawn();
	UTIL_SetOrigin( pGrenade->pev, vecStart );
	pGrenade->pev->velocity = vecVelocity;
	pGrenade->pev->angles = UTIL_VecToAngles(pGrenade->pev->velocity);
	pGrenade->pev->owner = ENT(pevOwner);
	
	pGrenade->SetTouch( &CGrenade::BounceTouch );	// Bounce if touched
	
	// Take one second off of the desired detonation time and set the think to PreDetonate. PreDetonate
	// will insert a DANGER sound into the world sound list and delay detonation for one second so that 
	// the grenade explodes after the exact amount of time specified in the call to ShootTimed(). 

	pGrenade->pev->dmgtime = gpGlobals->time + time;
	pGrenade->SetThink( &CGrenade::TumbleThink );
	pGrenade->pev->nextthink = gpGlobals->time + 0.1;
	if (time < 0.1)
	{
		pGrenade->pev->nextthink = gpGlobals->time;
		pGrenade->pev->velocity = Vector( 0, 0, 0 );
	}
		
	pGrenade->pev->sequence = RANDOM_LONG( 3, 6 );
	pGrenade->pev->framerate = 1.0;

	// Tumble through the air
	// pGrenade->pev->avelocity.x = -400;

	pGrenade->pev->gravity = 0.5;
	pGrenade->pev->friction = 0.8;

	SET_MODEL(ENT(pGrenade->pev), "models/w_grenade.mdl");
	pGrenade->pev->dmg = 100;

	return pGrenade;
}


void CGrenade :: UseSatchelCharges( entvars_t *pevOwner, SATCHELCODE code )
{
	edict_t *pentFind;
	edict_t *pentOwner;

	if ( !pevOwner )
		return;

	CBaseEntity	*pOwner = CBaseEntity::Instance( pevOwner );

	pentOwner = pOwner->edict();

	pentFind = FIND_ENTITY_BY_CLASSNAME( NULL, "grenade" );
	while ( !FNullEnt( pentFind ) )
	{
		CBaseEntity *pEnt = Instance( pentFind );
		if ( pEnt )
		{
			if ( FBitSet( pEnt->pev->spawnflags, SF_DETONATE ) && pEnt->pev->owner == pentOwner )
			{
				if ( code == SATCHEL_DETONATE )
					pEnt->Use( pOwner, pOwner, USE_ON, 0 );
				else	// SATCHEL_RELEASE
					pEnt->pev->owner = NULL;
			}
		}
		pentFind = FIND_ENTITY_BY_CLASSNAME( pentFind, "grenade" );
	}
}


// ===============
//	SMOKE GRENADE
// ===============

CGrenade *CGrenade::ShootSmokeGrenade( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float time, unsigned short usEvent )
{
    CGrenade *pGrenade = GetClassPtr( ( CGrenade* )NULL );
    
    pGrenade->Spawn();

    UTIL_SetOrigin( pGrenade->pev, vecStart );

    pGrenade->pev->velocity = vecVelocity;
    pGrenade->pev->angles   = pevOwner->angles;
    pGrenade->pev->owner    = ENT( pevOwner );
    pGrenade->m_usEvent     = usEvent;
    pGrenade->m_bLightSmoke = false;
    pGrenade->m_bDetonated  = false;

    pGrenade->SetTouch( &CGrenade::BounceTouch );
    pGrenade->SetThink( &CGrenade::SG_TumbleThink );

    pGrenade->pev->dmgtime   = gpGlobals->time + time;
    pGrenade->pev->nextthink = gpGlobals->time + 0.1;

    if( time < 0.1 )
    {
        pGrenade->pev->nextthink = gpGlobals->time;
        pGrenade->pev->velocity  = Vector( 0, 0, 0 );
    }

    pGrenade->pev->sequence  = RANDOM_LONG( 3, 6 );
    pGrenade->pev->framerate = 1;
    pGrenade->m_bJustBlew    = true;
    pGrenade->pev->gravity   = 0.5;
    pGrenade->pev->friction  = 0.8;
    pGrenade->m_SGSmoke      = 0;

    SET_MODEL( pGrenade->edict(), "models/w_smokegrenade.mdl" );

    pGrenade->pev->dmg = 35;

    return pGrenade;
}

void CGrenade::SG_TumbleThink( void )
{
    if( !IsInWorld() )
    {
        UTIL_Remove( this );
        return;
    }

    if( FBitSet( pev->flags, FL_ONGROUND ) )
    {
        pev->velocity = pev->velocity * 0.95;
    }

    StudioFrameAdvance();
    pev->nextthink = gpGlobals->time + 0.1;

    if( pev->dmgtime - 1.0 < gpGlobals->time )
    {
        CSoundEnt::InsertSound( bits_SOUND_DANGER, pev->origin + pev->velocity * ( pev->dmgtime - gpGlobals->time ), 400, 0.1 );
    }

    if( pev->dmgtime <= gpGlobals->time )
    {
        if( FBitSet(  pev->flags, FL_ONGROUND ) )
        {
            SetThink( &CGrenade::SG_Detonate );
        }
    }

    if( pev->waterlevel != 0 )
    {
        pev->velocity = pev->velocity * 0.5;
        pev->framerate = 0.2;
    }
}

void CGrenade::SG_Detonate( void )
{
    TraceResult tr;
    Vector vecSpot = pev->origin + Vector( 0, 0, 8 );

    UTIL_TraceLine( vecSpot, vecSpot + Vector( 0, 0, -40 ), ignore_monsters, edict(), &tr );

    EMIT_SOUND( edict(), CHAN_WEAPON, "weapons/sg_explode.wav", VOL_NORM, ATTN_NORM );
    
    edict_t *pentGrenade = NULL;

    while( ( pentGrenade = FIND_ENTITY_BY_STRING( pentGrenade, "classname", "grenade" ) ) != NULL )
    {
        if( FNullEnt( pentGrenade ) )
        {
            break;
        }

        CBaseEntity *pGrenade = CBaseEntity::Instance( pentGrenade );

        if( pGrenade )
        {
            float fDistance = ( pGrenade->pev->origin - pev->origin ).Length();

            if( fDistance != 0 && fDistance <= 250 && gpGlobals->time < pGrenade->pev->dmgtime )
            {
                m_bLightSmoke = true;
            }
        }
    }

    m_bDetonated = true;

    PLAYBACK_EVENT_FULL( FEV_CLIENT, NULL, m_usEvent, 0, pev->origin, ( float* )&g_vecZero, 0, 0, 0, 1, m_bLightSmoke, FALSE );

    m_vSmokeDetonate = pev->origin;

    pev->velocity = Vector( RANDOM_FLOAT( -175.0, 175.0 ), RANDOM_FLOAT( -175.0, 175.0 ), RANDOM_FLOAT( 250.0, 350.0 ) );
    pev->nextthink = gpGlobals->time + 0.1;

    SetThink( &CGrenade::SG_Smoke );
}

void CGrenade::SG_Smoke( void )
{
    if( UTIL_PointContents( pev->origin ) != CONTENTS_WATER )
    {
        Vector  vecDir;
        float   interval[2];
        int     maxSmokePuffs;

        UTIL_MakeVectors( pev->angles );

        vecDir = gpGlobals->v_forward * RANDOM_FLOAT( 3, 8 );

        maxSmokePuffs = ( int )(RANDOM_FLOAT( 1.5, 3.5 ) * 100 );

        interval[0] = vecDir.x * cos( ( float )m_angle * ( 180 / M_PI ) ) - vecDir.y * sin( ( float )m_angle * ( 180 / M_PI ) );
        interval[1] = vecDir.x * sin( ( float )m_angle * ( 180 / M_PI ) ) + vecDir.y * cos( ( float )m_angle * ( 180 / M_PI ) );
        
        m_angle = ( m_angle + 30 ) % 360;
        
        PLAYBACK_EVENT_FULL( 0, NULL, m_usEvent, 0, pev->origin, m_vSmokeDetonate, interval[0], interval[1], maxSmokePuffs, 4, m_bLightSmoke, 6 );
    }
    else
    {
        UTIL_Bubbles( pev->origin - Vector( 64, 64, 64 ), pev->origin + Vector( 64, 64, 64 ), 100 );
    }

    if( m_SGSmoke <= 20 )
    {
        pev->nextthink = gpGlobals->time + 1.0;
        SetThink( &CGrenade::SG_Smoke );

        m_SGSmoke++;
    }
    else
    {
        pev->effects |= EF_NODRAW;
        
        // TODO: Implement me
        // TheBots->CBotManager::RemoveGrenade( this );

        UTIL_Remove( this );
    }
}


// ==========
//	C4 BOMB
// ==========

CGrenade *CGrenade::ShootSatchelCharge( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity )	// Last check: 2013, May 28
{
	CGrenade *pGrenade = GetClassPtr( ( CGrenade* )NULL );

	pGrenade->pev->movetype	= MOVETYPE_TOSS;
	pGrenade->pev->solid	= SOLID_BBOX;

	// TODO: Implements me.
	// if( pev->classname )
	//		RemoveEntityHashValue( STRING( pev->classname ) );

	pGrenade->pev->classname = MAKE_STRING( "grenade" );

	// TODO: Implements me.
	// AddEntityHashValue( STRING( pev->classname ) );

	SET_MODEL( ENT( pGrenade->pev ), "models/w_c4.mdl" );

	UTIL_SetSize( pGrenade->pev, Vector( 3, 6, 8 ), Vector( -3, -6, 0 ) );
	UTIL_SetOrigin( pGrenade->pev, vecStart );

	pGrenade->pev->dmg		= 100;
	pGrenade->pev->angles	= vecVelocity;
	pGrenade->pev->velocity = g_vecZero;
	pGrenade->pev->owner	= ENT( pevOwner );

	pGrenade->SetThink( &CGrenade::C4Think );
	pGrenade->SetTouch( &CGrenade::C4Touch );

	pGrenade->pev->spawnflags = SF_DETONATE;
	pGrenade->pev->nextthink  = gpGlobals->time + 0.1;

	pGrenade->m_flC4Blow			= gpGlobals->time + g_pGameRules->m_iC4Timer;
	pGrenade->m_flNextFreqInterval	= g_pGameRules->m_iC4Timer / 4;
	pGrenade->m_flNextFreq			= gpGlobals->time;
	pGrenade->m_flNextBeep			= gpGlobals->time + 0.5;
	pGrenade->m_flNextBlink			= gpGlobals->time + 2.0;

	pGrenade->m_iCurWave	= 0;
	pGrenade->m_fAttenu		= 0;
	pGrenade->m_sBeepName	= NULL;

	pGrenade->m_fNextDefuse = 0;

	pGrenade->m_bIsC4		= true;
	pGrenade->m_bStartDefuse= false;
	pGrenade->m_bJustBlew	= false;

	pGrenade->pev->friction = 0.9;

	if( !FNullEnt( pevOwner ) )
	{
		CBasePlayer *pPlayer = ( CBasePlayer* )Instance( pevOwner );

		if( pPlayer->IsPlayer() )
			pGrenade->m_pentCurBombTarget = pPlayer->m_pentCurBombTarget;
	}
	else
		pGrenade->m_pentCurBombTarget = NULL;

	return pGrenade;
}

void CGrenade::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value ) // Last check: 2013, May 28
{
	if( !m_bIsC4 )
	{
		return;
	}

	int barTime = 0;

	CBasePlayer* pPlayer = GetClassPtr( ( CBasePlayer* )pActivator->pev );

	if( pPlayer->m_iTeam == CT )
	{
		if( m_bStartDefuse )
		{
			m_fNextDefuse = gpGlobals->time + 0.5;
			return;
		}

		g_engfuncs.pfnSetClientMaxspeed( ENT( pPlayer->pev ), 1.0 );

		// TODO: Adds support for bots.
		// TheBots->OnEvent( EVENT_BOMB_DEFUSING, pActivator, NULL );

		if( g_pGameRules->IsCareer() )
		{
			TheCareerTasks->HandleEvent( EVENT_BOMB_DEFUSING, NULL, NULL );
		}

		if( pPlayer->m_bHasDefuser )
		{
			UTIL_LogPrintf(	"\"%s<%i><%s><CT>\" triggered \"Begin_Bomb_Defuse_With_Kit\"\n", 
				STRING( pPlayer->pev->netname ),
				GETPLAYERAUTHID( pPlayer->edict() ),
				GETPLAYERUSERID( pPlayer->edict() ) );

			ClientPrint( pPlayer->pev, HUD_PRINTCENTER, "#Defusing_Bomb_With_Defuse_Kit" );
			EMIT_SOUND( ENT( pPlayer->pev ), CHAN_ITEM, "weapons/c4_disarm.wav", VOL_NORM, ATTN_NORM );

			pPlayer->m_bIsDefusing = true;

			m_pBombDefuser		= pActivator;
			m_bStartDefuse		= TRUE;
			m_flDefuseCountDown	= gpGlobals->time + 5.0;
			m_fNextDefuse		= gpGlobals->time + 0.5;

			barTime = 5;
		}
		else
		{
			UTIL_LogPrintf(	"\"%s<%i><%s><CT>\" triggered \"Begin_Bomb_Defuse_Without_Kit\"\n", 
				STRING( pPlayer->pev->netname ),
				GETPLAYERAUTHID( pPlayer->edict() ),
				GETPLAYERUSERID( pPlayer->edict() ) );

			ClientPrint( pPlayer->pev, HUD_PRINTCENTER, "#Defusing_Bomb_Without_Defuse_Kit" );
			EMIT_SOUND( ENT( pPlayer->pev ), CHAN_ITEM, "weapons/c4_disarm.wav", VOL_NORM, ATTN_NORM );

			pPlayer->m_bIsDefusing = true;

			m_pBombDefuser		= pActivator;
			m_bStartDefuse		= TRUE;
			m_flDefuseCountDown	= gpGlobals->time + 10.0;
			m_fNextDefuse		= gpGlobals->time + 0.5;

			barTime = 10;
		}

		pPlayer->SetProgressBarTime( barTime );
	}
}

void CGrenade::Detonate2( void ) // Last check: 2013, May 28
{
	TraceResult tr;
	Vector		vecSpot;

	vecSpot = pev->origin + Vector( 0 , 0 , 8 );
	UTIL_TraceLine( vecSpot, vecSpot + Vector( 0, 0, -40 ), ignore_monsters, ENT( pev ), &tr );

	Explode2( &tr, DMG_BLAST );
}

void CGrenade::Smoke2( void ) // Last check: 2013, May 28
{
	if( UTIL_PointContents( pev->origin ) == CONTENTS_WATER )
	{
		UTIL_Bubbles( pev->origin - Vector( 64, 64, 64 ), pev->origin + Vector( 64, 64, 64 ), 100 );
	}
	else
	{
		MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, pev->origin );
			WRITE_BYTE( TE_SMOKE );
			WRITE_COORD( pev->origin.x );
			WRITE_COORD( pev->origin.y );
			WRITE_COORD( pev->origin.z );
			WRITE_SHORT( g_sModelIndexSmoke );
			WRITE_BYTE( 150 );	// scale * 10
			WRITE_BYTE( 8  );	// framerate
		MESSAGE_END();
	}

	UTIL_Remove( this );
}

void CGrenade::Explode2( TraceResult* pTrace, int bitsDamageType )  // Last check: 2013, May 28
{
    CHalfLifeMultiplay *pGameRules = ( CHalfLifeMultiplay* )g_pGameRules;

    pev->model		= iStringNull;	// invisible
    pev->solid		= SOLID_NOT;	// intangible
    pev->takedamage = DAMAGE_NO;

    UTIL_ScreenShake( pTrace->vecEndPos, 25.0, 150.0, 1.0, 3000.0 );

    pGameRules->m_bTargetBombed = true;

    if( pGameRules->IsCareer() )
    {
        TheCareerTasks->LatchRoundEndMessage();
    }

    m_bJustBlew = true;
    pGameRules->CheckWinConditions();

    if( pTrace->flFraction != 1.0 )
    {
        pev->origin = pTrace->vecEndPos + ( pTrace->vecPlaneNormal * ( pev->dmg - 24 ) * 0.6 );
    }

    bool isInWater = UTIL_PointContents( pev->origin ) == CONTENTS_WATER;

    MESSAGE_BEGIN( MSG_PAS, SVC_TEMPENTITY, pev->origin );
        WRITE_BYTE( TE_SPRITE );		
        WRITE_COORD( pev->origin.x );
        WRITE_COORD( pev->origin.y );
        WRITE_COORD( pev->origin.z - 10.0 );
        WRITE_SHORT( g_sModelIndexFireball3 );
        WRITE_BYTE( ( pev->dmg - 275 ) * 0.6 );
        WRITE_BYTE( 150 ); 
    MESSAGE_END();

    MESSAGE_BEGIN( MSG_PAS, SVC_TEMPENTITY, pev->origin );
        WRITE_BYTE( TE_SPRITE );		
        WRITE_COORD( pev->origin.x + RANDOM_FLOAT( -512, 512 ) );
        WRITE_COORD( pev->origin.y + RANDOM_FLOAT( -512, 512 ) );
        WRITE_COORD( pev->origin.z + RANDOM_FLOAT( -10, 10 ) );
        WRITE_SHORT( g_sModelIndexFireball2 );
        WRITE_BYTE( floor( ( pev->dmg - 275 ) * 0.6 ) );
        WRITE_BYTE( 150 ); 
    MESSAGE_END();

    MESSAGE_BEGIN( MSG_PAS, SVC_TEMPENTITY, pev->origin );
        WRITE_BYTE( TE_SPRITE );		
        WRITE_COORD( pev->origin.x + RANDOM_FLOAT( -512, 512 ) );
        WRITE_COORD( pev->origin.y + RANDOM_FLOAT( -512, 512 ) );
        WRITE_COORD( pev->origin.z + RANDOM_FLOAT( -10, 10 ) );
        WRITE_SHORT( g_sModelIndexFireball3 );
        WRITE_BYTE( floor( ( pev->dmg - 275 ) * 0.6 ) );
        WRITE_BYTE( 150 ); 
    MESSAGE_END();

    MESSAGE_BEGIN( MSG_PAS, SVC_TEMPENTITY, pev->origin );
        WRITE_BYTE( TE_SPRITE );		
        WRITE_COORD( pev->origin.x + RANDOM_FLOAT( -512, 512 ) );
        WRITE_COORD( pev->origin.y + RANDOM_FLOAT( -512, 512 ) );
        WRITE_COORD( pev->origin.z + RANDOM_FLOAT( -10, 10 ) );
        WRITE_SHORT( g_sModelIndexFireball );
        WRITE_BYTE( floor( ( pev->dmg - 275 ) * 0.6 ) );
        WRITE_BYTE( 17 ); 
    MESSAGE_END();

    EMIT_SOUND( ENT( pev ), CHAN_WEAPON, "weapons/c4_explode1.wav", VOL_NORM, 0.25 );

    CSoundEnt::InsertSound( bits_SOUND_COMBAT, pev->origin, NORMAL_EXPLOSION_VOLUME, 3.0 );

    entvars_t *pevOwner;

    if ( pev->owner )
        pevOwner = VARS( pev->owner );
    else
        pevOwner = NULL;

    pev->owner = NULL; // Can't traceline attack owner if this is set.

    // TODO: Fix me.
    //RadiusDamage( pev, pevOwner, pev->dmg, g_pGameRules->m_flBombRadius, CLASS_NONE, bitsDamageType );

	if( pGameRules->IsCareer() )
    {
        TheCareerTasks->UnlatchRoundEndMessage();
    }

    MESSAGE_BEGIN( MSG_SPEC, SVC_DIRECTOR );
        WRITE_BYTE( 9 );
        WRITE_BYTE( DRC_CMD_EVENT );
        WRITE_SHORT( ENTINDEX( this->edict() ) );
        WRITE_SHORT( NULL );
        WRITE_ENTITY( DRC_FLAG_FINAL | DRC_FLAG_PRIO_MASK );
    MESSAGE_END();

    UTIL_DecalTrace( pTrace, RANDOM_FLOAT( 0 , 1 ) < 0.5 ? DECAL_SCORCH1 : DECAL_SCORCH2 );

    switch ( RANDOM_LONG( 0, 2 ) )
    {
        case 0:	EMIT_SOUND( ENT( pev ), CHAN_VOICE, "weapons/debris1.wav", 0.55, ATTN_NORM ); break;
        case 1:	EMIT_SOUND( ENT( pev ), CHAN_VOICE, "weapons/debris2.wav", 0.55, ATTN_NORM ); break;
        case 2:	EMIT_SOUND( ENT( pev ), CHAN_VOICE, "weapons/debris3.wav", 0.55, ATTN_NORM ); break;
    }

    pev->effects |= EF_NODRAW;
    SetThink( &CGrenade::Smoke2 );

    pev->velocity	= g_vecZero;
    pev->nextthink	= gpGlobals->time + 0.85;

    if( !isInWater )
    {
        int sparkCount = RANDOM_LONG( 0, 3 );

        for( int i = 0; i < sparkCount; i++ )
        {
            Create( "spark_shower", pev->origin, pTrace->vecPlaneNormal, NULL );
        }
    }
} 

void CGrenade::C4Think( void )
{
	if( !IsInWorld() )
	{
		UTIL_Remove( this );
		return;
	}

	pev->nextthink = gpGlobals->time + 0.12;

	if( gpGlobals->time >= m_flNextFreq )
	{
		m_flNextFreq = gpGlobals->time + m_flNextFreqInterval;
		m_flNextFreqInterval *= 0.9;

		switch( m_iCurWave )
		{
			case 0 :
			{
                m_sBeepName = "weapons/c4_beep1.wav";
				m_fAttenu = 1.5;

				if( UTIL_IsGame( "czero" ) )
				{
					MESSAGE_BEGIN( MSG_ALL, gmsgScenarioIcon );
						WRITE_BYTE( 1 );
						WRITE_STRING( "bombticking" );
						WRITE_BYTE( 255 );
						WRITE_SHORT( 140 );
						WRITE_SHORT( 0 );
					MESSAGE_END();
				}

				break;
			}
			case 1 :
			{
                m_sBeepName = "weapons/c4_beep2.wav";
				m_fAttenu = 1.0;

				if( UTIL_IsGame( "czero" ) )
				{
					MESSAGE_BEGIN( MSG_ALL, gmsgScenarioIcon );
						WRITE_BYTE( 1 );
						WRITE_STRING( "bombticking" );
						WRITE_BYTE( 255 );
						WRITE_SHORT( 70 );
						WRITE_SHORT( 0 );
					MESSAGE_END();
				}
	
				break;
			}
			case 2 :
			{
                m_sBeepName = "weapons/c4_beep3.wav";
				m_fAttenu = 0.8;
				
				if( UTIL_IsGame( "czero" ) )
				{
					MESSAGE_BEGIN( MSG_ALL, gmsgScenarioIcon );
						WRITE_BYTE( 1 );
						WRITE_STRING( "bombticking" );
						WRITE_BYTE( 255 );
						WRITE_SHORT( 40 );
						WRITE_SHORT( 0 );
					MESSAGE_END();
				}

				break;
			}
			case 3 :
			{
                m_sBeepName = "weapons/c4_beep4.wav";
				m_fAttenu = 0.5;

				if( UTIL_IsGame( "czero" ) )
				{
					MESSAGE_BEGIN( MSG_ALL, gmsgScenarioIcon );
						WRITE_BYTE( 1 );
						WRITE_STRING( "bombticking" );
						WRITE_BYTE( 255 );
						WRITE_SHORT( 30 );
						WRITE_SHORT( 0 );
					MESSAGE_END();
				}

				break;
			}
			case 4 :
			{
                m_sBeepName = "weapons/c4_beep5.wav";
				m_fAttenu = 0.2;

				if( UTIL_IsGame( "czero" ) )
				{
					MESSAGE_BEGIN( MSG_ALL, gmsgScenarioIcon );
						WRITE_BYTE( 1 );
						WRITE_STRING( "bombticking" );
						WRITE_BYTE( 255 );
						WRITE_SHORT( 20 );
						WRITE_SHORT( 0 );
					MESSAGE_END();
				}

				break;
			}		
		}

		++m_iCurWave;
	}

	if( m_flNextBeep < gpGlobals->time )
	{
		m_flNextBeep = gpGlobals->time + 1.4;
		EMIT_SOUND( ENT( pev ), CHAN_VOICE, m_sBeepName, VOL_NORM, m_fAttenu );

		// TODO: Adds support for bots.
		// TheBots->OnEvent( EVENT_BOMB_BEEP, this, NULL );
	}

	if( m_flNextBlink < gpGlobals->time )
	{
		m_flNextBlink = gpGlobals->time	+ 2.0;

		MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, pev->origin );
			WRITE_BYTE( TE_GLOWSPRITE );
			WRITE_COORD( pev->origin.x );
			WRITE_COORD( pev->origin.y );
			WRITE_COORD( pev->origin.z + 5.0 );
			WRITE_SHORT( g_sModelIndexC4Glow );
			WRITE_BYTE( 1 );
			WRITE_BYTE( 3 );
			WRITE_BYTE( 255 );
		MESSAGE_END();
	}

	CHalfLifeMultiplay *pGameRules = ( CHalfLifeMultiplay* )g_pGameRules;

	if( m_flC4Blow <= gpGlobals->time )
	{
		// TODO: Adds support for bots.
		// TheBots->OnEvent( EVENT_BOMB_EXPLODED, NULL, NULL );

		MESSAGE_BEGIN( MSG_ALL, gmsgScenarioIcon );
		WRITE_BYTE( 0 );
		MESSAGE_END();

		if( m_pentCurBombTarget )
		{
			CBaseEntity *pEntity = CBaseEntity::Instance( m_pentCurBombTarget );

			if( pEntity )
			{
				CBaseEntity* pPlayer = CBaseEntity::Instance( pev->owner );

				if( pPlayer )
				{
					pEntity->Use( pPlayer, this, USE_TOGGLE, 0 );
				}
			}
		}

		CBasePlayer* pPlayer = (CBasePlayer *)CBaseEntity::Instance( pev->owner );

		if( pPlayer )
		{
			pPlayer->pev->frags += 3;
		}

		MESSAGE_BEGIN( MSG_ALL, gmsgBombPickup );
		MESSAGE_END();

		pGameRules->m_bBombDropped = false;

		if( pev->waterlevel )
			UTIL_Remove( this );
		else
			SetThink( &CGrenade::Detonate2 );	
	}

	if( m_bStartDefuse )
	{
		CBasePlayer* pDefuser = (CBasePlayer *)((CBaseEntity *)m_pBombDefuser);

		if( pDefuser && m_flDefuseCountDown > gpGlobals->time )
		{
			BOOL isOnGround = !!( pDefuser->pev->flags & FL_ONGROUND );

			if( !isOnGround || m_fNextDefuse < gpGlobals->time )
			{
				if( !isOnGround )
				{
					ClientPrint( pDefuser->pev, HUD_PRINTCENTER, "#C4_Defuse_Must_Be_On_Ground" );
				}

				pDefuser->ResetMaxSpeed();
				pDefuser->SetProgressBarTime( 0 );

				pDefuser->m_bIsDefusing = false;

				m_bStartDefuse = false;
				m_flDefuseCountDown = 0.0;

				// TODO: Adds support for bots.
				// TheBots->OnEvent( EVENT_DEFUSE_ABORTED, NULL, NULL );
			}
		}
		else
		{
			// TODO: Adds support for bots.
			// TheBots->OnEvent( EVENT_BOMB_DEFUSED, pDefuser, NULL );

			Broadcast( "BOMBDEF" );

			MESSAGE_BEGIN( MSG_SPEC, SVC_DIRECTOR );
				WRITE_BYTE( 9 );
				WRITE_BYTE( DRC_CMD_EVENT );
				WRITE_SHORT( ENTINDEX( this->edict() ) );
				WRITE_SHORT( NULL );
				WRITE_ENTITY( DRC_FLAG_FINAL | DRC_FLAG_FACEPLAYER | DRC_FLAG_DRAMATIC | 15 );
			MESSAGE_END();

			UTIL_LogPrintf(	"\"%s<%i><%s><CT>\" triggered \"Defused_The_Bomb\"\n", 
				STRING( pDefuser->pev->netname ),
				GETPLAYERAUTHID( pDefuser->edict() ),
				GETPLAYERUSERID( pDefuser->edict() ) );

			UTIL_EmitAmbientSound( ENT( pev ), pev->origin, "weapons/c4_beep5.wav", 0, ATTN_NONE, SND_STOP, 0 );
			EMIT_SOUND( ENT( pDefuser->pev ), CHAN_WEAPON, "weapons/c4_disarmed.wav", 0.8, ATTN_NORM );

			UTIL_Remove( this );
			m_bJustBlew = true;

			pDefuser->ResetMaxSpeed();
			pDefuser->m_bIsDefusing = false;

			MESSAGE_BEGIN( MSG_ALL, gmsgScenarioIcon );
			WRITE_BYTE( 0 );
			MESSAGE_END();

			if( pGameRules->IsCareer() )
			{
				TheCareerTasks->HandleEvent( EVENT_BOMB_DEFUSED, pDefuser, NULL );
			}

			pGameRules->m_bBombDefused = true;
			pGameRules->CheckWinConditions();

			pDefuser->pev->frags += 3;

			MESSAGE_BEGIN( MSG_ALL, gmsgBombPickup );
			MESSAGE_END();

			pGameRules->m_bBombDropped = FALSE;
			m_bStartDefuse = false;			
		}
	}
}

void CGrenade::C4Touch( CBaseEntity* pOther ) 
{
	// Nothing.
}

