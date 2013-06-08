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
//
// teamplay_gamerules.cpp
//

#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"player.h"
#include	"weapons.h"
#include	"gamerules.h"
 
#include	"skill.h"
#include	"game.h"
#include	"items.h"
#include	"voice_gamemgr.h"
#include	"hltv.h"
#include "bot.h"
#include "client.h"

extern DLL_GLOBAL CHalfLifeMultiplay *g_pGameRules;
extern DLL_GLOBAL BOOL	g_fGameOver;
extern int gmsgDeathMsg;	// client dll messages
extern int gmsgScoreInfo;
extern int gmsgMOTD;
extern int gmsgServerName;

// CS
extern int gmsgBombPickup;
extern int gmsgShowTimer;
extern int gmsgHLTV;
extern int gmsgTeamScore;
extern int gmsgSendAudio;

extern int g_teamplay;

#define ITEM_RESPAWN_TIME	30
#define WEAPON_RESPAWN_TIME	20
#define AMMO_RESPAWN_TIME	20

float g_flIntermissionStartTime = 0;

// CS
float g_flResetTime = 0;
float g_flTimeLimit = 0;

CVoiceGameMgr	g_VoiceGameMgr;

class CMultiplayGameMgrHelper : public IVoiceGameMgrHelper
{
public:
	virtual bool		CanPlayerHearPlayer(CBasePlayer *pListener, CBasePlayer *pTalker)
	{
		if ( g_teamplay )
		{
			if ( g_pGameRules->PlayerRelationship( pListener, pTalker ) != GR_TEAMMATE )
			{
				return false;
			}
		}

		return true;
	}
};
static CMultiplayGameMgrHelper g_GameMgrHelper;


// CS
void Broadcast( const char *sentence )
{
    if( sentence )
    {
        char message[ 32 ];

        strcpy( message, "%!MRAD_" );
        strcat( message, UTIL_VarArgs( "%s", sentence ) );

        MESSAGE_BEGIN( MSG_BROADCAST, gmsgSendAudio );
            WRITE_BYTE( 0 );
            WRITE_STRING( message );
            WRITE_SHORT( 100 );
        MESSAGE_END();
    }
}

// CS
char *GetTeam( int teamNo )
{
    switch( teamNo )
    {
        case TERRORIST : return "TERRORIST";
        case CT        : return "CT";
        case SPECTATOR : return "SPECTATOR";
    }

    return "";
}

// CS
void EndRoundMessage( const char *sentence, int event )
{
    char *team = NULL;
    bool withTeam = true;
    GameEventType botEvent = EVENT_INVALID;

    UTIL_ClientPrintAll( HUD_PRINTCENTER, sentence );

    switch( event )
    {
        case Event_Target_Bombed:
        case Event_VIP_Assassinated:
        case Event_Terrorists_Escaped:
        case Event_Terrorists_Win:
        case Event_Hostages_Not_Rescued:
        case Event_VIP_Not_Escaped: team = GetTeam( TERRORIST ); botEvent = EVENT_TERRORISTS_WIN; break;
        case Event_VIP_Escaped:
        case Event_CTs_PreventEscape:
        case Event_Escaping_Terrorists_Neutralized:
        case Event_Bomb_Defused:
        case Event_CTs_Win:
        case Event_All_Hostages_Rescued:
        case Event_Target_Saved:
        case Event_Terrorists_Not_Escaped: team = GetTeam( CT ); botEvent = EVENT_CTS_WIN; break;
        default: withTeam = false; botEvent = EVENT_ROUND_DRAW; break;
    }

    if( botEvent )
    {
        // TODO: Impplement me.
        // TheBots->OnEvent( botEvent, NULL, NULL );
    }

    if( g_pGameRules )
    {
        if( withTeam )
            UTIL_LogPrintf( "Team \"%s\" triggered \"%s\" (CT \"%i\") (T \"%i\")\n", team, &sentence[ 1 ], g_pGameRules->m_iNumCTWins, g_pGameRules->m_iNumTerroristWins );
        else
            UTIL_LogPrintf( "World triggered \"%s\" (CT \"%i\") (T \"%i\")\n", &sentence[ 1 ], g_pGameRules->m_iNumCTWins, g_pGameRules->m_iNumTerroristWins );
    }

    UTIL_LogPrintf( "World triggered \"Round_End\"\n" );
}

//*********************************************************
// Rules for the half-life multiplayer game.
//*********************************************************

CHalfLifeMultiplay :: CHalfLifeMultiplay()
{
	g_VoiceGameMgr.Init(&g_GameMgrHelper, gpGlobals->maxClients);

	RefreshSkillData();
	m_flIntermissionEndTime = 0;
	g_flIntermissionStartTime = 0;
	
	// 11/8/98
	// Modified by YWB:  Server .cfg file is now a cvar, so that 
	//  server ops can run multiple game servers, with different server .cfg files,
	//  from a single installed directory.
	// Mapcyclefile is already a cvar.

	// 3/31/99
	// Added lservercfg file cvar, since listen and dedicated servers should not
	// share a single config file. (sjb)
	if ( IS_DEDICATED_SERVER() )
	{
		// dedicated server
		char *servercfgfile = (char *)CVAR_GET_STRING( "servercfgfile" );

		if ( servercfgfile && servercfgfile[0] )
		{
			char szCommand[256];
			
			ALERT( at_console, "Executing dedicated server config file\n" );
			sprintf( szCommand, "exec %s\n", servercfgfile );
			SERVER_COMMAND( szCommand );
		}
	}
	else
	{
		// listen server
		char *lservercfgfile = (char *)CVAR_GET_STRING( "lservercfgfile" );

		if ( lservercfgfile && lservercfgfile[0] )
		{
			char szCommand[256];
			
			ALERT( at_console, "Executing listen server config file\n" );
			sprintf( szCommand, "exec %s\n", lservercfgfile );
			SERVER_COMMAND( szCommand );
		}
	}
}

BOOL CHalfLifeMultiplay::ClientCommand( CBasePlayer *pPlayer, const char *pcmd )
{
	if(g_VoiceGameMgr.ClientCommand(pPlayer, pcmd))
		return TRUE;

	return CGameRules::ClientCommand(pPlayer, pcmd);
}

//=========================================================
//=========================================================
void CHalfLifeMultiplay::RefreshSkillData( void )
{
// load all default values
	CGameRules::RefreshSkillData();

// override some values for multiplay.

	// suitcharger
	gSkillData.suitchargerCapacity = 30;

	// Crowbar whack
	gSkillData.plrDmgCrowbar = 25;

	// Glock Round
	gSkillData.plrDmg9MM = 12;

	// 357 Round
	gSkillData.plrDmg357 = 40;

	// MP5 Round
	gSkillData.plrDmgMP5 = 12;

	// M203 grenade
	gSkillData.plrDmgM203Grenade = 100;

	// Shotgun buckshot
	gSkillData.plrDmgBuckshot = 20;// fewer pellets in deathmatch

	// Crossbow
	gSkillData.plrDmgCrossbowClient = 20;

	// RPG
	gSkillData.plrDmgRPG = 120;

	// Egon
	gSkillData.plrDmgEgonWide = 20;
	gSkillData.plrDmgEgonNarrow = 10;

	// Hand Grendade
	gSkillData.plrDmgHandGrenade = 100;

	// Satchel Charge
	gSkillData.plrDmgSatchel = 120;

	// Tripmine
	gSkillData.plrDmgTripmine = 150;

	// hornet
	gSkillData.plrDmgHornet = 10;
}

// longest the intermission can last, in seconds
#define MAX_INTERMISSION_TIME		120

extern cvar_t timeleft, fragsleft;

extern cvar_t mp_chattime;

//=========================================================
//=========================================================
void CHalfLifeMultiplay :: Think ( void )
{
	g_VoiceGameMgr.Update(gpGlobals->frametime);

	///// Check game rules /////
	static int last_frags;
	static int last_time;

	int frags_remaining = 0;
	int time_remaining = 0;

	if ( g_fGameOver )   // someone else quit the game already
	{
		// bounds check
		int time = (int)CVAR_GET_FLOAT( "mp_chattime" );
		if ( time < 1 )
			CVAR_SET_STRING( "mp_chattime", "1" );
		else if ( time > MAX_INTERMISSION_TIME )
			CVAR_SET_STRING( "mp_chattime", UTIL_dtos1( MAX_INTERMISSION_TIME ) );

		m_flIntermissionEndTime = g_flIntermissionStartTime + mp_chattime.value;

		// check to see if we should change levels now
		if ( m_flIntermissionEndTime < gpGlobals->time )
		{
			if ( m_iEndIntermissionButtonHit  // check that someone has pressed a key, or the max intermission time is over
				|| ( ( g_flIntermissionStartTime + MAX_INTERMISSION_TIME ) < gpGlobals->time) ) 
				ChangeLevel(); // intermission is over
		}

		return;
	}

	float flTimeLimit = timelimit.value * 60;
	float flFragLimit = fraglimit.value;

	time_remaining = (int)(flTimeLimit ? ( flTimeLimit - gpGlobals->time ) : 0);
	
	if ( flTimeLimit != 0 && gpGlobals->time >= flTimeLimit )
	{
		GoToIntermission();
		return;
	}

	if ( flFragLimit )
	{
		int bestfrags = 9999;
		int remain;

		// check if any player is over the frag limit
		for ( int i = 1; i <= gpGlobals->maxClients; i++ )
		{
			CBaseEntity *pPlayer = UTIL_PlayerByIndex( i );

			if ( pPlayer && pPlayer->pev->frags >= flFragLimit )
			{
				GoToIntermission();
				return;
			}


			if ( pPlayer )
			{
				remain = static_cast<int>(flFragLimit - pPlayer->pev->frags);
				if ( remain < bestfrags )
				{
					bestfrags = remain;
				}
			}

		}
		frags_remaining = bestfrags;
	}

	// Updates when frags change
	if ( frags_remaining != last_frags )
	{
		g_engfuncs.pfnCvar_DirectSet( &fragsleft, UTIL_VarArgs( "%i", frags_remaining ) );
	}

	// Updates once per second
	if ( timeleft.value != last_time )
	{
		g_engfuncs.pfnCvar_DirectSet( &timeleft, UTIL_VarArgs( "%i", time_remaining ) );
	}

	last_frags = frags_remaining;
	last_time  = time_remaining;
}


