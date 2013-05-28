#include "voice_gamemgr.h"
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
//=========================================================
// GameRules
//=========================================================

//#include "weapons.h"
//#include "items.h"
class CBasePlayerItem;
class CBasePlayer;
class CItem;
class CBasePlayerAmmo;

// weapon respawning return codes
enum
{	
	GR_NONE = 0,
	
	GR_WEAPON_RESPAWN_YES,
	GR_WEAPON_RESPAWN_NO,
	
	GR_AMMO_RESPAWN_YES,
	GR_AMMO_RESPAWN_NO,
	
	GR_ITEM_RESPAWN_YES,
	GR_ITEM_RESPAWN_NO,

	GR_PLR_DROP_GUN_ALL,
	GR_PLR_DROP_GUN_ACTIVE,
	GR_PLR_DROP_GUN_NO,

	GR_PLR_DROP_AMMO_ALL,
	GR_PLR_DROP_AMMO_ACTIVE,
	GR_PLR_DROP_AMMO_NO,
};

// Player relationship return codes
enum
{
	GR_NOTTEAMMATE = 0,
	GR_TEAMMATE,
	GR_ENEMY,
	GR_ALLY,
	GR_NEUTRAL,
};

class CGameRules
{
public:
	virtual void RefreshSkillData( void );// fill skill data struct with proper values
	virtual void Think( void ) = 0;// GR_Think - runs every server frame, should handle any timer tasks, periodic events, etc.
	virtual BOOL IsAllowedToSpawn( CBaseEntity *pEntity ) = 0;  // Can this item spawn (eg monsters don't spawn in deathmatch).

	virtual BOOL FAllowFlashlight( void ) = 0;// Are players allowed to switch on their flashlight?
	virtual BOOL FShouldSwitchWeapon( CBasePlayer *pPlayer, CBasePlayerItem *pWeapon ) = 0;// should the player switch to this weapon?
	virtual BOOL GetNextBestWeapon( CBasePlayer *pPlayer, CBasePlayerItem *pCurrentWeapon ) = 0;// I can't use this weapon anymore, get me the next best one.

// Functions to verify the single/multiplayer status of a game
	virtual BOOL IsMultiplayer( void ) = 0;// is this a multiplayer game? (either coop or deathmatch)
	virtual BOOL IsDeathmatch( void ) = 0;//is this a deathmatch game?
	virtual BOOL IsTeamplay( void ) { return FALSE; };// is this deathmatch game being played with team rules?
	virtual BOOL IsCoOp( void ) = 0;// is this a coop game?
	virtual const char *GetGameDescription( void ) { return "Half-Life"; }  // this is the game name that gets seen in the server browser
	
// Client connection/disconnection
	virtual BOOL ClientConnected( edict_t *pEntity, const char *pszName, const char *pszAddress, char szRejectReason[ 128 ] ) = 0;// a client just connected to the server (player hasn't spawned yet)
	virtual void InitHUD( CBasePlayer *pl ) = 0;		// the client dll is ready for updating
	virtual void ClientDisconnected( edict_t *pClient ) = 0;// a client just disconnected from the server
	virtual void UpdateGameMode( CBasePlayer *pPlayer ) {}  // the client needs to be informed of the current game mode

// Client damage rules
	virtual float FlPlayerFallDamage( CBasePlayer *pPlayer ) = 0;// this client just hit the ground after a fall. How much damage?
	virtual BOOL  FPlayerCanTakeDamage( CBasePlayer *pPlayer, CBaseEntity *pAttacker ) {return TRUE;};// can this player take damage from this attacker?
	virtual BOOL ShouldAutoAim( CBasePlayer *pPlayer, edict_t *target ) { return TRUE; }

// Client spawn/respawn control
	virtual void PlayerSpawn( CBasePlayer *pPlayer ) = 0;// called by CBasePlayer::Spawn just before releasing player into the game
	virtual void PlayerThink( CBasePlayer *pPlayer ) = 0; // called by CBasePlayer::PreThink every frame, before physics are run and after keys are accepted
	virtual BOOL FPlayerCanRespawn( CBasePlayer *pPlayer ) = 0;// is this player allowed to respawn now?
	virtual float FlPlayerSpawnTime( CBasePlayer *pPlayer ) = 0;// When in the future will this player be able to spawn?
	virtual edict_t *GetPlayerSpawnSpot( CBasePlayer *pPlayer );// Place this player on their spawnspot and face them the proper direction.

