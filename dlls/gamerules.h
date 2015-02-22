/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
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
#include "voice_gamemgr.h"

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
	virtual void RefreshSkillData(void);
	virtual void Think(void) = 0;
	virtual BOOL IsAllowedToSpawn(CBaseEntity* pEntity) = 0;

	virtual BOOL FAllowFlashlight(void) = 0;
	virtual BOOL FShouldSwitchWeapon(CBasePlayer *pPlayer, CBasePlayerItem *pWeapon) = 0;
	virtual BOOL GetNextBestWeapon(CBasePlayer *pPlayer, CBasePlayerItem *pCurrentWeapon) = 0;

	// Functions to verify the single/multiplayer status of a game
	virtual BOOL IsMultiplayer(void) = 0;
	virtual BOOL IsDeathmatch(void) = 0;
	virtual BOOL IsTeamplay(void) { return FALSE; };
	virtual BOOL IsCoOp(void) = 0;
	virtual const char *GetGameDescription(void) { return "Counter-Strike"; }

	// Client connection/disconnection
	virtual BOOL ClientConnected(edict_t *pEntity, const char *pszName, const char *pszAddress, char szRejectReason[128]) = 0;
	virtual void InitHUD(CBasePlayer *pl) = 0;
	virtual void ClientDisconnected(edict_t *pClient) = 0;
	virtual void UpdateGameMode(CBasePlayer *pPlayer) {}

	// Client damage rules
	virtual float FlPlayerFallDamage(CBasePlayer *pPlayer) = 0;
	virtual BOOL FPlayerCanTakeDamage(CBasePlayer *pPlayer, CBaseEntity *pAttacker) { return TRUE; }
	virtual BOOL ShouldAutoAim(CBasePlayer *pPlayer, edict_t *target) { return TRUE; }

	// Client spawn/respawn control
	virtual void PlayerSpawn(CBasePlayer *pPlayer) = 0;
	virtual void PlayerThink(CBasePlayer *pPlayer) = 0;
	virtual BOOL FPlayerCanRespawn(CBasePlayer *pPlayer) = 0;
	virtual float FlPlayerSpawnTime(CBasePlayer *pPlayer) = 0;
	virtual edict_t * GetPlayerSpawnSpot(CBasePlayer *pPlayer);

	virtual BOOL AllowAutoTargetCrosshair(void)  { return TRUE; }
	virtual BOOL ClientCommand_DeadOrAlive(CBasePlayer *pPlayer, const char *pcmd) { return FALSE; };
	virtual BOOL ClientCommand(CBasePlayer *pPlayer, const char *pcmd) { return FALSE; };
	virtual void ClientUserInfoChanged(CBasePlayer *pPlayer, char *infobuffer) {}

	// Client kills/scoring
	virtual int IPointsForKill(CBasePlayer *pAttacker, CBasePlayer *pKilled) = 0;
	virtual void PlayerKilled(CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor) = 0;
	virtual void DeathNotice(CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor) =  0;

	// Weapon retrieval
	virtual BOOL CanHavePlayerItem(CBasePlayer *pPlayer, CBasePlayerItem *pWeapon);
	virtual void PlayerGotWeapon(CBasePlayer *pPlayer, CBasePlayerItem *pWeapon) = 0;

	// Weapon spawn/respawn control
	virtual int WeaponShouldRespawn(CBasePlayerItem *pWeapon) = 0;
	virtual float FlWeaponRespawnTime(CBasePlayerItem *pWeapon) = 0;
	virtual float FlWeaponTryRespawn(CBasePlayerItem *pWeapon) = 0;
	virtual class Vector VecWeaponRespawnSpot(CBasePlayerItem *pWeapon) = 0;

	// Item retrieval
	virtual BOOL CanHaveItem(CBasePlayer *pPlayer, CItem *pItem) = 0;
	virtual void PlayerGotItem(CBasePlayer *pPlayer, CItem *pItem) = 0;

	// Item spawn/respawn control
	virtual int ItemShouldRespawn(CItem *pItem) = 0;
	virtual float FlItemRespawnTime(CItem *pItem) = 0;
	virtual class Vector VecItemRespawnSpot(CItem *pItem) = 0;

	// Ammo retrieval
	virtual BOOL CanHaveAmmo(CBasePlayer *pPlayer, const char *pszAmmoName, int iMaxCarry);
	virtual void PlayerGotAmmo(CBasePlayer *pPlayer, char *szName, int iCount) = 0;

	// Ammo spawn/respawn control
	virtual int AmmoShouldRespawn(CBasePlayerAmmo *pAmmo) = 0;
	virtual float FlAmmoRespawnTime(CBasePlayerAmmo *pAmmo) = 0;
	virtual class Vector VecAmmoRespawnSpot(CBasePlayerAmmo *pAmmo) = 0;

	// Healthcharger respawn control
	virtual float FlHealthChargerRechargeTime(void) = 0;
	virtual float FlHEVChargerRechargeTime(void) { return 0; }

	// What happens to a dead player's weapons
	virtual int DeadPlayerWeapons(CBasePlayer *pPlayer) = 0;

	// What happens to a dead player's ammo
	virtual int DeadPlayerAmmo(CBasePlayer *pPlayer) = 0;

	// Teamplay stuff
	virtual const char *GetTeamID(CBaseEntity *pEntity) = 0;
	virtual int PlayerRelationship(CBaseEntity *pPlayer, CBaseEntity *pTarget) = 0;
	virtual int GetTeamIndex(const char *pTeamName) { return -1; }
	virtual const char *GetIndexedTeamName(int teamIndex) { return ""; }
	virtual BOOL IsValidTeam(const char *pTeamName) { return TRUE; }
	virtual void ChangePlayerTeam(CBasePlayer *pPlayer, const char *pTeamName, BOOL bKill, BOOL bGib) {}
	virtual const char *SetDefaultPlayerTeam(CBasePlayer *pPlayer) { return ""; }

	// Sounds
	virtual BOOL PlayTextureSounds(void) { return TRUE; }

	// Monsters
	virtual BOOL FAllowMonsters(void) = 0;

	// Immediately end a multiplayer game
	virtual void EndMultiplayerGame(void) {}
	virtual BOOL IsFreezePeriod(void) { return FALSE; }
	virtual void ServerDeactivate(void) {}

	virtual void CheckMapConditions(void) {};

	BOOL                       m_bFreezePeriod;      /*     4     4 */
	BOOL                       m_bBombDropped;       /*     8     4 */
};