//=========================================================
//=========================================================
BOOL CHalfLifeMultiplay::IsMultiplayer( void )
{
	return TRUE;
}

//=========================================================
//=========================================================
BOOL CHalfLifeMultiplay::IsDeathmatch( void )
{
	return TRUE;
}

//=========================================================
//=========================================================
BOOL CHalfLifeMultiplay::IsCoOp( void )
{
	return static_cast<BOOL>(gpGlobals->coop);
}

//=========================================================
//=========================================================
BOOL CHalfLifeMultiplay::FShouldSwitchWeapon( CBasePlayer *pPlayer, CBasePlayerItem *pWeapon )
{
	if ( !pWeapon->CanDeploy() )
	{
		// that weapon can't deploy anyway.
		return FALSE;
	}

	if ( !pPlayer->m_pActiveItem )
	{
		// player doesn't have an active item!
		return TRUE;
	}

	if ( !pPlayer->m_pActiveItem->CanHolster() )
	{
		// can't put away the active item.
		return FALSE;
	}

	if ( pWeapon->iWeight() > pPlayer->m_pActiveItem->iWeight() )
	{
		return TRUE;
	}

	return FALSE;
}

BOOL CHalfLifeMultiplay :: GetNextBestWeapon( CBasePlayer *pPlayer, CBasePlayerItem *pCurrentWeapon )
{

	CBasePlayerItem *pCheck;
	CBasePlayerItem *pBest;// this will be used in the event that we don't find a weapon in the same category.
	int iBestWeight;
	int i;

	iBestWeight = -1;// no weapon lower than -1 can be autoswitched to
	pBest = NULL;

	if ( !pCurrentWeapon->CanHolster() )
	{
		// can't put this gun away right now, so can't switch.
		return FALSE;
	}

	for ( i = 0 ; i < MAX_ITEM_TYPES ; i++ )
	{
		pCheck = pPlayer->m_rgpPlayerItems[ i ];

		while ( pCheck )
		{
			if ( pCheck->iWeight() > -1 && pCheck->iWeight() == pCurrentWeapon->iWeight() && pCheck != pCurrentWeapon )
			{
				// this weapon is from the same category. 
				if ( pCheck->CanDeploy() )
				{
					if ( pPlayer->SwitchWeapon( pCheck ) )
					{
						return TRUE;
					}
				}
			}
			else if ( pCheck->iWeight() > iBestWeight && pCheck != pCurrentWeapon )// don't reselect the weapon we're trying to get rid of
			{
				//ALERT ( at_console, "Considering %s\n", STRING( pCheck->pev->classname ) );
				// we keep updating the 'best' weapon just in case we can't find a weapon of the same weight
				// that the player was using. This will end up leaving the player with his heaviest-weighted 
				// weapon. 
				if ( pCheck->CanDeploy() )
				{
					// if this weapon is useable, flag it as the best
					iBestWeight = pCheck->iWeight();
					pBest = pCheck;
				}
			}

			pCheck = pCheck->m_pNext;
		}
	}

	// if we make it here, we've checked all the weapons and found no useable 
	// weapon in the same catagory as the current weapon. 
	
	// if pBest is null, we didn't find ANYTHING. Shouldn't be possible- should always 
	// at least get the crowbar, but ya never know.
	if ( !pBest )
	{
		return FALSE;
	}

	pPlayer->SwitchWeapon( pBest );

	return TRUE;
}

//=========================================================
//=========================================================
BOOL CHalfLifeMultiplay :: ClientConnected( edict_t *pEntity, const char *pszName, const char *pszAddress, char szRejectReason[ 128 ] )
{
	g_VoiceGameMgr.ClientConnected(pEntity);
	return TRUE;
}

extern int gmsgSayText;
extern int gmsgGameMode;

void CHalfLifeMultiplay :: UpdateGameMode( CBasePlayer *pPlayer )
{
	MESSAGE_BEGIN( MSG_ONE, gmsgGameMode, NULL, pPlayer->edict() );
		WRITE_BYTE( 0 );  // game mode none
	MESSAGE_END();
}

void CHalfLifeMultiplay :: InitHUD( CBasePlayer *pl )
{
	// notify other clients of player joining the game
	UTIL_ClientPrintAll( HUD_PRINTNOTIFY, UTIL_VarArgs( "%s has joined the game\n", 
		( pl->pev->netname && STRING(pl->pev->netname)[0] != 0 ) ? STRING(pl->pev->netname) : "unconnected" ) );

	// team match?
	if ( g_teamplay )
	{
		UTIL_LogPrintf( "\"%s<%i><%s><%s>\" entered the game\n",  
			STRING( pl->pev->netname ), 
			GETPLAYERUSERID( pl->edict() ),
			GETPLAYERAUTHID( pl->edict() ),
			g_engfuncs.pfnInfoKeyValue( g_engfuncs.pfnGetInfoKeyBuffer( pl->edict() ), "model" ) );
	}
	else
	{
		UTIL_LogPrintf( "\"%s<%i><%s><%i>\" entered the game\n",  
			STRING( pl->pev->netname ), 
			GETPLAYERUSERID( pl->edict() ),
			GETPLAYERAUTHID( pl->edict() ),
			GETPLAYERUSERID( pl->edict() ) );
	}

	UpdateGameMode( pl );

	// sending just one score makes the hud scoreboard active;  otherwise
	// it is just disabled for single play
	MESSAGE_BEGIN( MSG_ONE, gmsgScoreInfo, NULL, pl->edict() );
		WRITE_BYTE( ENTINDEX(pl->edict()) );
		WRITE_SHORT( 0 );
		WRITE_SHORT( 0 );
		WRITE_SHORT( 0 );
		WRITE_SHORT( 0 );
	MESSAGE_END();

	SendMOTDToClient( pl->edict() );

	// loop through all active players and send their score info to the new client
	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		// FIXME:  Probably don't need to cast this just to read m_iDeaths
		CBasePlayer *plr = (CBasePlayer *)UTIL_PlayerByIndex( i );

		if ( plr )
		{
			MESSAGE_BEGIN( MSG_ONE, gmsgScoreInfo, NULL, pl->edict() );
				WRITE_BYTE( i );	// client number
				WRITE_SHORT( static_cast<int>(plr->pev->frags) );
				WRITE_SHORT( plr->m_iDeaths );
				WRITE_SHORT( 0 );
				WRITE_SHORT( GetTeamIndex( plr->m_szTeamName ) + 1 );
			MESSAGE_END();
		}
	}

	if ( g_fGameOver )
	{
		MESSAGE_BEGIN( MSG_ONE, SVC_INTERMISSION, NULL, pl->edict() );
		MESSAGE_END();
	}
}

//=========================================================
//=========================================================
void CHalfLifeMultiplay :: ClientDisconnected( edict_t *pClient )
{
	if ( pClient )
	{
		CBasePlayer *pPlayer = (CBasePlayer *)CBaseEntity::Instance( pClient );

		if ( pPlayer )
		{
			FireTargets( "game_playerleave", pPlayer, pPlayer, USE_TOGGLE, 0 );

			// team match?
			if ( g_teamplay )
			{
				UTIL_LogPrintf( "\"%s<%i><%s><%s>\" disconnected\n",  
					STRING( pPlayer->pev->netname ), 
					GETPLAYERUSERID( pPlayer->edict() ),
					GETPLAYERAUTHID( pPlayer->edict() ),
					g_engfuncs.pfnInfoKeyValue( g_engfuncs.pfnGetInfoKeyBuffer( pPlayer->edict() ), "model" ) );
			}
			else
			{
				UTIL_LogPrintf( "\"%s<%i><%s><%i>\" disconnected\n",  
					STRING( pPlayer->pev->netname ), 
					GETPLAYERUSERID( pPlayer->edict() ),
					GETPLAYERAUTHID( pPlayer->edict() ),
					GETPLAYERUSERID( pPlayer->edict() ) );
			}

			pPlayer->RemoveAllItems( TRUE );// destroy all of the players weapons and items
		}
	}
}

//=========================================================
//=========================================================
float CHalfLifeMultiplay :: FlPlayerFallDamage( CBasePlayer *pPlayer )
{
	int iFallDamage = (int)falldamage.value;

	switch ( iFallDamage )
	{
	case 1://progressive
		pPlayer->m_flFallVelocity -= PLAYER_MAX_SAFE_FALL_SPEED;
		return pPlayer->m_flFallVelocity * DAMAGE_FOR_FALL_SPEED;
		break;
	default:
	case 0:// fixed
		return 10;
		break;
	}
} 

//=========================================================
//=========================================================
BOOL CHalfLifeMultiplay::FPlayerCanTakeDamage( CBasePlayer *pPlayer, CBaseEntity *pAttacker )
{
	return TRUE;
}

//=========================================================
//=========================================================
void CHalfLifeMultiplay :: PlayerThink( CBasePlayer *pPlayer )
{
	if ( g_fGameOver )
	{
		// check for button presses
		if ( pPlayer->m_afButtonPressed & ( IN_DUCK | IN_ATTACK | IN_ATTACK2 | IN_USE | IN_JUMP ) )
			m_iEndIntermissionButtonHit = TRUE;

		// clear attack/use commands from player
		pPlayer->m_afButtonPressed = 0;
		pPlayer->pev->button = 0;
		pPlayer->m_afButtonReleased = 0;
	}
}

//=========================================================
//=========================================================
void CHalfLifeMultiplay :: PlayerSpawn( CBasePlayer *pPlayer )
{
	BOOL		addDefault;
	CBaseEntity	*pWeaponEntity = NULL;

	pPlayer->pev->weapons |= (1<<WEAPON_SUIT);
	
	addDefault = TRUE;

	while (( pWeaponEntity = UTIL_FindEntityByClassname( pWeaponEntity, "game_player_equip" )))
	{
		pWeaponEntity->Touch( pPlayer );
		addDefault = FALSE;
	}

	if ( addDefault )
	{
		pPlayer->GiveNamedItem( "weapon_crowbar" );
		pPlayer->GiveNamedItem( "weapon_9mmhandgun" );
		pPlayer->GiveAmmo( 68, "9mm", _9MM_MAX_CARRY );// 4 full reloads
	}
}