	virtual BOOL AllowAutoTargetCrosshair( void ) { return TRUE; };
	virtual BOOL ClientCommand( CBasePlayer *pPlayer, const char *pcmd ) { return FALSE; };  // handles the user commands;  returns TRUE if command handled properly
	virtual void ClientUserInfoChanged( CBasePlayer *pPlayer, char *infobuffer ) {}		// the player has changed userinfo;  can change it now

// Client kills/scoring
	virtual int IPointsForKill( CBasePlayer *pAttacker, CBasePlayer *pKilled ) = 0;// how many points do I award whoever kills this player?
	virtual void PlayerKilled( CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor ) = 0;// Called each time a player dies
	virtual void DeathNotice( CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor )=  0;// Call this from within a GameRules class to report an obituary.
// Weapon retrieval
	virtual BOOL CanHavePlayerItem( CBasePlayer *pPlayer, CBasePlayerItem *pWeapon );// The player is touching an CBasePlayerItem, do I give it to him?
	virtual void PlayerGotWeapon( CBasePlayer *pPlayer, CBasePlayerItem *pWeapon ) = 0;// Called each time a player picks up a weapon from the ground

// Weapon spawn/respawn control
	virtual int WeaponShouldRespawn( CBasePlayerItem *pWeapon ) = 0;// should this weapon respawn?
	virtual float FlWeaponRespawnTime( CBasePlayerItem *pWeapon ) = 0;// when may this weapon respawn?
	virtual float FlWeaponTryRespawn( CBasePlayerItem *pWeapon ) = 0; // can i respawn now,  and if not, when should i try again?
	virtual Vector VecWeaponRespawnSpot( CBasePlayerItem *pWeapon ) = 0;// where in the world should this weapon respawn?

// Item retrieval
	virtual BOOL CanHaveItem( CBasePlayer *pPlayer, CItem *pItem ) = 0;// is this player allowed to take this item?
	virtual void PlayerGotItem( CBasePlayer *pPlayer, CItem *pItem ) = 0;// call each time a player picks up an item (battery, healthkit, longjump)

// Item spawn/respawn control
	virtual int ItemShouldRespawn( CItem *pItem ) = 0;// Should this item respawn?
	virtual float FlItemRespawnTime( CItem *pItem ) = 0;// when may this item respawn?
	virtual Vector VecItemRespawnSpot( CItem *pItem ) = 0;// where in the world should this item respawn?

// Ammo retrieval
	virtual BOOL CanHaveAmmo( CBasePlayer *pPlayer, const char *pszAmmoName, int iMaxCarry );// can this player take more of this ammo?
	virtual void PlayerGotAmmo( CBasePlayer *pPlayer, char *szName, int iCount ) = 0;// called each time a player picks up some ammo in the world

// Ammo spawn/respawn control
	virtual int AmmoShouldRespawn( CBasePlayerAmmo *pAmmo ) = 0;// should this ammo item respawn?
	virtual float FlAmmoRespawnTime( CBasePlayerAmmo *pAmmo ) = 0;// when should this ammo item respawn?
	virtual Vector VecAmmoRespawnSpot( CBasePlayerAmmo *pAmmo ) = 0;// where in the world should this ammo item respawn?
																			// by default, everything spawns

// Healthcharger respawn control
	virtual float FlHealthChargerRechargeTime( void ) = 0;// how long until a depleted HealthCharger recharges itself?
	virtual float FlHEVChargerRechargeTime( void ) { return 0; }// how long until a depleted HealthCharger recharges itself?

// What happens to a dead player's weapons
	virtual int DeadPlayerWeapons( CBasePlayer *pPlayer ) = 0;// what do I do with a player's weapons when he's killed?

// What happens to a dead player's ammo	
	virtual int DeadPlayerAmmo( CBasePlayer *pPlayer ) = 0;// Do I drop ammo when the player dies? How much?

// Teamplay stuff
	virtual const char *GetTeamID( CBaseEntity *pEntity ) = 0;// what team is this entity on?
	virtual int PlayerRelationship( CBaseEntity *pPlayer, CBaseEntity *pTarget ) = 0;// What is the player's relationship with this entity?
	virtual int GetTeamIndex( const char *pTeamName ) { return -1; }
	virtual const char *GetIndexedTeamName( int teamIndex ) { return ""; }
	virtual BOOL IsValidTeam( const char *pTeamName ) { return TRUE; }
	virtual void ChangePlayerTeam( CBasePlayer *pPlayer, const char *pTeamName, BOOL bKill, BOOL bGib ) {}
	virtual const char *SetDefaultPlayerTeam( CBasePlayer *pPlayer ) { return ""; }

// Sounds
	virtual BOOL PlayTextureSounds( void ) { return TRUE; }
	virtual BOOL PlayFootstepSounds( CBasePlayer *pl, float fvol ) { return TRUE; }

// Monsters
	virtual BOOL FAllowMonsters( void ) = 0;//are monsters allowed