extern CGameRules *InstallGameRules(void);

//=========================================================
// CHalfLifeRules - rules for the single player Half-Life
// game.
//=========================================================
class CHalfLifeRules : public CGameRules
{
public:
	CHalfLifeRules(void);

	// GR_Think
	virtual void Think(void);
	virtual BOOL IsAllowedToSpawn(CBaseEntity *pEntity);
	virtual BOOL FAllowFlashlight(void) { return TRUE; };

	virtual BOOL FShouldSwitchWeapon(CBasePlayer *pPlayer, CBasePlayerItem *pWeapon);
	virtual BOOL GetNextBestWeapon(CBasePlayer *pPlayer, CBasePlayerItem *pCurrentWeapon);

	// Functions to verify the single/multiplayer status of a game
	virtual BOOL IsMultiplayer(void);
	virtual BOOL IsDeathmatch(void);
	virtual BOOL IsCoOp(void);

	// Client connection/disconnection
	virtual BOOL ClientConnected(edict_t *pEntity, const char *pszName, const char *pszAddress, char szRejectReason[128]);
	virtual void InitHUD(CBasePlayer *pl);		// the client dll is ready for updating
	virtual void ClientDisconnected(edict_t *pClient);

	// Client damage rules
	virtual float FlPlayerFallDamage(CBasePlayer *pPlayer);

	// Client spawn/respawn control
	virtual void PlayerSpawn(CBasePlayer *pPlayer);
	virtual void PlayerThink(CBasePlayer *pPlayer);
	virtual BOOL FPlayerCanRespawn(CBasePlayer *pPlayer);
	virtual float FlPlayerSpawnTime(CBasePlayer *pPlayer);

	virtual BOOL AllowAutoTargetCrosshair(void);

	// Client kills/scoring
	virtual int IPointsForKill(CBasePlayer *pAttacker, CBasePlayer *pKilled);
	virtual void PlayerKilled(CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor);
	virtual void DeathNotice(CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor);

	// Weapon retrieval
	virtual void PlayerGotWeapon(CBasePlayer *pPlayer, CBasePlayerItem *pWeapon);