//=========================================================
//=========================================================
float CHalfLifeMultiplay :: FlPlayerSpawnTime( CBasePlayer *pPlayer )
{
	return gpGlobals->time;//now!
}

BOOL CHalfLifeMultiplay :: AllowAutoTargetCrosshair( void )
{
	return ( aimcrosshair.value != 0 );
}

//=========================================================
// IPointsForKill - how many points awarded to anyone
// that kills this player?
//=========================================================
int CHalfLifeMultiplay :: IPointsForKill( CBasePlayer *pAttacker, CBasePlayer *pKilled )
{
	return 1;
}


//=========================================================
// PlayerKilled - someone/something killed this player
//=========================================================
void CHalfLifeMultiplay :: PlayerKilled( CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor )
{
	DeathNotice( pVictim, pKiller, pInflictor );

	pVictim->m_iDeaths += 1;


	FireTargets( "game_playerdie", pVictim, pVictim, USE_TOGGLE, 0 );
	CBasePlayer *peKiller = NULL;
	CBaseEntity *ktmp = CBaseEntity::Instance( pKiller );
	if ( ktmp && (ktmp->Classify() == CLASS_PLAYER) )
		peKiller = (CBasePlayer*)ktmp;

	if ( pVictim->pev == pKiller )  
	{  // killed self
		pKiller->frags -= 1;
	}
	else if ( ktmp && ktmp->IsPlayer() )
	{
		// if a player dies in a deathmatch game and the killer is a client, award the killer some points
		pKiller->frags += IPointsForKill( peKiller, pVictim );
		
		FireTargets( "game_playerkill", ktmp, ktmp, USE_TOGGLE, 0 );
	}
	else
	{  // killed by the world
		pKiller->frags -= 1;
	}

	// update the scores
	// killed scores
	MESSAGE_BEGIN( MSG_ALL, gmsgScoreInfo );
		WRITE_BYTE( ENTINDEX(pVictim->edict()) );
		WRITE_SHORT( static_cast<int>(pVictim->pev->frags) );
		WRITE_SHORT( pVictim->m_iDeaths );
		WRITE_SHORT( 0 );
		WRITE_SHORT( GetTeamIndex( pVictim->m_szTeamName ) + 1 );
	MESSAGE_END();

	// killers score, if it's a player
	CBaseEntity *ep = CBaseEntity::Instance( pKiller );
	if ( ep && ep->Classify() == CLASS_PLAYER )
	{
		CBasePlayer *PK = (CBasePlayer*)ep;

		MESSAGE_BEGIN( MSG_ALL, gmsgScoreInfo );
			WRITE_BYTE( ENTINDEX(PK->edict()) );
			WRITE_SHORT( static_cast<int>(PK->pev->frags) );
			WRITE_SHORT( PK->m_iDeaths );
			WRITE_SHORT( 0 );
			WRITE_SHORT( GetTeamIndex( PK->m_szTeamName) + 1 );
		MESSAGE_END();

		// let the killer paint another decal as soon as he'd like.
		PK->m_flNextDecalTime = gpGlobals->time;
	}
}

//=========================================================
// Deathnotice. 
//=========================================================
void CHalfLifeMultiplay::DeathNotice( CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pevInflictor )
{
	// Work out what killed the player, and send a message to all clients about it
	CBaseEntity::Instance( pKiller );

	const char *killer_weapon_name = "world";		// by default, the player is killed by the world
	int killer_index = 0;
	
	// Hack to fix name change
	char *tau = "tau_cannon";
	char *gluon = "gluon gun";

	if ( pKiller->flags & FL_CLIENT )
	{
		killer_index = ENTINDEX(ENT(pKiller));
		
		if ( pevInflictor )
		{
			if ( pevInflictor == pKiller )
			{
				// If the inflictor is the killer,  then it must be their current weapon doing the damage
				CBasePlayer *pPlayer = (CBasePlayer*)CBaseEntity::Instance( pKiller );
				
				if ( pPlayer->m_pActiveItem )
				{
					killer_weapon_name = pPlayer->m_pActiveItem->pszName();
				}
			}
			else
			{
				killer_weapon_name = STRING( pevInflictor->classname );  // it's just that easy
			}
		}
	}
	else
	{
		killer_weapon_name = STRING( pevInflictor->classname );
	}

	// strip the monster_* or weapon_* from the inflictor's classname
	if ( strncmp( killer_weapon_name, "weapon_", 7 ) == 0 )
		killer_weapon_name += 7;
	else if ( strncmp( killer_weapon_name, "monster_", 8 ) == 0 )
		killer_weapon_name += 8;
	else if ( strncmp( killer_weapon_name, "func_", 5 ) == 0 )
		killer_weapon_name += 5;

	MESSAGE_BEGIN( MSG_ALL, gmsgDeathMsg );
		WRITE_BYTE( killer_index );						// the killer
		WRITE_BYTE( ENTINDEX(pVictim->edict()) );		// the victim
		WRITE_STRING( killer_weapon_name );		// what they were killed by (should this be a string?)
	MESSAGE_END();

	// replace the code names with the 'real' names
	if ( !strcmp( killer_weapon_name, "egon" ) )
		killer_weapon_name = gluon;
	else if ( !strcmp( killer_weapon_name, "gauss" ) )
		killer_weapon_name = tau;

	if ( pVictim->pev == pKiller )  
	{
		// killed self

		// team match?
		if ( g_teamplay )
		{
			UTIL_LogPrintf( "\"%s<%i><%s><%s>\" committed suicide with \"%s\"\n",  
				STRING( pVictim->pev->netname ), 
				GETPLAYERUSERID( pVictim->edict() ),
				GETPLAYERAUTHID( pVictim->edict() ),
				g_engfuncs.pfnInfoKeyValue( g_engfuncs.pfnGetInfoKeyBuffer( pVictim->edict() ), "model" ),
				killer_weapon_name );		
		}
		else
		{
			UTIL_LogPrintf( "\"%s<%i><%s><%i>\" committed suicide with \"%s\"\n",  
				STRING( pVictim->pev->netname ), 
				GETPLAYERUSERID( pVictim->edict() ),
				GETPLAYERAUTHID( pVictim->edict() ),
				GETPLAYERUSERID( pVictim->edict() ),
				killer_weapon_name );		
		}
	}
	else if ( pKiller->flags & FL_CLIENT )
	{
		// team match?
		if ( g_teamplay )
		{
			UTIL_LogPrintf( "\"%s<%i><%s><%s>\" killed \"%s<%i><%s><%s>\" with \"%s\"\n",  
				STRING( pKiller->netname ),
				GETPLAYERUSERID( ENT(pKiller) ),
				GETPLAYERAUTHID( ENT(pKiller) ),
				g_engfuncs.pfnInfoKeyValue( g_engfuncs.pfnGetInfoKeyBuffer( ENT(pKiller) ), "model" ),
				STRING( pVictim->pev->netname ),
				GETPLAYERUSERID( pVictim->edict() ),
				GETPLAYERAUTHID( pVictim->edict() ),
				g_engfuncs.pfnInfoKeyValue( g_engfuncs.pfnGetInfoKeyBuffer( pVictim->edict() ), "model" ),
				killer_weapon_name );
		}
		else
		{
			UTIL_LogPrintf( "\"%s<%i><%s><%i>\" killed \"%s<%i><%s><%i>\" with \"%s\"\n",  
				STRING( pKiller->netname ),
				GETPLAYERUSERID( ENT(pKiller) ),
				GETPLAYERAUTHID( ENT(pKiller) ),
				GETPLAYERUSERID( ENT(pKiller) ),
				STRING( pVictim->pev->netname ),
				GETPLAYERUSERID( pVictim->edict() ),
				GETPLAYERAUTHID( pVictim->edict() ),
				GETPLAYERUSERID( pVictim->edict() ),
				killer_weapon_name );
		}
	}
	else
	{ 
		// killed by the world

		// team match?
		if ( g_teamplay )
		{
			UTIL_LogPrintf( "\"%s<%i><%s><%s>\" committed suicide with \"%s\" (world)\n",
				STRING( pVictim->pev->netname ), 
				GETPLAYERUSERID( pVictim->edict() ), 
				GETPLAYERAUTHID( pVictim->edict() ),
				g_engfuncs.pfnInfoKeyValue( g_engfuncs.pfnGetInfoKeyBuffer( pVictim->edict() ), "model" ),
				killer_weapon_name );		
		}
		else
		{
			UTIL_LogPrintf( "\"%s<%i><%s><%i>\" committed suicide with \"%s\" (world)\n",
				STRING( pVictim->pev->netname ), 
				GETPLAYERUSERID( pVictim->edict() ), 
				GETPLAYERAUTHID( pVictim->edict() ),
				GETPLAYERUSERID( pVictim->edict() ),
				killer_weapon_name );		
		}
	}

	MESSAGE_BEGIN( MSG_SPEC, SVC_DIRECTOR );
		WRITE_BYTE ( 9 );	// command length in bytes
		WRITE_BYTE ( DRC_CMD_EVENT );	// player killed
		WRITE_SHORT( ENTINDEX(pVictim->edict()) );	// index number of primary entity
		if (pevInflictor)
			WRITE_SHORT( ENTINDEX(ENT(pevInflictor)) );	// index number of secondary entity
		else
			WRITE_SHORT( ENTINDEX(ENT(pKiller)) );	// index number of secondary entity
		WRITE_LONG( 7 | DRC_FLAG_DRAMATIC);   // eventflags (priority and flags)
	MESSAGE_END();

//  Print a standard message
	// TODO: make this go direct to console
	return; // just remove for now
/*
	char	szText[ 128 ];

	if ( pKiller->flags & FL_MONSTER )
	{
		// killed by a monster
		strcpy ( szText, STRING( pVictim->pev->netname ) );
		strcat ( szText, " was killed by a monster.\n" );
		return;
	}

	if ( pKiller == pVictim->pev )
	{
		strcpy ( szText, STRING( pVictim->pev->netname ) );
		strcat ( szText, " commited suicide.\n" );
	}
	else if ( pKiller->flags & FL_CLIENT )
	{
		strcpy ( szText, STRING( pKiller->netname ) );

		strcat( szText, " : " );
		strcat( szText, killer_weapon_name );
		strcat( szText, " : " );

		strcat ( szText, STRING( pVictim->pev->netname ) );
		strcat ( szText, "\n" );
	}
	else if ( FClassnameIs ( pKiller, "worldspawn" ) )
	{
		strcpy ( szText, STRING( pVictim->pev->netname ) );
		strcat ( szText, " fell or drowned or something.\n" );
	}
	else if ( pKiller->solid == SOLID_BSP )
	{
		strcpy ( szText, STRING( pVictim->pev->netname ) );
		strcat ( szText, " was mooshed.\n" );
	}
	else
	{
		strcpy ( szText, STRING( pVictim->pev->netname ) );
		strcat ( szText, " died mysteriously.\n" );
	}

	UTIL_ClientPrintAll( szText );
*/
}