	// Immediately end a multiplayer game
	virtual void EndMultiplayerGame( void ) {}

	BOOL                       m_bFreezePeriod;      /*     4     4 */
	BOOL                       m_bBombDropped;       /*     8     4 */
};

extern CGameRules *InstallGameRules( void );


//=========================================================
// CHalfLifeRules - rules for the single player Half-Life 
// game.
//=========================================================
class CHalfLifeRules : public CGameRules
{
public:
	CHalfLifeRules ( void );

// GR_Think
	virtual void Think( void );
	virtual BOOL IsAllowedToSpawn( CBaseEntity *pEntity );
	virtual BOOL FAllowFlashlight( void ) { return TRUE; };

	virtual BOOL FShouldSwitchWeapon( CBasePlayer *pPlayer, CBasePlayerItem *pWeapon );
	virtual BOOL GetNextBestWeapon( CBasePlayer *pPlayer, CBasePlayerItem *pCurrentWeapon );
	
// Functions to verify the single/multiplayer status of a game
	virtual BOOL IsMultiplayer( void );
	virtual BOOL IsDeathmatch( void );
	virtual BOOL IsCoOp( void );

// Client connection/disconnection
	virtual BOOL ClientConnected( edict_t *pEntity, const char *pszName, const char *pszAddress, char szRejectReason[ 128 ] );
	virtual void InitHUD( CBasePlayer *pl );		// the client dll is ready for updating
	virtual void ClientDisconnected( edict_t *pClient );

// Client damage rules
	virtual float FlPlayerFallDamage( CBasePlayer *pPlayer );
	
// Client spawn/respawn control
	virtual void PlayerSpawn( CBasePlayer *pPlayer );
	virtual void PlayerThink( CBasePlayer *pPlayer );
	virtual BOOL FPlayerCanRespawn( CBasePlayer *pPlayer );
	virtual float FlPlayerSpawnTime( CBasePlayer *pPlayer );