	// Weapon spawn/respawn control
	virtual int WeaponShouldRespawn(CBasePlayerItem *pWeapon);
	virtual float FlWeaponRespawnTime(CBasePlayerItem *pWeapon);
	virtual float FlWeaponTryRespawn(CBasePlayerItem *pWeapon);
	virtual Vector VecWeaponRespawnSpot(CBasePlayerItem *pWeapon);

	// Item retrieval
	virtual BOOL CanHaveItem(CBasePlayer *pPlayer, CItem *pItem);
	virtual void PlayerGotItem(CBasePlayer *pPlayer, CItem *pItem);

	// Item spawn/respawn control
	virtual int ItemShouldRespawn(CItem *pItem);
	virtual float FlItemRespawnTime(CItem *pItem);
	virtual Vector VecItemRespawnSpot(CItem *pItem);

	// Ammo retrieval
	virtual void PlayerGotAmmo(CBasePlayer *pPlayer, char *szName, int iCount);

	// Ammo spawn/respawn control
	virtual int AmmoShouldRespawn(CBasePlayerAmmo *pAmmo);
	virtual float FlAmmoRespawnTime(CBasePlayerAmmo *pAmmo);
	virtual Vector VecAmmoRespawnSpot(CBasePlayerAmmo *pAmmo);

	// Healthcharger respawn control
	virtual float FlHealthChargerRechargeTime(void);

	// What happens to a dead player's weapons
	virtual int DeadPlayerWeapons(CBasePlayer *pPlayer);

	// What happens to a dead player's ammo
	virtual int DeadPlayerAmmo(CBasePlayer *pPlayer);

	// Monsters
	virtual BOOL FAllowMonsters(void);

	// Teamplay stuff
	virtual const char *GetTeamID(CBaseEntity *pEntity) { return ""; };
	virtual int PlayerRelationship(CBaseEntity *pPlayer, CBaseEntity *pTarget);
};

//=========================================================
// CHalfLifeMultiplay - rules for the basic half life multiplayer
// competition
//=========================================================

#define MAX_VIP_QUEUES 5

enum EventMessage
{
	Event_Target_Bombed = 1,
	Event_VIP_Escaped,
	Event_VIP_Assassinated,
	Event_Terrorists_Escaped,
	Event_CTs_PreventEscape,
	Event_Escaping_Terrorists_Neutralized,
	Event_Bomb_Defused,
	Event_CTs_Win,
	Event_Terrorists_Win,
	Event_Round_Draw,
	Event_All_Hostages_Rescued,
	Event_Target_Saved,
	Event_Hostages_Not_Rescued,
	Event_Terrorists_Not_Escaped,
	Event_VIP_Not_Escaped,
	Event_Game_Commencing,
};

enum WinStatus
{
	WinStatus_CT = 1,
	WinStatus_Terrorist,
	WinStatus_Draw
};

class CHalfLifeMultiplay : public CGameRules
{
public:

	CHalfLifeMultiplay();

	// GR_Think
	virtual void Think(void);
	virtual void RefreshSkillData(void);
	virtual BOOL IsAllowedToSpawn(CBaseEntity *pEntity);
	virtual BOOL FAllowFlashlight(void);

	virtual BOOL FShouldSwitchWeapon(CBasePlayer *pPlayer, CBasePlayerItem *pWeapon);
	virtual BOOL GetNextBestWeapon(CBasePlayer *pPlayer, CBasePlayerItem *pCurrentWeapon);

	// Functions to verify the single/multiplayer status of a game
	virtual BOOL IsMultiplayer(void);
	virtual BOOL IsDeathmatch(void);
	virtual BOOL IsCoOp(void);

	// Career
	BOOL IsCareer(void) { return IS_CAREER_MATCH(); }
	void QueueCareerRoundEndMenu(float tmDelay, int iWinStatus);
	void SetCareerMatchLimit(int minWins, int winDifference);
	bool IsInCareerRound(void);
	void CareerRestart(void);

	bool ShouldSkipSpawn(void);
	void MarkSpawnSkipped(void);

	// Client connection/disconnection
	virtual BOOL ClientConnected(edict_t *pEntity, const char *pszName, const char *pszAddress, char szRejectReason[128]);
	virtual void InitHUD(CBasePlayer *pl);
	virtual void ClientDisconnected(edict_t *pClient);
	virtual void UpdateGameMode(CBasePlayer *pPlayer);

	// Client damage rules
	virtual float FlPlayerFallDamage(CBasePlayer *pPlayer);
	virtual BOOL FPlayerCanTakeDamage(CBasePlayer *pPlayer, CBaseEntity *pAttacker);