//=========================================================
// PlayerGotWeapon - player has grabbed a weapon that was
// sitting in the world
//=========================================================
void CHalfLifeMultiplay :: PlayerGotWeapon( CBasePlayer *pPlayer, CBasePlayerItem *pWeapon )
{
}

//=========================================================
// FlWeaponRespawnTime - what is the time in the future
// at which this weapon may spawn?
//=========================================================
float CHalfLifeMultiplay :: FlWeaponRespawnTime( CBasePlayerItem *pWeapon )
{
	if ( weaponstay.value > 0 )
	{
		// make sure it's only certain weapons
		if ( !(pWeapon->iFlags() & ITEM_FLAG_LIMITINWORLD) )
		{
			return gpGlobals->time + 0;		// weapon respawns almost instantly
		}
	}

	return gpGlobals->time + WEAPON_RESPAWN_TIME;
}

// when we are within this close to running out of entities,  items 
// marked with the ITEM_FLAG_LIMITINWORLD will delay their respawn
#define ENTITY_INTOLERANCE	100

//=========================================================
// FlWeaponRespawnTime - Returns 0 if the weapon can respawn 
// now,  otherwise it returns the time at which it can try
// to spawn again.
//=========================================================
float CHalfLifeMultiplay :: FlWeaponTryRespawn( CBasePlayerItem *pWeapon )
{
	if ( pWeapon && pWeapon->m_iId && (pWeapon->iFlags() & ITEM_FLAG_LIMITINWORLD) )
	{
		if ( NUMBER_OF_ENTITIES() < (gpGlobals->maxEntities - ENTITY_INTOLERANCE) )
			return 0;

		// we're past the entity tolerance level,  so delay the respawn
		return FlWeaponRespawnTime( pWeapon );
	}

	return 0;
}

//=========================================================
// VecWeaponRespawnSpot - where should this weapon spawn?
// Some game variations may choose to randomize spawn locations
//=========================================================
Vector CHalfLifeMultiplay :: VecWeaponRespawnSpot( CBasePlayerItem *pWeapon )
{
	return pWeapon->pev->origin;
}

//=========================================================
// WeaponShouldRespawn - any conditions inhibiting the
// respawning of this weapon?
//=========================================================
int CHalfLifeMultiplay :: WeaponShouldRespawn( CBasePlayerItem *pWeapon )
{
	if ( pWeapon->pev->spawnflags & SF_NORESPAWN )
	{
		return GR_WEAPON_RESPAWN_NO;
	}

	return GR_WEAPON_RESPAWN_YES;
}

//=========================================================
// CanHaveWeapon - returns FALSE if the player is not allowed
// to pick up this weapon
//=========================================================
BOOL CHalfLifeMultiplay::CanHavePlayerItem( CBasePlayer *pPlayer, CBasePlayerItem *pItem )
{
	if ( weaponstay.value > 0 )
	{
		if ( pItem->iFlags() & ITEM_FLAG_LIMITINWORLD )
			return CGameRules::CanHavePlayerItem( pPlayer, pItem );

		// check if the player already has this weapon
		for ( int i = 0 ; i < MAX_ITEM_TYPES ; i++ )
		{
			CBasePlayerItem *it = pPlayer->m_rgpPlayerItems[i];

			while ( it != NULL )
			{
				if ( it->m_iId == pItem->m_iId )
				{
					return FALSE;
				}

				it = it->m_pNext;
			}
		}
	}

	return CGameRules::CanHavePlayerItem( pPlayer, pItem );
}

//=========================================================
//=========================================================
BOOL CHalfLifeMultiplay::CanHaveItem( CBasePlayer *pPlayer, CItem *pItem )
{
	return TRUE;
}

//=========================================================
//=========================================================
void CHalfLifeMultiplay::PlayerGotItem( CBasePlayer *pPlayer, CItem *pItem )
{
}

//=========================================================
//=========================================================
int CHalfLifeMultiplay::ItemShouldRespawn( CItem *pItem )
{
	if ( pItem->pev->spawnflags & SF_NORESPAWN )
	{
		return GR_ITEM_RESPAWN_NO;
	}

	return GR_ITEM_RESPAWN_YES;
}


//=========================================================
// At what time in the future may this Item respawn?
//=========================================================
float CHalfLifeMultiplay::FlItemRespawnTime( CItem *pItem )
{
	return gpGlobals->time + ITEM_RESPAWN_TIME;
}

//=========================================================
// Where should this item respawn?
// Some game variations may choose to randomize spawn locations
//=========================================================
Vector CHalfLifeMultiplay::VecItemRespawnSpot( CItem *pItem )
{
	return pItem->pev->origin;
}

//=========================================================
//=========================================================
void CHalfLifeMultiplay::PlayerGotAmmo( CBasePlayer *pPlayer, char *szName, int iCount )
{
}

//=========================================================
//=========================================================
BOOL CHalfLifeMultiplay::IsAllowedToSpawn( CBaseEntity *pEntity )
{
//	if ( pEntity->pev->flags & FL_MONSTER )
//		return FALSE;

	return TRUE;
}

//=========================================================
//=========================================================
int CHalfLifeMultiplay::AmmoShouldRespawn( CBasePlayerAmmo *pAmmo )
{
	if ( pAmmo->pev->spawnflags & SF_NORESPAWN )
	{
		return GR_AMMO_RESPAWN_NO;
	}

	return GR_AMMO_RESPAWN_YES;
}

//=========================================================
//=========================================================
float CHalfLifeMultiplay::FlAmmoRespawnTime( CBasePlayerAmmo *pAmmo )
{
	return gpGlobals->time + AMMO_RESPAWN_TIME;
}

//=========================================================
//=========================================================
Vector CHalfLifeMultiplay::VecAmmoRespawnSpot( CBasePlayerAmmo *pAmmo )
{
	return pAmmo->pev->origin;
}

//=========================================================
//=========================================================
float CHalfLifeMultiplay::FlHealthChargerRechargeTime( void )
{
	return 60;
}


float CHalfLifeMultiplay::FlHEVChargerRechargeTime( void )
{
	return 30;
}

//=========================================================
//=========================================================
int CHalfLifeMultiplay::DeadPlayerWeapons( CBasePlayer *pPlayer )
{
	return GR_PLR_DROP_GUN_ACTIVE;
}

//=========================================================
//=========================================================
int CHalfLifeMultiplay::DeadPlayerAmmo( CBasePlayer *pPlayer )
{
	return GR_PLR_DROP_AMMO_ACTIVE;
}

edict_t *CHalfLifeMultiplay::GetPlayerSpawnSpot( CBasePlayer *pPlayer )
{
	edict_t *pentSpawnSpot = CGameRules::GetPlayerSpawnSpot( pPlayer );	
	if ( IsMultiplayer() && pentSpawnSpot->v.target )
	{
		FireTargets( STRING(pentSpawnSpot->v.target), pPlayer, pPlayer, USE_TOGGLE, 0 );
	}

	return pentSpawnSpot;
}


//=========================================================
//=========================================================
int CHalfLifeMultiplay::PlayerRelationship( CBaseEntity *pPlayer, CBaseEntity *pTarget )
{
	// half life deathmatch has only enemies
	return GR_NOTTEAMMATE;
}

/*BOOL CHalfLifeMultiplay :: PlayFootstepSounds( CBasePlayer *pl, float fvol )
{
	if ( g_footsteps && g_footsteps->value == 0 )
		return FALSE;

	if ( pl->IsOnLadder() || pl->pev->velocity.Length2D() > 220 )
		return TRUE;  // only make step sounds in multiplayer if the player is moving fast enough

	return FALSE;
}*/

BOOL CHalfLifeMultiplay :: FAllowFlashlight( void ) 
{ 
	return flashlight.value != 0; 
}

//=========================================================
//=========================================================
BOOL CHalfLifeMultiplay :: FAllowMonsters( void )
{
	return ( allowmonsters.value != 0 );
}

//=========================================================
//======== CHalfLifeMultiplay private functions ===========
#define INTERMISSION_TIME		6

void CHalfLifeMultiplay :: GoToIntermission( void )
{
	if ( g_fGameOver )
		return;  // intermission has already been triggered, so ignore.

	MESSAGE_BEGIN(MSG_ALL, SVC_INTERMISSION);
	MESSAGE_END();

	// bounds check
	int time = (int)CVAR_GET_FLOAT( "mp_chattime" );
	if ( time < 1 )
		CVAR_SET_STRING( "mp_chattime", "1" );
	else if ( time > MAX_INTERMISSION_TIME )
		CVAR_SET_STRING( "mp_chattime", UTIL_dtos1( MAX_INTERMISSION_TIME ) );

	m_flIntermissionEndTime = gpGlobals->time + ( (int)mp_chattime.value );
	g_flIntermissionStartTime = gpGlobals->time;

	g_fGameOver = TRUE;
	m_iEndIntermissionButtonHit = FALSE;
}

#define MAX_RULE_BUFFER 1024

typedef struct mapcycle_item_s
{
	struct mapcycle_item_s *next;

	char mapname[ 32 ];
	int  minplayers, maxplayers;
	char rulebuffer[ MAX_RULE_BUFFER ];
} mapcycle_item_t;

typedef struct mapcycle_s
{
	struct mapcycle_item_s *items;
	struct mapcycle_item_s *next_item;
} mapcycle_t;