	virtual BOOL AllowAutoTargetCrosshair( void );

// Client kills/scoring
	virtual int IPointsForKill( CBasePlayer *pAttacker, CBasePlayer *pKilled );
	virtual void PlayerKilled( CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor );
	virtual void DeathNotice( CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor );

// Weapon retrieval
	virtual void PlayerGotWeapon( CBasePlayer *pPlayer, CBasePlayerItem *pWeapon );

// Weapon spawn/respawn control
	virtual int WeaponShouldRespawn( CBasePlayerItem *pWeapon );
	virtual float FlWeaponRespawnTime( CBasePlayerItem *pWeapon );
	virtual float FlWeaponTryRespawn( CBasePlayerItem *pWeapon );
	virtual Vector VecWeaponRespawnSpot( CBasePlayerItem *pWeapon );

// Item retrieval
	virtual BOOL CanHaveItem( CBasePlayer *pPlayer, CItem *pItem );
	virtual void PlayerGotItem( CBasePlayer *pPlayer, CItem *pItem );

// Item spawn/respawn control
	virtual int ItemShouldRespawn( CItem *pItem );
	virtual float FlItemRespawnTime( CItem *pItem );
	virtual Vector VecItemRespawnSpot( CItem *pItem );

// Ammo retrieval
	virtual void PlayerGotAmmo( CBasePlayer *pPlayer, char *szName, int iCount );

// Ammo spawn/respawn control
	virtual int AmmoShouldRespawn( CBasePlayerAmmo *pAmmo );
	virtual float FlAmmoRespawnTime( CBasePlayerAmmo *pAmmo );
	virtual Vector VecAmmoRespawnSpot( CBasePlayerAmmo *pAmmo );

// Healthcharger respawn control
	virtual float FlHealthChargerRechargeTime( void );

// What happens to a dead player's weapons
	virtual int DeadPlayerWeapons( CBasePlayer *pPlayer );

// What happens to a dead player's ammo	
	virtual int DeadPlayerAmmo( CBasePlayer *pPlayer );

// Monsters
	virtual BOOL FAllowMonsters( void );

// Teamplay stuff	
	virtual const char *GetTeamID( CBaseEntity *pEntity ) {return "";};
	virtual int PlayerRelationship( CBaseEntity *pPlayer, CBaseEntity *pTarget );
};

//=========================================================
// CHalfLifeMultiplay - rules for the basic half life multiplayer
// competition
//=========================================================
class CHalfLifeMultiplay : public CGameRules
{
public:
	CHalfLifeMultiplay();

// GR_Think
	virtual void Think( void );
	virtual void RefreshSkillData( void );
	virtual BOOL IsAllowedToSpawn( CBaseEntity *pEntity );
	virtual BOOL FAllowFlashlight( void );

	virtual BOOL FShouldSwitchWeapon( CBasePlayer *pPlayer, CBasePlayerItem *pWeapon );
	virtual BOOL GetNextBestWeapon( CBasePlayer *pPlayer, CBasePlayerItem *pCurrentWeapon );

// Functions to verify the single/multiplayer status of a game
	virtual BOOL IsMultiplayer( void );
	virtual BOOL IsDeathmatch( void );
	virtual BOOL IsCoOp( void );

// Client connection/disconnection
	// If ClientConnected returns FALSE, the connection is rejected and the user is provided the reason specified in
	//  svRejectReason
	// Only the client's name and remote address are provided to the dll for verification.
	virtual BOOL ClientConnected( edict_t *pEntity, const char *pszName, const char *pszAddress, char szRejectReason[ 128 ] );
	virtual void InitHUD( CBasePlayer *pl );		// the client dll is ready for updating
	virtual void ClientDisconnected( edict_t *pClient );
	virtual void UpdateGameMode( CBasePlayer *pPlayer );  // the client needs to be informed of the current game mode

// Client damage rules
	virtual float FlPlayerFallDamage( CBasePlayer *pPlayer );
	virtual BOOL  FPlayerCanTakeDamage( CBasePlayer *pPlayer, CBaseEntity *pAttacker );

// Client spawn/respawn control
	virtual void PlayerSpawn( CBasePlayer *pPlayer );
	virtual void PlayerThink( CBasePlayer *pPlayer );
	virtual BOOL FPlayerCanRespawn( CBasePlayer *pPlayer );
	virtual float FlPlayerSpawnTime( CBasePlayer *pPlayer );
	virtual edict_t *GetPlayerSpawnSpot( CBasePlayer *pPlayer );