	// Client spawn/respawn control
	virtual void PlayerSpawn(CBasePlayer *pPlayer);
	virtual void PlayerThink(CBasePlayer *pPlayer);
	virtual BOOL FPlayerCanRespawn(CBasePlayer *pPlayer);
	virtual float FlPlayerSpawnTime(CBasePlayer *pPlayer);
	virtual edict_t * GetPlayerSpawnSpot(CBasePlayer *pPlayer);

	virtual BOOL AllowAutoTargetCrosshair(void);

	virtual int IPointsForKill(CBasePlayer *pAttacker, CBasePlayer *pKilled);
	virtual void PlayerKilled(CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor);
	virtual void DeathNotice(CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor);

	// Weapon retrieval
	virtual void PlayerGotWeapon(CBasePlayer *pPlayer, CBasePlayerItem *pWeapon);
	virtual BOOL CanHavePlayerItem(CBasePlayer *pPlayer, CBasePlayerItem *pWeapon);

	// Weapon spawn/respawn control
	virtual int WeaponShouldRespawn(CBasePlayerItem *pWeapon);
	virtual float FlWeaponRespawnTime(CBasePlayerItem *pWeapon);
	virtual float FlWeaponTryRespawn(CBasePlayerItem *pWeapon);
	virtual Vector VecWeaponRespawnSpot(CBasePlayerItem *pWeapon);

	// Item retrieval
	virtual BOOL CanHaveItem(CBasePlayer *pPlayer, CItem *pItem);
	virtual void PlayerGotItem(CBasePlayer *pPlayer, CItem *pItem);

	// Item spawn/respawn control
	virtual int ItemShouldRespawn(CItem *pItem);
	virtual float FlItemRespawnTime(CItem *pItem);
	virtual Vector VecItemRespawnSpot(CItem *pItem);

	// Ammo retrieval
	virtual void PlayerGotAmmo(CBasePlayer *pPlayer, char *szName, int iCount);

	// Ammo spawn/respawn control
	virtual int AmmoShouldRespawn(CBasePlayerAmmo *pAmmo);
	virtual float FlAmmoRespawnTime(CBasePlayerAmmo *pAmmo);
	virtual Vector VecAmmoRespawnSpot(CBasePlayerAmmo *pAmmo);

	// Healthcharger respawn control
	virtual float FlHealthChargerRechargeTime(void);
	virtual float FlHEVChargerRechargeTime(void);

	// What happens to a dead player's weapons
	virtual int DeadPlayerWeapons(CBasePlayer *pPlayer);

	// What happens to a dead player's ammo
	virtual int DeadPlayerAmmo(CBasePlayer *pPlayer);

	// Teamplay stuff
	virtual const char *GetTeamID(CBaseEntity *pEntity) { return ""; }
	virtual int PlayerRelationship(CBaseEntity *pPlayer, CBaseEntity *pTarget);

	virtual BOOL PlayTextureSounds(void) { return FALSE; }

	// Monsters
	virtual BOOL FAllowMonsters(void);

	// Immediately end a multiplayer game
	virtual void EndMultiplayerGame(void);
	virtual void ServerDeactivate(void);

	// Player stuff
	virtual BOOL ClientCommand_DeadOrAlive(CBasePlayer *pPlayer, const char *pcmd);
	virtual BOOL ClientCommand(CBasePlayer *pPlayer, const char *pcmd);
	virtual void ClientUserInfoChanged(CBasePlayer *pPlayer, char *infobuffer);
	void PlayerJoinedTeam(CBasePlayer *pPlayer);

	virtual void CleanUpMap(void);
	virtual void RestartRound(void);

	virtual void CheckWinConditions(void);
	virtual void CheckMapConditions(void);

	virtual void RemoveGuns(void);
	virtual void GiveC4(void);

	float TimeRemaining(void) { return m_iRoundTimeSecs - gpGlobals->time + m_fRoundCount; }

	BOOL TeamFull(int team_id);
	BOOL TeamStacked(int newTeam_id, int curTeam_id);

	// VIP Stuff
	bool IsVIPQueueEmpty(void);
	bool AddToVIPQueue(CBasePlayer *toAdd);
	void PickNextVIP(void);
	void StackVIPQueue(void);
	void ResetCurrentVIP(void);

	// Team stuff
	void BalanceTeams(void);
	void SwapAllPlayers(void);
	void UpdateTeamScores(void);