/*
==============
DestroyMapCycle

Clean up memory used by mapcycle when switching it
==============
*/
void DestroyMapCycle( mapcycle_t *cycle )
{
	mapcycle_item_t *p, *n, *start;
	p = cycle->items;
	if ( p )
	{
		start = p;
		p = p->next;
		while ( p != start )
		{
			n = p->next;
			delete p;
			p = n;
		}
		
		delete cycle->items;
	}
	cycle->items = NULL;
	cycle->next_item = NULL;
}

static char com_token[ 1500 ];

/*
==============
COM_Parse

Parse a token out of a string
==============
*/
char *COM_Parse (char *data)
{
	int             c;
	int             len;
	
	len = 0;
	com_token[0] = 0;
	
	if (!data)
		return NULL;
		
// skip whitespace
skipwhite:
	while ( (c = *data) <= ' ')
	{
		if (c == 0)
			return NULL;                    // end of file;
		data++;
	}
	
// skip // comments
	if (c=='/' && data[1] == '/')
	{
		while (*data && *data != '\n')
			data++;
		goto skipwhite;
	}
	

// handle quoted strings specially
	if (c == '\"')
	{
		data++;
		while (1)
		{
			c = *data++;
			if (c=='\"' || !c)
			{
				com_token[len] = 0;
				return data;
			}
			com_token[len] = c;
			len++;
		}
	}

// parse single characters
	if (c=='{' || c=='}'|| c==')'|| c=='(' || c=='\'' || c == ',' )
	{
		com_token[len] = c;
		len++;
		com_token[len] = 0;
		return data+1;
	}

// parse a regular word
	do
	{
		com_token[len] = c;
		data++;
		len++;
		c = *data;
	if (c=='{' || c=='}'|| c==')'|| c=='(' || c=='\'' || c == ',' )
			break;
	} while (c>32);
	
	com_token[len] = 0;
	return data;
}

/*
==============
COM_TokenWaiting

Returns 1 if additional data is waiting to be processed on this line
==============
*/
int COM_TokenWaiting( char *buffer )
{
	char *p;

	p = buffer;
	while ( *p && *p!='\n')
	{
		if ( !isspace( *p ) || isalnum( *p ) )
			return 1;

		p++;
	}

	return 0;
}



/*
==============
ReloadMapCycleFile


Parses mapcycle.txt file into mapcycle_t structure
==============
*/
int ReloadMapCycleFile( char *filename, mapcycle_t *cycle )
{
	char szBuffer[ MAX_RULE_BUFFER ];
	char szMap[ 32 ];
	int length;
	char *pFileList;
	char *aFileList = pFileList = (char*)LOAD_FILE_FOR_ME( filename, &length );
	int hasbuffer;
	mapcycle_item_s *item, *newlist = NULL, *next;

	if ( pFileList && length )
	{
		// the first map name in the file becomes the default
		while ( 1 )
		{
			hasbuffer = 0;
			memset( szBuffer, 0, MAX_RULE_BUFFER );

			pFileList = COM_Parse( pFileList );
			if ( strlen( com_token ) <= 0 )
				break;

			strcpy( szMap, com_token );

			// Any more tokens on this line?
			if ( COM_TokenWaiting( pFileList ) )
			{
				pFileList = COM_Parse( pFileList );
				if ( strlen( com_token ) > 0 )
				{
					hasbuffer = 1;
					strcpy( szBuffer, com_token );
				}
			}

			// Check map
			if ( IS_MAP_VALID( szMap ) )
			{
				// Create entry
				char *s;

				item = new mapcycle_item_s;

				strcpy( item->mapname, szMap );

				item->minplayers = 0;
				item->maxplayers = 0;

				memset( item->rulebuffer, 0, MAX_RULE_BUFFER );

				if ( hasbuffer )
				{
					s = g_engfuncs.pfnInfoKeyValue( szBuffer, "minplayers" );
					if ( s && s[0] )
					{
						item->minplayers = atoi( s );
						item->minplayers = max( item->minplayers, 0 );
						item->minplayers = min( item->minplayers, gpGlobals->maxClients );
					}
					s = g_engfuncs.pfnInfoKeyValue( szBuffer, "maxplayers" );
					if ( s && s[0] )
					{
						item->maxplayers = atoi( s );
						item->maxplayers = max( item->maxplayers, 0 );
						item->maxplayers = min( item->maxplayers, gpGlobals->maxClients );
					}

					// Remove keys
					//
					g_engfuncs.pfnInfo_RemoveKey( szBuffer, "minplayers" );
					g_engfuncs.pfnInfo_RemoveKey( szBuffer, "maxplayers" );

					strcpy( item->rulebuffer, szBuffer );
				}

				item->next = cycle->items;
				cycle->items = item;
			}
			else
			{
				ALERT( at_console, "Skipping %s from mapcycle, not a valid map\n", szMap );
			}

		}

		FREE_FILE( aFileList );
	}

	// Fixup circular list pointer
	item = cycle->items;

	// Reverse it to get original order
	while ( item )
	{
		next = item->next;
		item->next = newlist;
		newlist = item;
		item = next;
	}
	cycle->items = newlist;
	item = cycle->items;

	// Didn't parse anything
	if ( !item )
	{
		return 0;
	}

	while ( item->next )
	{
		item = item->next;
	}
	item->next = cycle->items;
	
	cycle->next_item = item->next;

	return 1;
}

/*
==============
CountPlayers

Determine the current # of active players on the server for map cycling logic
==============
*/
int CountPlayers( void )
{
	int	num = 0;

	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CBaseEntity *pEnt = UTIL_PlayerByIndex( i );

		if ( pEnt )
		{
			num = num + 1;
		}
	}

	return num;
}

/*
==============
ExtractCommandString

Parse commands/key value pairs to issue right after map xxx command is issued on server
 level transition
==============
*/
void ExtractCommandString( char *s, char *szCommand )
{
	// Now make rules happen
	char	pkey[512];
	char	value[512];	// use two buffers so compares
								// work without stomping on each other
	char	*o;
	
	if ( *s == '\\' )
		s++;

	while (1)
	{
		o = pkey;
		while ( *s != '\\' )
		{
			if ( !*s )
				return;
			*o++ = *s++;
		}
		*o = 0;
		s++;

		o = value;

		while (*s != '\\' && *s)
		{
			if (!*s)
				return;
			*o++ = *s++;
		}
		*o = 0;

		strcat( szCommand, pkey );
		if ( strlen( value ) > 0 )
		{
			strcat( szCommand, " " );
			strcat( szCommand, value );
		}
		strcat( szCommand, "\n" );

		if (!*s)
			return;
		s++;
	}
}

/*
==============
ChangeLevel

Server is changing to a new level, check mapcycle.txt for map name and setup info
==============
*/
void CHalfLifeMultiplay :: ChangeLevel( void )
{
	static char szPreviousMapCycleFile[ 256 ];
	static mapcycle_t mapcycle;

	char szNextMap[32];
	char szFirstMapInList[32];
	char szCommands[ 1500 ];
	char szRules[ 1500 ];
	int minplayers = 0, maxplayers = 0;
	strcpy( szFirstMapInList, "hldm1" );  // the absolute default level is hldm1

	int	curplayers;
	BOOL do_cycle = TRUE;

	// find the map to change to
	char *mapcfile = (char*)CVAR_GET_STRING( "mapcyclefile" );
	ASSERT( mapcfile != NULL );

	szCommands[ 0 ] = '\0';
	szRules[ 0 ] = '\0';

	curplayers = CountPlayers();

	// Has the map cycle filename changed?
	if ( stricmp( mapcfile, szPreviousMapCycleFile ) )
	{
		strcpy( szPreviousMapCycleFile, mapcfile );

		DestroyMapCycle( &mapcycle );

		if ( !ReloadMapCycleFile( mapcfile, &mapcycle ) || ( !mapcycle.items ) )
		{
			ALERT( at_console, "Unable to load map cycle file %s\n", mapcfile );
			do_cycle = FALSE;
		}
	}

	if ( do_cycle && mapcycle.items )
	{
		BOOL keeplooking = FALSE;
		BOOL found = FALSE;
		mapcycle_item_s *item;

		// Assume current map
		strcpy( szNextMap, STRING(gpGlobals->mapname) );
		strcpy( szFirstMapInList, STRING(gpGlobals->mapname) );

		// Traverse list
		for ( item = mapcycle.next_item; item->next != mapcycle.next_item; item = item->next )
		{
			keeplooking = FALSE;

			ASSERT( item != NULL );

			if ( item->minplayers != 0 )
			{
				if ( curplayers >= item->minplayers )
				{
					found = TRUE;
					minplayers = item->minplayers;
				}
				else
				{
					keeplooking = TRUE;
				}
			}

			if ( item->maxplayers != 0 )
			{
				if ( curplayers <= item->maxplayers )
				{
					found = TRUE;
					maxplayers = item->maxplayers;
				}
				else
				{
					keeplooking = TRUE;
				}
			}

			if ( keeplooking )
				continue;

			found = TRUE;
			break;
		}

		if ( !found )
		{
			item = mapcycle.next_item;
		}			
		
		// Increment next item pointer
		mapcycle.next_item = item->next;

		// Perform logic on current item
		strcpy( szNextMap, item->mapname );

		ExtractCommandString( item->rulebuffer, szCommands );
		strcpy( szRules, item->rulebuffer );
	}

	if ( !IS_MAP_VALID(szNextMap) )
	{
		strcpy( szNextMap, szFirstMapInList );
	}

	g_fGameOver = TRUE;

	ALERT( at_console, "CHANGE LEVEL: %s\n", szNextMap );
	if ( minplayers || maxplayers )
	{
		ALERT( at_console, "PLAYER COUNT:  min %i max %i current %i\n", minplayers, maxplayers, curplayers );
	}
	if ( strlen( szRules ) > 0 )
	{
		ALERT( at_console, "RULES:  %s\n", szRules );
	}
	
	CHANGE_LEVEL( szNextMap, NULL );
	if ( strlen( szCommands ) > 0 )
	{
		SERVER_COMMAND( szCommands );
	}
}

#define MAX_MOTD_CHUNK	  60
#define MAX_MOTD_LENGTH   1536 // (MAX_MOTD_CHUNK * 4)