	virtual BOOL AllowAutoTargetCrosshair( void );
	virtual BOOL ClientCommand( CBasePlayer *pPlayer, const char *pcmd );

// Client kills/scoring
	virtual int IPointsForKill( CBasePlayer *pAttacker, CBasePlayer *pKilled );
	virtual void PlayerKilled( CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor );
	virtual void DeathNotice( CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor );

// Weapon retrieval
	virtual void PlayerGotWeapon( CBasePlayer *pPlayer, CBasePlayerItem *pWeapon );
	virtual BOOL CanHavePlayerItem( CBasePlayer *pPlayer, CBasePlayerItem *pWeapon );// The player is touching an CBasePlayerItem, do I give it to him?

// Weapon spawn/respawn control
	virtual int WeaponShouldRespawn( CBasePlayerItem *pWeapon );
	virtual float FlWeaponRespawnTime( CBasePlayerItem *pWeapon );
	virtual float FlWeaponTryRespawn( CBasePlayerItem *pWeapon );
	virtual Vector VecWeaponRespawnSpot( CBasePlayerItem *pWeapon );

// Item retrieval
	virtual BOOL CanHaveItem( CBasePlayer *pPlayer, CItem *pItem );
	virtual void PlayerGotItem( CBasePlayer *pPlayer, CItem *pItem );

// Item spawn/respawn control
	virtual int ItemShouldRespawn( CItem *pItem );
	virtual float FlItemRespawnTime( CItem *pItem );
	virtual Vector VecItemRespawnSpot( CItem *pItem );

// Ammo retrieval
	virtual void PlayerGotAmmo( CBasePlayer *pPlayer, char *szName, int iCount );

// Ammo spawn/respawn control
	virtual int AmmoShouldRespawn( CBasePlayerAmmo *pAmmo );
	virtual float FlAmmoRespawnTime( CBasePlayerAmmo *pAmmo );
	virtual Vector VecAmmoRespawnSpot( CBasePlayerAmmo *pAmmo );

// Healthcharger respawn control
	virtual float FlHealthChargerRechargeTime( void );
	virtual float FlHEVChargerRechargeTime( void );

// What happens to a dead player's weapons
	virtual int DeadPlayerWeapons( CBasePlayer *pPlayer );

// What happens to a dead player's ammo	
	virtual int DeadPlayerAmmo( CBasePlayer *pPlayer );

// Teamplay stuff	
	virtual const char *GetTeamID( CBaseEntity *pEntity ) {return "";}
	virtual int PlayerRelationship( CBaseEntity *pPlayer, CBaseEntity *pTarget );

	virtual BOOL PlayTextureSounds( void ) { return FALSE; }
	virtual BOOL PlayFootstepSounds( CBasePlayer *pl, float fvol );

// Monsters
	virtual BOOL FAllowMonsters( void );

	// Immediately end a multiplayer game
	virtual void EndMultiplayerGame( void ) { GoToIntermission(); }

	CVoiceGameMgr				m_VoiceGameMgr;						/*    12     0 */

	float						m_fTeamCount;						/*    40     4 */
	float						m_flCheckWinConditions;				/*    44     4 */
	float						m_fRoundCount;						/*    48     4 */
	int							m_iRoundTime;						/*    52     4 */
	int							m_iRoundTimeSecs;					/*    56     4 */
	int							m_iIntroRoundTime;					/*    60     4 */
	float						m_fIntroRoundCount;					/*    64     4 */

	int							m_iAccountTerrorist;				/*    68     4 */
	int							m_iAccountCT;						/*    72     4 */

	int							m_iNumTerrorist;					/*    76     4 */
	int							m_iNumCT;							/*    80     4 */
	int							m_iNumSpawnableTerrorist;			/*    84     4 */
	int							m_iNumSpawnableCT;					/*    88     4 */
	int							m_iSpawnPointCount_Terrorist;		/*    92     4 */
	int							m_iSpawnPointCount_CT;				/*    96     4 */

	int							m_iHostagesRescued;					/*   100     4 */
	int							m_iHostagesTouched;					/*   104     4 */