	// Map stuff
	void DisplayMaps(CBasePlayer *pPlayer, int iVote);
	void ResetAllMapVotes(void);
	void ProcessMapVote(CBasePlayer *pPlayer, int iVote);

	// C4 stuff
	BOOL IsThereABomber(void);
	BOOL IsThereABomb(void);

	bool IsMatchStarted(void);

	virtual void ChangeLevel(void);
	virtual void GoToIntermission(void);

	void SendMOTDToClient(edict_t *client);

private:

	bool HasRoundTimeExpired(void);
	bool IsBombPlanted(void);

	void MarkLivingPlayersOnTeamAsNotReceivingMoneyNextRound(int iTeam);

	inline void TerminateRound(float tmDelay, int iWinStatus)
	{
		m_iRoundWinStatus   = iWinStatus;
		m_bRoundTerminating = true;
		m_fTeamCount        = gpGlobals->time + tmDelay;
	}

public:

	CVoiceGameMgr               m_VoiceGameMgr;                     /*    12     0 */

	float                       m_fTeamCount;                       /*    40     4 */
	float                       m_flCheckWinConditions;             /*    44     4 */
	float                       m_fRoundCount;                      /*    48     4 */
	int                         m_iRoundTime;                       /*    52     4 */
	int                         m_iRoundTimeSecs;                   /*    56     4 */
	int                         m_iIntroRoundTime;                  /*    60     4 */
	float                       m_fIntroRoundCount;                 /*    64     4 */

	int                         m_iAccountTerrorist;                /*    68     4 */
	int                         m_iAccountCT;                       /*    72     4 */

	int                         m_iNumTerrorist;                    /*    76     4 */
	int                         m_iNumCT;                           /*    80     4 */
	int                         m_iNumSpawnableTerrorist;           /*    84     4 */
	int                         m_iNumSpawnableCT;                  /*    88     4 */
	int                         m_iSpawnPointCount_Terrorist;       /*    92     4 */
	int                         m_iSpawnPointCount_CT;              /*    96     4 */

	int                         m_iHostagesRescued;                 /*   100     4 */
	int                         m_iHostagesTouched;                 /*   104     4 */

	int                         m_iRoundWinStatus;                  /*   108     4 */
	short int                   m_iNumCTWins;                       /*   112     2 */
	short int                   m_iNumTerroristWins;                /*   114     2 */

	bool                        m_bTargetBombed;                    /*   116     1 */
	bool                        m_bBombDefused;                     /*   117     1 */

	bool                        m_bMapHasBombTarget;                /*   118     1 */
	bool                        m_bMapHasBombZone;                  /*   119     1 */
	bool                        m_bMapHasBuyZone;                   /*   120     1 */
	bool                        m_bMapHasRescueZone;                /*   121     1 */
	bool                        m_bMapHasEscapeZone;                /*   122     1 */
	int                         m_iMapHasVIPSafetyZone;             /*   124     4 */
	int                         m_bMapHasCameras;                   /*   128     4 */

	int                         m_iC4Timer;                         /*   132     4 */
	int                         m_iC4Guy;                           /*   136     4 */

	int                         m_iLoserBonus;                      /*   140     4 */
	int                         m_iNumConsecutiveCTLoses;           /*   144     4 */
	int                         m_iNumConsecutiveTerroristLoses;    /*   148     4 */

	float                       m_fMaxIdlePeriod;                   /*   152     4 */
	int                         m_iLimitTeams;                      /*   156     4 */
	bool                        m_bLevelInitialized;                /*   160     1 */
	bool                        m_bRoundTerminating;                /*   161     1 */
	bool                        m_bCompleteReset;                   /*   162     1 */

	float                       m_flRequiredEscapeRatio;            /*   164     4 */
	int                         m_iNumEscapers;                     /*   168     4 */
	int                         m_iHaveEscaped;                     /*   172     4 */

	bool                        m_bCTCantBuy;                       /*   176     1 */
	bool                        m_bTCantBuy;                        /*   177     1 */

	float                       m_flBombRadius;                     /*   180     4 */

	int                         m_iConsecutiveVIP;                  /*   184     4 */

	int                         m_iTotalGunCount;                   /*   188     4 */
	int                         m_iTotalGrenadeCount;               /*   192     4 */
	int                         m_iTotalArmourCount;                /*   196     4 */