void CHalfLifeMultiplay :: SendMOTDToClient( edict_t *client )
{
	// read from the MOTD.txt file
	int length, char_count = 0;
	char *pFileList;
	char *aFileList = pFileList = (char*)LOAD_FILE_FOR_ME( (char *)CVAR_GET_STRING( "motdfile" ), &length );

	// send the server name
	MESSAGE_BEGIN( MSG_ONE, gmsgServerName, NULL, client );
		WRITE_STRING( CVAR_GET_STRING("hostname") );
	MESSAGE_END();

	// Send the message of the day
	// read it chunk-by-chunk,  and send it in parts

	while ( pFileList && *pFileList && char_count < MAX_MOTD_LENGTH )
	{
		char chunk[MAX_MOTD_CHUNK+1];
		
		if ( strlen( pFileList ) < MAX_MOTD_CHUNK )
		{
			strcpy( chunk, pFileList );
		}
		else
		{
			strncpy( chunk, pFileList, MAX_MOTD_CHUNK );
			chunk[MAX_MOTD_CHUNK] = 0;		// strncpy doesn't always append the null terminator
		}

		char_count += strlen( chunk );
		if ( char_count < MAX_MOTD_LENGTH )
			pFileList = aFileList + char_count; 
		else
			*pFileList = 0;

		MESSAGE_BEGIN( MSG_ONE, gmsgMOTD, NULL, client );
			WRITE_BYTE( *pFileList ? FALSE : TRUE );	// FALSE means there is still more message to come
			WRITE_STRING( chunk );
		MESSAGE_END();
	}

	FREE_FILE( aFileList );
}


BOOL CHalfLifeMultiplay::ClientCommand_DeadOrAlive( CBasePlayer *pPlayer, const char *pcmd ) 
{
    return m_VoiceGameMgr.ClientCommand( pPlayer, pcmd ) ? TRUE : FALSE;
}

void CHalfLifeMultiplay::EndMultiplayerGame()
{
    GoToIntermission();
}

void CHalfLifeMultiplay::RemoveGuns()
{
    CBaseEntity *pEntity = NULL;
    
    while( ( pEntity = UTIL_FindEntityByClassname( pEntity, "weaponbox" ) ) != NULL )
    {
        ( ( CWeaponBox* )pEntity )->Kill();
    }

    while( ( pEntity = UTIL_FindEntityByClassname( pEntity, "weapon_shield" ) ) != NULL )
    {
        pEntity->SetThink( &CBaseEntity::SUB_Remove );
        pEntity->pev->nextthink = gpGlobals->time + 0.1;
    }
}

// CS

void CHalfLifeMultiplay::BalanceTeams( void )
{
    TeamName teamToSwap = UNASSIGNED;
    int numPlayerToSwap;

    if( m_iMapHasVIPSafetyZone == 1 )
    {
        int idealNumCT = ( ( m_iNumCT + m_iNumTerrorist ) % 2 != 0 ) ? ( int )( ( m_iNumCT + m_iNumTerrorist ) * 0.55 ) + 1 : ( int )( ( m_iNumCT + m_iNumTerrorist ) * 0.5 );
        int idealNumTerrorist = ( m_iNumCT + m_iNumTerrorist ) - idealNumCT;

        if( m_iNumCT < idealNumCT )
        {
            teamToSwap = TERRORIST;
            numPlayerToSwap = idealNumCT - m_iNumCT;
        }
        else if( m_iNumTerrorist < idealNumTerrorist )
        {
            teamToSwap = CT;
            numPlayerToSwap = idealNumTerrorist - m_iNumTerrorist;
        }
        else
            return;
    }
    else
    {
        if( m_iNumCT > m_iNumTerrorist )
        {
            teamToSwap = CT;
            numPlayerToSwap = ( m_iNumCT - m_iNumTerrorist ) * 0.5;
        }
        else if( m_iNumTerrorist > m_iNumCT )
        {
            teamToSwap = TERRORIST;
            numPlayerToSwap = ( m_iNumTerrorist - m_iNumCT ) * 0.5;
        }
        else
            return;
    }

    if( numPlayerToSwap > 4 )
        numPlayerToSwap = 4;

    int highestUserID = -1;

    CBasePlayer *toSwap  = NULL;
    CBaseEntity *pEntity = NULL;

    for( int i = 1; i <= numPlayerToSwap; i++ )
    {
        highestUserID = 0;
        toSwap = NULL;

        while( ( pEntity = UTIL_FindEntityByClassname( pEntity, "player" ) ) != NULL )
        {
            if( FNullEnt( pEntity->edict() ) )
                break;

            if( pEntity->IsDormant() )
                continue;

            CBasePlayer *pPlayer = GetClassPtr( ( CBasePlayer* )pEntity->pev );

            if( pPlayer->m_iTeam == teamToSwap && GETPLAYERUSERID( pPlayer->edict() ) > highestUserID && m_pVIP != pPlayer )
            {
                highestUserID = GETPLAYERUSERID( pPlayer->edict() );
                toSwap = pPlayer;
            }
        }

        if( toSwap != NULL )
            toSwap->SwitchTeam();
    }
}

// CS
void CHalfLifeMultiplay::CleanUpMap()
{
    CBaseEntity *pEntity = NULL;

    while( ( pEntity = UTIL_FindEntityByClassname( pEntity, "cycler_sprite" ) ) != NULL )
        pEntity->Restart();

    while( ( pEntity = UTIL_FindEntityByClassname( pEntity, "light" ) ) != NULL )
        pEntity->Restart();

    while( ( pEntity = UTIL_FindEntityByClassname( pEntity, "func_door" ) ) != NULL )
        pEntity->Restart();

    while( ( pEntity = UTIL_FindEntityByClassname( pEntity, "func_water" ) ) != NULL )
        pEntity->Restart();

    while( ( pEntity = UTIL_FindEntityByClassname( pEntity, "func_door_rotating" ) ) != NULL )
        pEntity->Restart();

    while( ( pEntity = UTIL_FindEntityByClassname( pEntity, "func_tracktrain" ) ) != NULL )
        pEntity->Restart();

    while( ( pEntity = UTIL_FindEntityByClassname( pEntity, "func_vehicle" ) ) != NULL )
        pEntity->Restart();

    while( ( pEntity = UTIL_FindEntityByClassname( pEntity, "func_train" ) ) != NULL )
        pEntity->Restart();

    while( ( pEntity = UTIL_FindEntityByClassname( pEntity, "armoury_entity" ) ) != NULL )
        pEntity->Restart();

    while( ( pEntity = UTIL_FindEntityByClassname( pEntity, "ambient_generic" ) ) != NULL )
        pEntity->Restart();

    while( ( pEntity = UTIL_FindEntityByClassname( pEntity, "env_sprite" ) ) != NULL )
        pEntity->Restart();

    while( ( pEntity = UTIL_FindEntityByClassname( pEntity, "grenade" ) ) != NULL )
        UTIL_Remove( pEntity );

    while( ( pEntity = UTIL_FindEntityByClassname( pEntity, "item_thighpack" ) ) != NULL )
        UTIL_Remove( pEntity );

    RemoveGuns();

    // TODO: Finish me. 
    // PLAYBACK_EVENT_FULL( 6, 0, m_usResetDecals, 0, g_vecZero, g_vecZero, 0, 0, 0, 0, 0, 0 );
}

void CHalfLifeMultiplay::CheckMapConditions()
{
    if( UTIL_FindEntityByClassname( NULL, "func_bomb_target" ) != NULL )
    {
        m_bMapHasBombTarget = true;
        m_bMapHasBombZone   = true;
    }
    else if( UTIL_FindEntityByClassname( NULL, "info_bomb_target" ) != NULL )
    {
        m_bMapHasBombTarget = true;
        m_bMapHasBombZone   = false;
    }
    else
    {
        m_bMapHasBombTarget = false;
        m_bMapHasBombZone   = false;
    }

    m_bMapHasRescueZone    = UTIL_FindEntityByClassname( NULL, "func_hostage_rescue" ) != NULL;
    m_bMapHasBuyZone       = UTIL_FindEntityByClassname( NULL, "func_buyzone" ) != NULL;
    m_bMapHasEscapeZone    = UTIL_FindEntityByClassname( NULL,  "func_escapezone" ) != NULL;
    m_iMapHasVIPSafetyZone = UTIL_FindEntityByClassname( NULL, "func_vip_safetyzone" ) != NULL ? 1 : 2;
}