	int							m_iRoundWinStatus;					/*   108     4 */
	short int					m_iNumCTWins;						/*   112     2 */
	short int					m_iNumTerroristWins;				/*   114     2 */

	bool						m_bTargetBombed;					/*   116     1 */
	bool						m_bBombDefused;						/*   117     1 */

	bool						m_bMapHasBombTarget;				/*   118     1 */
	bool						m_bMapHasBombZone;					/*   119     1 */
	bool						m_bMapHasBuyZone;					/*   120     1 */
	bool						m_bMapHasRescueZone;				/*   121     1 */
	bool						m_bMapHasEscapeZone;				/*   122     1 */
	int							m_iMapHasVIPSafetyZone;				/*   124     4 */
	int							m_bMapHasCameras;					/*   128     4 */

	int							m_iC4Timer;							/*   132     4 */
	int							m_iC4Guy;							/*   136     4 */

	int							m_iLoserBonus;						/*   140     4 */
	int							m_iNumConsecutiveCTLoses;			/*   144     4 */
	int							m_iNumConsecutiveTerroristLoses;	/*   148     4 */

	float						m_fMaxIdlePeriod;					/*   152     4 */
	int							m_iLimitTeams;						/*   156     4 */
	bool						m_bLevelInitialized;				/*   160     1 */
	bool						m_bRoundTerminating;				/*   161     1 */
	bool						m_bCompleteReset;					/*   162     1 */

	float						m_flRequiredEscapeRatio;			/*   164     4 */
	int							m_iNumEscapers;						/*   168     4 */
	int							m_iHaveEscaped;						/*   172     4 */

	bool						m_bCTCantBuy;						/*   176     1 */
	bool						m_bTCantBuy;						/*   177     1 */

	float						m_flBombRadius;						/*   180     4 */

	int							m_iConsecutiveVIP;					/*   184     4 */

	int							m_iTotalGunCount;					/*   188     4 */
	int							m_iTotalGrenadeCount;				/*   192     4 */
	int							m_iTotalArmourCount;				/*   196     4 */

	int							m_iUnBalancedRounds;				/*   200     4 */
	int							m_iNumEscapeRounds;					/*   204     4 */

	int							m_iMapVotes[100];					/*   208   400 */
	int							m_iLastPick;						/*   608     4 */
	int							m_iMaxMapTime;						/*   612     4 */
	int							m_iMaxRounds;						/*   616     4 */
	int							m_iTotalRoundsPlayed;				/*   620     4 */
	int							m_iMaxRoundsWon;					/*   624     4 */

	int							m_iStoredSpectValue;				/*   628     4 */
	float						m_flForceCameraValue;				/*   632     4 */
	float						m_flForceChaseCamValue;				/*   636     4 */
	float						m_flFadeToBlackValue;				/*   640     4 */

	class CBasePlayer *			m_pVIP;								/*   644     4 */
	class CBasePlayer *			VIPQueue[5];						/*   648    20 */

protected:

	virtual void ChangeLevel( void );
	virtual void GoToIntermission( void );

	float                      m_flIntermissionEndTime;				/*   668     4 */
	float                      m_flIntermissionStartTime;			/*   672     4 */
	BOOL                       m_iEndIntermissionButtonHit;			/*   676     4 */

	float                      m_tmNextPeriodicThink;				/*   680     4 */
	bool                       m_bFirstConnected;					/*   684     1 */
	bool                       m_bInCareerGame;						/*   685     1 */
	float                      m_fCareerRoundMenuTime;				/*   688     4 */
	int                        m_iCareerMatchWins;					/*   692     4 */
	int                        m_iRoundWinDifference;				/*   696     4 */
	float                      m_fCareerMatchMenuTime;				/*   700     4 */
	bool                       m_bSkipSpawn;						/*   704     1 */

	void SendMOTDToClient( edict_t *client );
};

extern DLL_GLOBAL CGameRules*	g_pGameRules;