	int                         m_iUnBalancedRounds;                /*   200     4 */
	int                         m_iNumEscapeRounds;                 /*   204     4 */

	int                         m_iMapVotes[100];                   /*   208   400 */
	int                         m_iLastPick;                        /*   608     4 */
	int                         m_iMaxMapTime;                      /*   612     4 */
	int                         m_iMaxRounds;                       /*   616     4 */
	int                         m_iTotalRoundsPlayed;               /*   620     4 */
	int                         m_iMaxRoundsWon;                    /*   624     4 */

	int                         m_iStoredSpectValue;                /*   628     4 */
	float                       m_flForceCameraValue;               /*   632     4 */
	float                       m_flForceChaseCamValue;             /*   636     4 */
	float                       m_flFadeToBlackValue;               /*   640     4 */

	class CBasePlayer *         m_pVIP;                             /*   644     4 */
	class CBasePlayer *         VIPQueue[5];                        /*   648    20 */

protected:

	float                      m_flIntermissionEndTime;             /*   668     4 */
	float                      m_flIntermissionStartTime;           /*   672     4 */
	BOOL                       m_iEndIntermissionButtonHit;         /*   676     4 */

	float                      m_tmNextPeriodicThink;               /*   680     4 */
	bool                       m_bFirstConnected;                   /*   684     1 */
	bool                       m_bInCareerGame;                     /*   685     1 */
	float                      m_fCareerRoundMenuTime;              /*   688     4 */
	int                        m_iCareerMatchWins;                  /*   692     4 */
	int                        m_iRoundWinDifference;               /*   696     4 */
	float                      m_fCareerMatchMenuTime;              /*   700     4 */
	bool                       m_bSkipSpawn;                        /*   704     1 */

	/* vtable has 60 entries:
	{
	[1]  = Think
	[0]  = RefreshSkillData
	[2]  = IsAllowedToSpawn
	[3]  = FAllowFlashlight
	[4]  = FShouldSwitchWeapon
	[5]  = GetNextBestWeapon
	[6]  = IsMultiplayer
	[7]  = IsDeathmatch
	[9]  = IsCoOp
	[11] = ClientConnected
	[12] = InitHUD
	[13] = ClientDisconnected
	[14] = UpdateGameMode
	[15] = FlPlayerFallDamage
	[16] = FPlayerCanTakeDamage
	[18] = PlayerSpawn
	[19] = PlayerThink
	[20] = FPlayerCanRespawn
	[21] = FlPlayerSpawnTime
	[22] = GetPlayerSpawnSpot
	[23] = AllowAutoTargetCrosshair
	[27] = IPointsForKill
	[28] = PlayerKilled
	[29] = DeathNotice
	[31] = PlayerGotWeapon
	[30] = CanHavePlayerItem
	[32] = WeaponShouldRespawn
	[33] = FlWeaponRespawnTime
	[34] = FlWeaponTryRespawn
	[35] = VecWeaponRespawnSpot
	[36] = CanHaveItem
	[37] = PlayerGotItem
	[38] = ItemShouldRespawn
	[39] = FlItemRespawnTime
	[40] = VecItemRespawnSpot
	[42] = PlayerGotAmmo
	[43] = AmmoShouldRespawn
	[44] = FlAmmoRespawnTime
	[45] = VecAmmoRespawnSpot
	[46] = FlHealthChargerRechargeTime
	[47] = FlHEVChargerRechargeTime
	[48] = DeadPlayerWeapons
	[49] = DeadPlayerAmmo
	[50] = GetTeamID
	[51] = PlayerRelationship
	[57] = PlayTextureSounds
	[58] = FAllowMonsters
	[59] = EndMultiplayerGame
	[61] = ServerDeactivate
	[24] = ClientCommand_DeadOrAlive
	[25] = ClientCommand
	[26] = ClientUserInfoChanged
	[63] = CleanUpMap
	[64] = RestartRound
	[65] = CheckWinConditions
	[62] = CheckMapConditions
	[66] = RemoveGuns
	[67] = GiveC4
	[68] = ChangeLevel
	[69] = GoToIntermission
	} */
	/* size: 708, cachelines: 12, members: 76 */
	/* sum members: 659, holes: 6, sum holes: 46 */
	/* padding: 3 */
	/* last cacheline: 4 bytes */
};

extern DLL_GLOBAL CHalfLifeMultiplay* g_pGameRules;

extern void Broadcast(const char *sentence);
extern char *GetTeam(int teamNo);