// CS
void CHalfLifeMultiplay::GiveC4( void )
{
    int numCurrTerrorist  = m_iNumTerrorist;
    int numFoundTerrorist = 0;

    ++m_iC4Guy;

    // TODO: Implement me.
    bool botDeferToHuman; // = cv_bot_defer_to_human.value > 0.0;

    if( botDeferToHuman )
    {
        for( int i = 1; i <= gpGlobals->maxClients; ++i )
        {
            CBasePlayer* pPlayer = ( CBasePlayer* )UTIL_PlayerByIndex( i );

            if( !FNullEnt( pPlayer->pev ) )
            {
                if( pPlayer->pev->deadflag == DEAD_NO && pPlayer->m_iTeam == TERRORIST && !pPlayer->IsBot() )
                {
                    ++numFoundTerrorist;
                }
            }
        }

        if( numFoundTerrorist )
            numCurrTerrorist = numFoundTerrorist;
        else
            botDeferToHuman = FALSE;
    }

    if( numCurrTerrorist < m_iC4Guy )
    {
        m_iC4Guy = 1;
    }

    CBaseEntity* pEntity = NULL;

    while( ( pEntity = UTIL_FindEntityByClassname( pEntity, "player" ) ) != NULL )
    {
        if( pEntity && pEntity->IsPlayer() && !pEntity->IsDormant() )
        {
            continue;
        }

        CBasePlayer* pPlayer = GetClassPtr( ( CBasePlayer* )pEntity->pev );

        if( pPlayer->pev->deadflag == DEAD_NO && pPlayer->m_iTeam == TERRORIST && ( !botDeferToHuman || !pPlayer->IsBot() ) )
        {
            if( ++numCurrTerrorist == m_iC4Guy )
            {
                pPlayer->m_bHasC4 = true;

                pPlayer->GiveNamedItem( "weapon_c4" );
                pPlayer->SetBombIcon( FALSE );

                pPlayer->pev->body = 1;

                pPlayer->m_flDisplayHistory |= Hint_you_have_the_bomb;
                pPlayer->HintMessage( "#Hint_you_have_the_bomb", FALSE, TRUE );

                UTIL_LogPrintf( "\"%s<%i><%s><TERRORIST>\" triggered \"Spawned_With_The_Bomb\"\n",
                    STRING( pPlayer->pev->netname ),
                    GETPLAYERAUTHID( pPlayer->edict() ),
                    GETPLAYERUSERID( pPlayer->edict() ) );

                m_bBombDropped = false;
            }
        }
    }

    if( !IsThereABomber() )
    {
        pEntity  = NULL;
        m_iC4Guy = 0;

        while( ( pEntity = UTIL_FindEntityByClassname( pEntity, "player" ) ) != NULL )
        {
            if( pEntity && pEntity->IsPlayer() && !pEntity->IsDormant() )
            {
                continue;
            }

            CBasePlayer* pPlayer = GetClassPtr( ( CBasePlayer* )pEntity->pev );

            if( pPlayer->pev->deadflag == DEAD_NO && pPlayer->m_iTeam == TERRORIST )
            {
                pPlayer->m_bHasC4 = true;

                pPlayer->GiveNamedItem( "weapon_c4" );
                pPlayer->SetBombIcon( FALSE );

                pPlayer->pev->body = 1;

                pPlayer->m_flDisplayHistory |= Hint_you_have_the_bomb;
                pPlayer->HintMessage( "#Hint_you_have_the_bomb", FALSE, TRUE );

                UTIL_LogPrintf( "\"%s<%i><%s><TERRORIST>\" triggered \"Spawned_With_The_Bomb\"\n",
                    STRING( pPlayer->pev->netname ),
                    GETPLAYERAUTHID( pPlayer->edict() ),
                    GETPLAYERUSERID( pPlayer->edict() ) );

                m_bBombDropped = false;
                return;
            }
        }
    }
}

// CS
bool CHalfLifeMultiplay::HasRoundTimeExpired( void )
{
    if( TimeRemaining() > 0 || m_iRoundWinStatus )
    {
        return false;
    }

    if( !IsBombPlanted() )
    {
        return false;
    }

    // TODO: Implement me.
    // if( cv_bot_nav_edit.value != 0.0 && !IS_DEDICATED_SERVER() && UTIL_HumansInGame( false ) == 1 )
    // {
    //     return false;
    // }

    return true;
}

// CS
bool CHalfLifeMultiplay::IsBombPlanted( void )
{
    if( m_bMapHasBombTarget )
    {
        CGrenade *pGrenade = NULL;

        if( ( pGrenade = ( CGrenade * )UTIL_FindEntityByClassname( pGrenade, "grenade" ) ) != NULL )
        {
            if( pGrenade->m_bIsC4 )
            {
                return true;
            }
        }
    }

    return false;
}

// CS
BOOL CHalfLifeMultiplay::IsThereABomber( void )
{
    CBasePlayer *pPlayer = NULL;

    while( ( pPlayer = ( CBasePlayer* )UTIL_FindEntityByClassname( pPlayer, "player" ) ) != NULL )
    {
        if( pPlayer->m_iTeam != CT && pPlayer->IsBombGuy() )
            return TRUE;
    }

    return FALSE;
}

// CS
BOOL CHalfLifeMultiplay::IsThereABomb( void )
{
    CGrenade *pWeaponC4 = NULL;
    bool bombFound = false;

    while( ( pWeaponC4 = ( CGrenade* )UTIL_FindEntityByClassname( pWeaponC4, "grenade" ) ) != NULL )
    {
        if( pWeaponC4->m_bIsC4 )
        {
            bombFound = true;
            break;
        }
    }

    if( !bombFound )
        return UTIL_FindEntityByClassname( NULL, "weapon_c4" ) != NULL;

    return bombFound;
}

// CS
bool CHalfLifeMultiplay::IsVIPQueueEmpty( void )
{
    for( int i = 0; i < MAX_VIP_QUEUES; i++ )
    {
        if( VIPQueue[ i ] && VIPQueue[ i ]->m_iTeam != CT )
        {
            VIPQueue[ i ] = NULL;
        }
    }

    StackVIPQueue();

    return !VIPQueue[ 0 ] && !VIPQueue[ 1 ] && !VIPQueue[ 2 ] && !VIPQueue[ 3 ] && !VIPQueue[ 4 ];
}

// CS
BOOL CHalfLifeMultiplay::FPlayerCanRespawn( CBasePlayer *pPlayer )
{
    if( pPlayer->m_iNumSpawns <= 0 )
    {
        m_iNumCT        = CountTeamPlayers( CT );
        m_iNumTerrorist = CountTeamPlayers( TERRORIST );

        if( m_iNumCT <= 0 || m_iNumTerrorist <= 0 || gpGlobals->time <= m_fRoundCount + 20.0 )
        {
            return pPlayer->m_iMenu != 3;
        }
        else if( fadetoblack.value != 0.0 )
        {
            UTIL_ScreenFade( pPlayer, 0, 3.0, 3.0, 255, 5 );
        }
    }

    return FALSE;
}

// CS
void CHalfLifeMultiplay::PickNextVIP()
{
    if( !IsVIPQueueEmpty() )
    {
        if( m_pVIP != NULL )
        {
            ResetCurrentVIP();
        }

        for( int i = 0; i < MAX_VIP_QUEUES; i++)
        {
            m_pVIP = VIPQueue[i];
            m_pVIP->MakeVIP();

            VIPQueue[i] = NULL;

            StackVIPQueue();
            m_iConsecutiveVIP = 0;
        }
    }
    else if( m_iConsecutiveVIP > 2 )
    {
        if( ++m_iLastPick > m_iNumCT )
        {
            m_iLastPick = 1;
        }

        int count = 1;

        CBasePlayer *pLastPlayer = NULL;
        CBaseEntity *pEntity     = NULL;

        while( ( pEntity = UTIL_FindEntityByClassname( pEntity, "player" ) ) != NULL )
        {
            if( FNullEnt(pEntity->edict() ) )
                break;

            if( pEntity->IsDormant() )
                continue;

            CBasePlayer *pPlayer = GetClassPtr( ( CBasePlayer* )pEntity->pev );

            if( pPlayer->m_iTeam == CT )
            {
                if( count == m_iLastPick )
                {
                    if( pPlayer == m_pVIP && pLastPlayer )
                    {
                        pPlayer = pLastPlayer;
                    }

                    if( m_pVIP != NULL )
                    {
                        ResetCurrentVIP();
                    }

                    pPlayer->MakeVIP();
                    m_iConsecutiveVIP = 0;

                    break;
                }

                count++;
            }

            if( pPlayer->m_iTeam != SPECTATOR )
            {
                pLastPlayer = pPlayer;
            }
        }
    }
    else if( !m_pVIP )
    {
        CBaseEntity *pEntity = NULL;

        while( ( pEntity = UTIL_FindEntityByClassname( pEntity, "player" ) ) != NULL )
        {
            if( FNullEnt( pEntity->edict() ) )
                break;

            if( pEntity->IsDormant() )
                continue;

            CBasePlayer *pPlayer = GetClassPtr( ( CBasePlayer* )pEntity->pev );

            if( pPlayer->m_iTeam = CT )
            {
                pPlayer->MakeVIP();
                m_iConsecutiveVIP = 0;
            }

            break;
        }
    }
}

// CS
void CHalfLifeMultiplay::RestartRound()
{
    // TODO: Implement me.
    // if( TheBots )
    //    TheBots->RestartRound();

    // TODO: Implement me.
    // if( g_pHostages )
    //    g_pHostages->RestartRound();

    ++m_iTotalRoundsPlayed;

    // TODO: Implement me.
    // ClearBodyQue();

    CVAR_SET_FLOAT( "sv_accelerate", 5.0 );
    CVAR_SET_FLOAT( "sv_friction", 4.0 );
    CVAR_SET_FLOAT( "sv_stopspeed", 75.0 );

    m_iNumCT        = CountTeamPlayers( CT );
    m_iNumTerrorist = CountTeamPlayers( TERRORIST );

    if( m_bMapHasBombTarget )
    {
        MESSAGE_BEGIN( MSG_ALL, gmsgBombPickup );
        MESSAGE_END();

        MESSAGE_BEGIN( MSG_ALL, gmsgShowTimer );
        MESSAGE_END();
    }

    g_pGameRules->m_bBombDropped = false;

    MESSAGE_BEGIN( MSG_SPEC, gmsgHLTV );
        WRITE_BYTE( 0 );
        WRITE_BYTE( 228 );
    MESSAGE_END();

    MESSAGE_BEGIN( MSG_SPEC, gmsgHLTV );
        WRITE_BYTE( 0 );
        WRITE_BYTE( 0 );
    MESSAGE_END();

    int autoBalance = ( int )CVAR_GET_FLOAT( "mp_autoteambalance" );

    if( autoBalance > 0 || m_iUnBalancedRounds > 0 )
    {
        BalanceTeams();
    }
    if( m_iNumCT - m_iNumTerrorist >= 2 || m_iNumTerrorist - m_iNumCT >= 2 )
        m_iUnBalancedRounds++;
    else
        m_iUnBalancedRounds = 0;

    if( autoBalance != 0 && m_iUnBalancedRounds == 1 )
    {
        UTIL_ClientPrintAll( print_center, "#Auto_Team_Balance_Next_Round" );
    }

    if( m_bCompleteReset )
    {
        if( timelimit.value < 0 )
        {
            CVAR_SET_FLOAT( "mp_timelimit", 0.0 );
        }

        g_flResetTime = gpGlobals->time;

        if( timelimit.value != 0 )
        {
            g_flTimeLimit = gpGlobals->time + timelimit.value * 60;
        }

        m_iTotalRoundsPlayed = 0;

        if( ( m_iMaxRounds = floor( CVAR_GET_FLOAT( "mp_maxrounds" ) ) ) < 0 )
        {
            m_iMaxRounds = 0;
            CVAR_SET_FLOAT( "mp_maxrounds", 0 );
        }

        if( ( m_iMaxRoundsWon = floor( CVAR_GET_FLOAT( "mp_winlimit" ) ) ) < 0 )
        {
            m_iMaxRoundsWon = 0;
            CVAR_SET_FLOAT( "mp_winlimit", 0 );
        }

        m_iNumTerroristWins             = 0;
        m_iNumCTWins                    = 0;
        m_iNumConsecutiveTerroristLoses = 0;
        m_iNumConsecutiveCTLoses        = 0;

        UpdateTeamScores();

        for( int i = 1; i <= gpGlobals->maxClients; i++ )
        {
            CBasePlayer *pPlayer = ( CBasePlayer* )UTIL_PlayerByIndex( i );

            if( pPlayer && !FNullEnt( pPlayer->pev ) )
            {
                pPlayer->Reset();
            }
        }

        // TODO: Implement me.
        // TheBots->OnEvents( EVENT_NEW_MATCH, NULL, NULL );
    }

    m_bFreezePeriod     = true;
    m_bRoundTerminating = false;

    // TODO: Implement me.
    // ReadMultiplayCvars();

    m_fMaxIdlePeriod = m_iRoundTime * 2;
    m_iRoundTimeSecs = m_iIntroRoundTime;

    // TODO: Implement me
    /*
    CBaseEntity * pEnt = UTIL_FindEntityByClassname( NULL, "info_map_parameters" );

    if( pEnt != NULL )
    {
        switch( pEnt->m_ibuying )
        {
            case 0: 
            {
                m_bCTCantBuy = false; 
                m_bTCantBuy  = false; 

                ALERT( at_console, "EVERYONE CAN BUY!\n" );
                break;
            }
            case 1: 
            {
                m_bCTCantBuy = false; 
                m_bTCantBuy  = true; 

                ALERT( at_console, "Only CT's can buy!!\n" );
                break;
            }
            case 2: 
            {
                m_bCTCantBuy = true; 
                m_bTCantBuy  = false; 

                ALERT( at_console, "Only T's can buy!!\n" );
                break;
            }
            case 3: 
            {
                m_bCTCantBuy = true; 
                m_bTCantBuy  = true;

                ALERT( at_console, "No one can buy!!\n" );
                break;
            }
            default: 
            {
                m_bCTCantBuy = false; 
                m_bTCantBuy  = false; 
                break;
            }
        }

        m_flBombRadius = pEnt->m_flBombRadius;
    }*/

    if( UTIL_FindEntityByClassname( NULL, "func_bomb_target" ) != NULL )
    {
        m_bMapHasBombTarget = true;
        m_bMapHasBombZone   = true;
    }
    else if( UTIL_FindEntityByClassname( NULL, "info_bomb_target" ) != NULL )
    {
        m_bMapHasBombTarget = true;
        m_bMapHasBombZone   = false;
    }
    else
    {
        m_bMapHasBombTarget = false;
        m_bMapHasBombZone   = false;
    }

    m_bMapHasRescueZone = UTIL_FindEntityByClassname( NULL, "func_hostage_rescue" ) != NULL;
    m_bMapHasBuyZone    = UTIL_FindEntityByClassname( NULL, "func_buyzone" ) != NULL;

    if( UTIL_FindEntityByClassname( NULL, "func_escapezone") != NULL )
    {
        m_bMapHasEscapeZone = true;
        m_iHaveEscaped      = 0;
        m_iNumEscapers      = 0;

        if( m_iNumEscapeRounds > 2 )
        {
            SwapAllPlayers();
            m_iNumEscapeRounds = 1;
        }
        else
            m_iNumEscapeRounds++;
    }
    else
    {
        m_bMapHasEscapeZone = false;
    }

    if( UTIL_FindEntityByClassname( NULL, "func_vip_safetyzone" ) != NULL )
    {
        PickNextVIP();

        m_iConsecutiveVIP++;
        m_iMapHasVIPSafetyZone = 1;
    }
    else
        m_iMapHasVIPSafetyZone = 2;

    int accountTemp = 0;

    // TODO: Implement me
    /* CHostage *pHostage;
    int accountTemp = 0;

    while( ( pHostage = ( CHostage* )UTIL_FindEntityByClassname( pHostage, "hostage_entity" ) ) != NULL )
    {
        if( pHostage->pev->solid )
        {
            accountTemp += 150;

            if( pHostage->pev->deadflag == DEAD_DEAD )
            {
                pHostage->pev->deeadflag = DEAD_RESPAWNABLE;
            }
        }

        pHostage->RePosition();

        if( accountTemp >= 2000 )
        {
            break;
        }
    }*/   

    switch( m_iRoundWinStatus )
    {
        case 1 :
        {
            if( m_iNumConsecutiveTerroristLoses > 1 )
            {
                m_iLoserBonus = 1500;
            }

            m_iNumConsecutiveTerroristLoses = 0;
            m_iNumConsecutiveCTLoses++;

            break;
        }
        case 2 :
        {
            if( m_iNumConsecutiveCTLoses > 1 )
            {
                m_iLoserBonus = 1500;
            }

            m_iNumConsecutiveCTLoses = 0;
            m_iNumConsecutiveTerroristLoses++;
            break;
        }
    }

    if( m_iLoserBonus < 3000 && ( m_iNumConsecutiveTerroristLoses > 1 || m_iNumConsecutiveCTLoses > 1 ) )
    {
        m_iLoserBonus += 500;
    }

    switch( m_iRoundWinStatus )
    {
        case 1 :
        {
            m_iAccountCT += accountTemp;

            if( !m_bMapHasEscapeZone )
            {
                m_iAccountTerrorist += m_iLoserBonus;
            }

            break;
        }
        case 2 :
        {
            m_iAccountTerrorist += accountTemp;
            m_iAccountCT += m_iLoserBonus;

            break;
        }
    }

    m_iAccountCT += m_iHostagesRescued * 750;
    m_fIntroRoundCount = m_fRoundCount = gpGlobals->time;

    if( m_bCompleteReset )
    {
        m_iAccountTerrorist             = 0;
        m_iAccountCT                    = 0;
        m_iNumTerroristWins             = 0; 
        m_iNumCTWins                    = 0;
        m_iNumConsecutiveTerroristLoses = 0;
        m_iNumConsecutiveCTLoses        = 0;
        m_iLoserBonus                   = 1400;
    }

    CBaseEntity *pEntity = NULL;

    while( ( pEntity = UTIL_FindEntityByClassname( pEntity, "player" ) ) != NULL )
    {
        CBasePlayer *pPlayer = GetClassPtr( ( CBasePlayer* )pEntity );

        pPlayer->m_iNumSpawns   = 0;
        pPlayer->m_bTeamChanged = false;

        if( !pPlayer->IsPlayer() )
        {
            pPlayer->SyncRoundTimer();
        }

        switch( pPlayer->m_iTeam )
        {
            case CT :
            {
                if( !pPlayer->m_bReceivesNoMoneyNextRound )
                {
                    pPlayer->AddAccount( m_iAccountCT, true );
                }
            }
            case TERRORIST :
            {
                m_iNumEscapers++;

                if( !pPlayer->m_bReceivesNoMoneyNextRound )
                {
                    pPlayer->AddAccount( m_iAccountTerrorist, true );
                }

                if( m_bMapHasEscapeZone )
                {
                    pPlayer->m_bNotKilled = false;
                }
            }
        }

        if( pPlayer->m_iTeam != UNASSIGNED && pPlayer->m_iTeam != SPECTATOR )
        {
            if( pPlayer->m_bHasC4 )
            {
                pPlayer->DropPlayerItem( "weapon_c4" );
            }

            pPlayer->RoundRespawn();
        }        
    }

    CleanUpMap();

    if( m_bMapHasBombTarget )
    {
        GiveC4();
    }

    // TODO: Implement me
    // TheBots->OnEvents( EVENT_BUY_TIME_START, NULL, NULL);

    m_flIntermissionEndTime   = 0;
    m_flIntermissionStartTime = 0;
    m_fTeamCount              = 0;
    m_iAccountTerrorist       = 0;
    m_iAccountCT              = 0;
    m_iHostagesRescued        = 0;
    m_iHostagesTouched        = 0;
    m_iRoundWinStatus         = 0;
    m_bTargetBombed           = false;
    m_bBombDefused            = false;
    m_bLevelInitialized       = false;
    m_bCompleteReset          = false;
}

// CS
void CHalfLifeMultiplay::ServerDeactivate()
{
    if( IS_CAREER_MATCH() )
    {
        CVAR_SET_FLOAT( "pausable", 0 );
        CVAR_SET_FLOAT( "mp_windifference", 1.0f );
        
        UTIL_LogPrintf( "Career End\n" );
    }
}

// CS
void CHalfLifeMultiplay::SwapAllPlayers()
{
    CBaseEntity *pEntity = NULL;

    while( ( pEntity = UTIL_FindEntityByClassname( pEntity, "player" ) ) != NULL )
    {
        if( FNullEnt( pEntity->edict() ) )
            break;

        if( pEntity->IsDormant() )
        {
            GetClassPtr( ( CBasePlayer* )pEntity->pev )->SwitchTeam();
        }
    }

    m_iNumTerroristWins ^= m_iNumCTWins;
    m_iNumCTWins        ^= m_iNumTerroristWins;
    m_iNumTerroristWins ^= m_iNumCTWins;

    UpdateTeamScores();
}

// CS
void CHalfLifeMultiplay::UpdateTeamScores()
{
    MESSAGE_BEGIN( MSG_ALL, gmsgTeamScore );
        WRITE_STRING( "CT" );
        WRITE_SHORT( m_iNumCTWins );
    MESSAGE_END();

    MESSAGE_BEGIN( MSG_ALL, gmsgTeamScore );
        WRITE_STRING( "TERRORIST" );
        WRITE_SHORT( m_iNumTerroristWins );
    MESSAGE_END();
}

// CS
void CHalfLifeMultiplay::ClientUserInfoChanged( CBasePlayer *pPlayer, char *infobuffer )
{
    pPlayer->SetPlayerModel( pPlayer->m_bHasC4 );
    pPlayer->SetPrefsFromUserinfo( infobuffer );
}
