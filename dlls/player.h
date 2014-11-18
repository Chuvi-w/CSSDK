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
#ifndef PLAYER_H
#define PLAYER_H

#include "pm_materials.h"
#include "unisignals.h"
#include "hintmessage.h"
#include "weapons.h"
#include "shared_util.h"

#define PLAYER_FATAL_FALL_SPEED		 1100
#define PLAYER_MAX_SAFE_FALL_SPEED	 500
#define DAMAGE_FOR_FALL_SPEED		 ( float )100 / ( PLAYER_FATAL_FALL_SPEED - PLAYER_MAX_SAFE_FALL_SPEED ) // damage per unit per second.
#define PLAYER_MIN_BOUNCE_SPEED		 350
#define PLAYER_FALL_PUNCH_THRESHHOLD ( float )350 // won't punch player's screen/make scrape noise unless player falling at least this fast.

#define MAX_PLAYER_NAME_LENGTH 32
#define MAX_AUTOBUY_LENGTH     256
#define MAX_REBUY_LENGTH       256

//
// Player PHYSICS FLAGS bits
//
#define		PFLAG_ONLADDER		( 1<<0 )
#define		PFLAG_ONSWING		( 1<<0 )
#define		PFLAG_ONTRAIN		( 1<<1 )
#define		PFLAG_ONBARNACLE	( 1<<2 )
#define		PFLAG_DUCKING		( 1<<3 )		// In the process of ducking, but totally squatted yet
#define		PFLAG_USING			( 1<<4 )		// Using a continuous entity
#define		PFLAG_OBSERVER		( 1<<5 )		// player is locked in stationary cam mode. Spectators can move, observers can't.

//
// generic player
//
//-----------------------------------------------------
//This is Half-Life player entity
//-----------------------------------------------------
#define CSUITPLAYLIST	4		// max of 4 suit sentences queued up at any time

#define SUIT_GROUP			TRUE
#define	SUIT_SENTENCE		FALSE

#define	SUIT_REPEAT_OK		0
#define SUIT_NEXT_IN_30SEC	30
#define SUIT_NEXT_IN_1MIN	60
#define SUIT_NEXT_IN_5MIN	300
#define SUIT_NEXT_IN_10MIN	600
#define SUIT_NEXT_IN_30MIN	1800
#define SUIT_NEXT_IN_1HOUR	3600

#define CSUITNOREPEAT		32

#define	SOUND_FLASHLIGHT_ON		"items/flashlight1.wav"
#define	SOUND_FLASHLIGHT_OFF	"items/flashlight1.wav"

#define TEAM_NAME_LENGTH	16

#define IGNOREMSG_NONE  0
#define IGNOREMSG_ENEMY 1
#define IGNOREMSG_TEAM  2

typedef enum
{
	PLAYER_IDLE,
	PLAYER_WALK,
	PLAYER_JUMP,
	PLAYER_SUPERJUMP,
	PLAYER_DIE,
	PLAYER_ATTACK1,

	// CS
	PLAYER_ATTACK2,
	PLAYER_FLINCH,
	PLAYER_LARGE_FLINCH,
	PLAYER_RELOAD,
	PLAYER_HOLDBOMB,
} PLAYER_ANIM;

#define MAX_ID_RANGE 2048
#define SBAR_STRING_SIZE 128

enum sbar_data
{
	SBAR_ID_TARGETNAME = 1,
	SBAR_ID_TARGETHEALTH,
	SBAR_ID_TARGETARMOR,
	SBAR_END,
};

// CS
typedef enum
{
	UNASSIGNED = 0,
	TERRORIST,
	CT,
	SPECTATOR,
} TeamName;

// CS
typedef enum
{
	SILENT = 0,
	CALM = 1,
	INTENSE = 2,
} MusicState;

// CS
typedef enum
{
	JOINED = 0,
	SHOWLTEXT,
	READINGLTEXT,
	SHOWTEAMSELECT,
	PICKINGTEAM,
	GETINTOGAME,
} JoinState;

// CS
typedef enum
{
	MODEL_UNASSIGNED = 0,
	MODEL_URBAN,
	MODEL_TERROR,
	MODEL_LEET,
	MODEL_ARCTIC,
	MODEL_GSG9,
	MODEL_GIGN,
	MODEL_SAS,
	MODEL_GUERILLA,
	MODEL_VIP,
	MODEL_MILITIA,
	MODEL_SPETSNAZ,
} ModelName;

// CS
typedef enum
{
	Menu_OFF = 0,
	Menu_ChooseTeam,
	Menu_IGChooseTeam,
	Menu_ChooseAppearance,
	Menu_Buy,
	Menu_BuyPistol,
	Menu_BuyRifle,
	Menu_BuyMachineGun,
	Menu_BuyShotgun,
	Menu_BuySubMachineGun,
	Menu_BuyItem,
	Menu_Radio1,
	Menu_Radio2,
	Menu_Radio3,
	Menu_ClientBuy,
} _Menu;

// CS
typedef enum
{
	CMD_SAY = 0,
	CMD_SAYTEAM,
	CMD_FULLUPDATE,
	CMD_VOTE,
	CMD_VOTEMAP,
	CMD_LISTMAPS,
	CMD_LISTPLAYERS,
	CMD_NIGHTVISION,
	COMMANDS_TO_TRACK,
} TrackCommands;

// CS
typedef struct
{
	int m_primaryWeapon;
	int m_primaryAmmo;
	int m_secondaryWeapon;
	int m_secondaryAmmo;
	int m_heGrenade;
	int m_flashbang;
	int m_smokeGrenade;
	int m_defuser;
	int m_nightVision;
	int m_armor;
} RebuyStruct;

typedef enum
{
	THROW_NONE,
	THROW_FORWARD,
	THROW_BACKWARD,
	THROW_HITVEL,
	THROW_BOMB,
	THROW_GRENADE,
	THROW_HITVEL_MINUS_AIRVEL
}
ThrowDirection;

// CS
typedef enum
{
	AUTOBUYCLASS_PRIMARY     = 1 << 0,  // 1
	AUTOBUYCLASS_SECONDARY   = 1 << 1,  // 2
	AUTOBUYCLASS_AMMO        = 1 << 2,  // 4
	AUTOBUYCLASS_ARMOR       = 1 << 3,  // 8
	AUTOBUYCLASS_DEFUSER     = 1 << 4,  // 16
	AUTOBUYCLASS_PISTOL      = 1 << 5,  // 32
	AUTOBUYCLASS_SMG         = 1 << 6,  // 64
	AUTOBUYCLASS_RIFLE       = 1 << 7,  // 128
	AUTOBUYCLASS_SNIPERRIFLE = 1 << 8,  // 256
	AUTOBUYCLASS_SHOTGUN     = 1 << 9,  // 512
	AUTOBUYCLASS_MACHINEGUN  = 1 << 10, // 1024
	AUTOBUYCLASS_GRENADE     = 1 << 11, // 4096
	AUTOBUYCLASS_NIGHTVISION = 1 << 12, // 8192
	AUTOBUYCLASS_SHIELD      = 1 << 13, // 16384
} AutoBuyClassType;

typedef struct
{
	int /* AutoBuyClassType*/   m_class;              /*     0     4 */
	char                       *m_command;            /*     4     4 */
	char                       *m_classname;          /*     8     4 */
} AutoBuyInfoStruct;

#define CHAT_INTERVAL 1.0f

class CBasePlayer : public CBaseMonster
{
public:
	int                         random_seed;                /*   404     4 */ // See that is shared between client & server for shared weapons code
	short unsigned int          m_usPlayerBleed;            /*   408     2 */

	EHANDLE                     m_hObserverTarget;          /*   412     8 */
	float                       m_flNextObserverInput;      /*   420     4 */
	int                         m_iObserverWeapon;          /*   424     4 */
	int                         m_iObserverC4State;         /*   428     4 */
	bool                        m_bObserverHasDefuser;      /*   432     1 */
	int                         m_iObserverLastMode;        /*   436     4 */

	float                       m_flFlinchTime;             /*   440     4 */
	float                       m_flAnimTime;               /*   444     4 */

	bool                        m_bHighDamage;              /*   448     1 */
	float                       m_flVelocityModifier;       /*   452     4 */

	int                         m_iLastZoom;                /*   456     4 */
	bool                        m_bResumeZoom;              /*   460     1 */
	float                       m_flEjectBrass;             /*   464     4 */

	int                         m_iKevlar;                  /*   468     4 */
	bool                        m_bNotKilled;               /*   472     1 */
	TeamName                    m_iTeam;                    /*   476     4 */
	int                         m_iAccount;                 /*   480     4 */
	bool                        m_bHasPrimary;              /*   484     1 */

	float                       m_flDeathThrowTime;         /*   488     4 */
	int                         m_iThrowDirection;          /*   492     4 */

	float                       m_flLastTalk;               /*   496     4 */
	bool                        m_bJustConnected;           /*   500     1 */
	bool                        m_bContextHelp;             /*   501     1 */
	JoinState                   m_iJoiningState;            /*   504     4 */

	CBaseEntity                *m_pIntroCamera;             /*   508     4 */
	float                       m_fIntroCamTime;            /*   512     4 */

	float                       m_fLastMovement;            /*   516     4 */
	bool                        m_bMissionBriefing;         /*   520     1 */
	bool                        m_bTeamChanged;             /*   521     1 */
	ModelName                   m_iModelName;               /*   524     4 */
	int                         m_iTeamKills;               /*   528     4 */
	int                         m_iIgnoreGlobalChat;        /*   532     4 */

	bool                        m_bHasNightVision;          /*   536     1 */
	bool                        m_bNightVisionOn;           /*   537     1 */

	Vector                      m_vRecentPath[20];          /*   540   240 */

	float                       m_flIdleCheckTime;          /*   780     4 */
	float                       m_flRadioTime;              /*   784     4 */
	int                         m_iRadioMessages;           /*   788     4 */
	bool                        m_bIgnoreRadio;             /*   792     1 */

	bool                        m_bHasC4;                   /*   793     1 */
	bool                        m_bHasDefuser;              /*   794     1 */
	bool                        m_bKilledByBomb;            /*   795     1 */
	Vector                      m_vBlastVector;             /*   796    12 */
	bool                        m_bKilledByGrenade;         /*   808     1 */

	CHintMessageQueue           m_hintMessageQueue;         /*   812    24 */
	int32                       m_flDisplayHistory;         /*   836     4 */

	_Menu                       m_iMenu;                    /*   840     4 */

	int                         m_iChaseTarget;             /*   844     4 */
	CBaseEntity                *m_pChaseTarget;             /*   848     4 */
	float                       m_fCamSwitch;               /*   852     4 */

	bool                        m_bEscaped;                 /*   856     1 */
	bool                        m_bIsVIP;                   /*   857     1 */

	float                       m_tmNextRadarUpdate;        /*   860     4 */
	Vector                      m_vLastOrigin;              /*   864    12 */

	int                         m_iCurrentKickVote;         /*   876     4 */
	float                       m_flNextVoteTime;           /*   880     4 */

	bool                        m_bJustKilledTeammate;      /*   884     1 */
	int                         m_iHostagesKilled;          /*   888     4 */

	int                         m_iMapVote;                 /*   892     4 */

	bool                        m_bCanShoot;                /*   896     1 */
	float                       m_flLastFired;              /*   900     4 */
	float                       m_flLastAttackedTeammate;   /*   904     4 */
	bool                        m_bHeadshotKilled;          /*   908     1 */
	bool                        m_bPunishedForTK;           /*   909     1 */
	bool                        m_bReceivesNoMoneyNextRound;/*   910     1 */

	int                         m_iTimeCheckAllowed;        /*   912     4 */
	bool                        m_bHasChangedName;          /*   916     1 */
	char                        m_szNewName[32];            /*   917    32 */
	bool                        m_bIsDefusing;              /*   949     1 */

	float                       m_tmHandleSignals;          /*   952     4 */
	CUnifiedSignals             m_signals;                  /*   956     8 */

	edict_t                    *m_pentCurBombTarget;        /*   964     4 */

	int                         m_iPlayerSound;             /*   968     4 */ // The index of the sound list slot reserved for this player.
	int                         m_iTargetVolume;            /*   972     4 */ // Ideal sound volume.
	int                         m_iWeaponVolume;            /*   976     4 */ // How loud the player's weapon is right now.
	int                         m_iExtraSoundTypes;         /*   980     4 */ // Additional classification for this weapon's sound.
	int                         m_iWeaponFlash;             /*   984     4 */ // Brightness of the weapon flash.
	float                       m_flStopExtraSoundTime;     /*   988     4 */

	float                       m_flFlashLightTime;         /*   992     4 */ // Time until next battery draw/Recharge.
	int                         m_iFlashBattery;            /*   996     4 */ // Flashlight Battery Draw.

	int                         m_afButtonLast;             /*  1000     4 */
	int                         m_afButtonPressed;          /*  1004     4 */
	int                         m_afButtonReleased;         /*  1008     4 */

	edict_t                    *m_pentSndLast;              /*  1012     4 */ // Last sound entity to modify player room type.
	float                       m_flSndRoomtype;            /*  1016     4 */ // Last roomtype set by sound entity.
	float                       m_flSndRange;               /*  1020     4 */ // Dist from player to sound entity.

	float                       m_flFallVelocity;           /*  1024     4 */

	int                         m_rgItems[4];               /*  1028    16 */
	int                         m_fNewAmmo;                 /*  1044     4 */ // True when a new item has been added.

	unsigned int                m_afPhysicsFlags;           /*  1048     4 */ // Physics flags - set when 'normal' physics should be revisited or overriden.
	float                       m_fNextSuicideTime;         /*  1052     4 */ // The time after which the player can next use the suicide command.

	float                       m_flTimeStepSound;          /*  1056     4 */ // When the last stepping sound was made.
	float                       m_flTimeWeaponIdle;         /*  1060     4 */ // When to play another weapon idle animation.
	float                       m_flSwimTime;               /*  1064     4 */ // How long player has been underwater.
	float                       m_flDuckTime;               /*  1068     4 */ // How long we've been ducking.
	float                       m_flWallJumpTime;           /*  1072     4 */ // How long until next walljump.

	float                       m_flSuitUpdate;                         /*  1076     4 */ // When to play next suit update.
	int                         m_rgSuitPlayList[CSUITPLAYLIST];        /*  1080    16 */ // Next sentencenum to play for suit update.
	int                         m_iSuitPlayNext;                        /*  1096     4 */ // Next sentence slot for queue storage.
	int                         m_rgiSuitNoRepeat[CSUITNOREPEAT];       /*  1100   128 */ // Suit sentence no repeat list.
	float                       m_rgflSuitNoRepeatTime[CSUITNOREPEAT];  /*  1228   128 */ // How long to wait before allowing repeat.
	int                         m_lastDamageAmount;                     /*  1356     4 */ // Last damage taken.
	float                       m_tbdPrev;                              /*  1360     4 */ // Time-based damage timer.

	float                       m_flgeigerRange;            /*  1364     4 */ // Range to nearest radiation source.
	float                       m_flgeigerDelay;            /*  1368     4 */ // Delay per update of range msg to client.
	int                         m_igeigerRangePrev;         /*  1372     4 */
	int                         m_iStepLeft;                /*  1376     4 */ // Alternate left/right foot stepping sound.
	char                        m_szTextureName[17];        /*  1380    17 */ // Current texture name we're standing on.
	char                        m_chTextureType;            /*  1397     1 */ // Current texture type.

	int                         m_idrowndmg;                /*  1400     4 */ // Track drowning damage taken.
	int                         m_idrownrestored;           /*  1404     4 */ // Track drowning damage restored.

	int                         m_bitsHUDDamage;            /*  1408     4 */ // Damage bits for the current fame. These get sent to the hud via the DAMAGE message.
	BOOL                        m_fInitHUD;                 /*  1412     4 */ // True when deferred HUD restart msg needs to be sent.
	BOOL                        m_fGameHUDInitialized;      /*  1416     4 */
	int                         m_iTrain;                   /*  1420     4 */ // Train control position.
	BOOL                        m_fWeapon;                  /*  1424     4 */ // Set this to FALSE to force a reset of the current weapon HUD info.

	EHANDLE                     m_pTank;                    /*  1428     8 */ // The tank which the player is currently controlling,  NULL if no tank.
	float                       m_fDeadTime;                /*  1436     4 */ // The time at which the player died  (used in PlayerDeathThink()).

	BOOL                        m_fNoPlayerSound;           /*  1440     4 */ // A debugging feature. Player makes no sound if this is true.
	BOOL                        m_fLongJump;                /*  1444     4 */ // Does this player have the longjump module?.

	float                       m_tSneaking;                /*  1448     4 */
	int                         m_iUpdateTime;              /*  1452     4 */ // stores the number of frame ticks before sending HUD update messages
	int                         m_iClientHealth;            /*  1456     4 */ // the health currently known by the client.  If this changes, send a new
	int                         m_iClientBattery;           /*  1460     4 */ // the Battery currently known by the client.  If this changes, send a new
	int                         m_iHideHUD;                 /*  1464     4 */ // the players hud weapon info is to be hidden
	int                         m_iClientHideHUD;           /*  1468     4 */
	int                         m_iFOV;                     /*  1472     4 */ // field of view
	int                         m_iClientFOV;               /*  1476     4 */ // client's known FOV

	int                         m_iNumSpawns;               /*  1480     4 */

	CBaseEntity                *m_pObserver;                /*  1484     4 */

	CBasePlayerItem            *m_rgpPlayerItems[MAX_ITEM_TYPES];  /*  1488    24 */
	CBasePlayerItem            *m_pActiveItem;              /*  1512     4 */
	CBasePlayerItem            *m_pClientActiveItem;        /*  1516     4 */
	CBasePlayerItem            *m_pLastItem;                /*  1520     4 */

	int                         m_rgAmmo[32];               /*  1524   128 */
	int                         m_rgAmmoLast[32];           /*  1652   128 */

	Vector                      m_vecAutoAim;               /*  1780    12 */
	BOOL                        m_fOnTarget;                /*  1792     4 */

	int                         m_iDeaths;                  /*  1796     4 */

	int                         m_izSBarState[SBAR_END];    /*  1800    16 */
	float                       m_flNextSBarUpdateTime;     /*  1816     4 */
	float                       m_flStatusBarDisappearDelay;/*  1820     4 */
	char                        m_SbarString0[SBAR_STRING_SIZE];  /*  1824   128 */

	int                         m_lastx;                    /*  1952     4 */ // These are the previous update's crosshair angles, DON"T SAVE/RESTORE
	int                         m_lasty;                    /*  1956     4 */

	int                         m_nCustomSprayFrames;       /*  1960     4 */ // Custom clan logo frames for this player
	float                       m_flNextDecalTime;          /*  1964     4 */

	char                        m_szTeamName[TEAM_NAME_LENGTH];  /*  1968    16 */ // next time this player can spray a decal
	int                         m_modelIndexPlayer;         /*  1984     4 */

	static TYPEDESCRIPTION      m_playerSaveData[];         /*     0   640 */

	char                        m_szAnimExtention[32];      /*  1988    32 */
	int                         m_iGaitsequence;            /*  2020     4 */
	float                       m_flGaitframe;              /*  2024     4 */
	float                       m_flGaityaw;                /*  2028     4 */
	Vector                      m_prevgaitorigin;           /*  2032    12 */
	float                       m_flPitch;                  /*  2044     4 */
	float                       m_flYaw;                    /*  2048     4 */
	float                       m_flGaitMovement;           /*  2052     4 */

	int                         m_iAutoWepSwitch;           /*  2056     4 */
	bool                        m_bVGUIMenus;               /*  2060     1 */
	bool                        m_bShowHints;               /*  2061     1 */
	bool                        m_bShieldDrawn;             /*  2062     1 */
	bool                        m_bOwnsShield;              /*  2063     1 */
	bool                        m_bWasFollowing;            /*  2064     1 */

	float                       m_flNextFollowTime;         /*  2068     4 */
	float                       m_flYawModifier;            /*  2072     4 */

	float                       m_blindUntilTime;           /*  2076     4 */
	float                       m_blindStartTime;           /*  2080     4 */
	float                       m_blindHoldTime;            /*  2084     4 */
	float                       m_blindFadeTime;            /*  2088     4 */
	int                         m_blindAlpha;               /*  2092     4 */

	float                       m_allowAutoFollowTime;      /*  2096     4 */

	char                        m_autoBuyString[256];       /*  2100   256 */
	char                       *m_rebuyString;              /*  2356     4 */
	RebuyStruct                 m_rebuyStruct;              /*  2360    40 */
	bool                        m_bIsInRebuy;               /*  2400     1 */

	float                       m_flLastUpdateTime;         /*  2404     4 */
	char                        m_lastLocation[32];         /*  2408    32 */

	float                       m_progressStart;            /*  2440     4 */
	float                       m_progressEnd;              /*  2444     4 */

	bool                        m_bObserverAutoDirector;    /*  2448     1 */
	bool                        m_canSwitchObserverModes;   /*  2449     1 */

	float                       m_heartBeatTime;            /*  2452     4 */
	float                       m_intenseTimestamp;         /*  2456     4 */
	float                       m_silentTimestamp;          /*  2460     4 */
	MusicState                  m_musicState;               /*  2464     4 */
	float                       m_flLastCommandTime[COMMANDS_TO_TRACK];   /*  2468    32 */

public:

	void SpawnClientSideCorpse(void);

	void Observer_FindNextPlayer(bool bReverse = false, const char *name = NULL);
	CBaseEntity* Observer_IsValidTarget(int iPlayerIndex, bool bSameTeam);
	void Observer_HandleButtons(void);
	void Observer_SetMode(int iMode);
	void Observer_CheckTarget(void);
	void Observer_CheckProperties(void);
	int IsObserver(void) { return pev->iuser1; }  // Last check: 2013, November 18.

	void PlantC4(void);

	void Radio(const char *msg_id, const char *msg_verbose, short int pitch = PITCH_NORM, bool showIcon = true);
	CBasePlayer*  GetNextRadioRecipient(CBasePlayer *pStartPlayer); // Used by bot only.
	void SmartRadio(void);

	void ThrowWeapon(char *pszItemName);
	void ThrowPrimary(void);

	void AddAccount(int amount, bool bTrackChange);
	void Disappear(void);
	void MakeVIP(void);
	bool CanPlayerBuy(bool display);
	void SwitchTeam(void);
	void TabulateAmmo(void);
	void Pain(int m_LastHitGroup, bool HasArmour);

	virtual void Spawn(void);
	virtual void Jump(void);
	virtual void Duck(void);
	virtual void PreThink(void);
	virtual void PostThink(void);

	virtual Vector GetGunPosition(void);

	virtual int TakeHealth(float flHealth, int bitsDamageType);
	virtual void TraceAttack(entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType);
	virtual int TakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType);
	virtual void Killed(entvars_t *pevAttacker, int iGib);

	virtual Vector BodyTarget(const Vector &posSrc) { return Center() + pev->view_ofs * RANDOM_FLOAT(0.5, 1.1); }; // Last check : 2014, November 11.

	virtual void StartSneaking(void) { m_tSneaking = gpGlobals->time - 1.0f; }
	virtual void StopSneaking(void) { m_tSneaking = gpGlobals->time + 30.0f; }
	virtual BOOL IsSneaking(void) { return m_tSneaking <= gpGlobals->time; } // Last check : 2014, November 17.

	virtual BOOL IsAlive(void) { return pev->deadflag == DEAD_NO && pev->health > 0; } // Last check : 2014, November 17.
	virtual BOOL ShouldFadeOnDeath(void) { return FALSE; }
	virtual BOOL IsPlayer(void) { return (pev->flags & FL_SPECTATOR) == 0; }
	virtual BOOL IsBot(void) { return FALSE; } // Last check : 2014, November 17.
	virtual BOOL IsNetClient(void) { return TRUE; } // Last check: 2013, November 17.

	BOOL IsBombGuy(void);
	bool IsLookingAtPosition(const Vector *pos, float angleTolerance);

	virtual const char *TeamID(void);

	virtual int Save(CSave &save);
	virtual int Restore(CRestore &restore);

	void Reset(void);
	void SetScoreboardAttributes(CBasePlayer *destination = NULL);

	void RenewItems(void);
	void PackDeadPlayerItems(void);
	void GiveDefaultItems(void);
	void RemoveAllItems(BOOL removeSuit);
	void SetBombIcon(BOOL bFlash);

	void SetProgressBarTime(int time);
	void SetProgressBarTime2(int time, float timeElapsed);

	void SetPlayerModel(BOOL HasC4);
	void SetNewPlayerModel(const char *modelName);

	BOOL SwitchWeapon(CBasePlayerItem *pWeapon);
	void CheckPowerups(entvars_t *pev);

	bool CanAffordPrimary(void); // Tutor only.
	bool CanAffordPrimaryAmmo(void);
	bool CanAffordSecondaryAmmo(void);
	bool CanAffordArmor(void);
	bool CanAffordDefuseKit(void);
	bool CanAffordGrenade(void);

	bool NeedsPrimaryAmmo(void);  // Tutor only.
	bool NeedsSecondaryAmmo(void);
	bool NeedsArmor(void);
	bool NeedsDefuseKit(void);
	bool NeedsGrenade(void);

	virtual void UpdateClientData(void);
	virtual int ObjectCaps(void) { return CBaseMonster::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; } // Last check: 2013, November 18.
	virtual void Precache(void);

	BOOL IsOnLadder(void);

	BOOL FlashlightIsOn(void);
	void FlashlightTurnOn(void);
	void FlashlightTurnOff(void);

	void UpdatePlayerSound(void);
	void DeathSound(void);

	virtual int Classify(void);

	void SetAnimation(PLAYER_ANIM playerAnim);

	virtual void ImpulseCommands(void);
	void CheatImpulseCommands(int iImpulse);

	virtual void RoundRespawn(void);

	void StartDeathCam(void);
	void StartObserver(Vector vecPosition, Vector vecViewAngle);

	virtual void AddPoints(int score, BOOL bAllowNegativeScore);
	virtual void AddPointsToTeam(int score, BOOL bAllowNegativeScore);

	void HandleSignals(void);

	virtual int AddPlayerItem(CBasePlayerItem *pItem);
	virtual int RemovePlayerItem(CBasePlayerItem *pItem);
	void DropPlayerItem(char *pszItemName);
	BOOL HasPlayerItem(CBasePlayerItem * pCheckItem);
	BOOL HasNamedPlayerItem(const char *pszItemName);
	BOOL HasWeapons(void);

	void SelectPrevItem(int iItem);
	void SelectNextItem(int iItem);
	void SelectLastItem(void);
	void SelectItem(const char *pstr);

	void ItemPreFrame(void);
	void ItemPostFrame(void);

	void GiveNamedItem(const char *szName);
	void EnableControl(BOOL fControl);
	virtual void ResetMaxSpeed(void);
	bool HintMessage(const char *pMessage, BOOL bDisplayIfPlayerDead = FALSE, BOOL bOverride = FALSE);

	virtual int GiveAmmo(int iAmount, char *szName, int iMax);
	void SendAmmoUpdate(void);
	void SendFOV(int fov);

	void WaterMove(void);
	void PlayerDeathThink(void);
	void PlayerUse(void);
	void HostageUsed(void);
	void JoiningThink(void);

	void RemoveLevelText(void);
	void MenuPrint(const char *pszText);
	void ResetMenu(void);

	void SyncRoundTimer(void);
	void CheckSuitUpdate(void);
	void SetSuitUpdate(char *name, int fgroup, int iNoRepeatTime);
	void UpdateGeigerCounter(void);
	void CheckTimeBasedDamage(void);

	virtual BOOL FBecomeProne(void);
	void BarnacleVictimBitten(entvars_t * pevBarnacle);
	void BarnacleVictimReleased(void);

	static int GetAmmoIndex(const char *psz);
	int AmmoInventory(int iAmmoIndex);

	virtual int Illumination(void);
	void ResetAutoaim(void);
	virtual Vector GetAutoaimVector(float flDelta);
	Vector AutoaimDeflection(Vector &vecSrc, float flDist, float flDelta);
	void ForceClientDllUpdate(void);

	void SetCustomDecalFrames(int nFrames);
	int GetCustomDecalFrames(void);

	void InitStatusBar(void);
	void UpdateStatusBar(void);

	void StudioEstimateGait(void);
	void StudioPlayerBlend(int *pBlend, float * pPitch);
	void CalculatePitchBlend(void);
	void CalculateYawBlend(void);
	void StudioProcessGait(void);

	void SendHostagePos(void);
	void SendHostageIcons(void);

	void ResetStamina(void);
	BOOL IsArmored(int nHitGroup);
	BOOL ShouldDoLargeFlinch(int nHitGroup, int nGunType);
	void SetPrefsFromUserinfo(char *infobuffer);
	void SendWeatherInfo(void);

	void UpdateShieldCrosshair(bool draw);
	bool HasShield(void)  { return m_bOwnsShield; }
	bool IsProtectedByShield(void);
	void RemoveShield(void);
	void DropShield(bool bDeploy);
	void GiveShield(bool bDeploy);
	bool IsHittingShield(const Vector &vecDirection, TraceResult * ptr);

	bool IsReloading(void);
	bool IsBlind(const CBasePlayer *pPlayer) { return m_blindUntilTime > gpGlobals->time; }
	virtual void Blind(float duration, float holdTime, float fadeTime, int alpha);
	virtual void OnTouchingWeapon(CWeaponBox *box) { /*empty */ }  // Last check: 2013, November 18.

	// bool IsAutoFollowAllowed( const CBasePlayer *pPlayer); // Unsure.
	void InhibitAutoFollow(float duration);
	void AllowAutoFollow(void);

	void ClearAutoBuyData(void);
	void AddAutoBuyData(const char *str);
	void AutoBuy(void);
	void ClientCommand(const char *arg0, const char *arg1 = NULL, const char *arg2 = NULL, const char *arg3 = NULL);
	void PrioritizeAutoBuyString(char *autobuyString, const char *priorityString);

	const char *PickPrimaryCareerTaskWeapon(void);
	const char *PickSecondaryCareerTaskWeapon(void);
	const char *PickFlashKillWeaponString(void);
	const char *PickGrenadeKillWeaponString(void);

	bool ShouldExecuteAutoBuyCommand(const AutoBuyInfoStruct *commandInfo, bool boughtPrimary, bool boughtSecondary);
	void PostAutoBuyCommandProcessing(const AutoBuyInfoStruct *commandInfo, bool &boughtPrimary, bool &boughtSecondary);
	void ParseAutoBuyString(const char *string, bool &boughtPrimary, bool &boughtSecondary);
	AutoBuyInfoStruct *GetAutoBuyCommandInfo(const char *command);
	void InitRebuyData(const char *str);
	void BuildRebuyStruct(void);

	void Rebuy(void);
	void RebuyPrimaryWeapon(void);
	void RebuyPrimaryAmmo(void);
	void RebuySecondaryWeapon(void);
	void RebuySecondaryAmmo(void);
	void RebuyHEGrenade(void);
	void RebuyFlashbang(void);
	void RebuySmokeGrenade(void);
	void RebuyDefuser(void);
	void RebuyNightVision(void);
	void RebuyArmor(void);

	void UpdateLocation(bool forceUpdate);

	void SetObserverAutoDirector(bool val);
	bool IsObservingPlayer(CBasePlayer *pPlayer);
	//static inline bool CanSwitchObserverModes( const CBasePlayer* pPlayer ); // Unsure.

	void Intense(void);
};

#define AUTOAIM_2DEGREES  0.0348994967025
#define AUTOAIM_5DEGREES  0.08715574274766
#define AUTOAIM_8DEGREES  0.1391731009601
#define AUTOAIM_10DEGREES 0.1736481776669

extern int  gmsgHudTextArgs;
extern int	gmsgHudText;

//CS
extern int gmsgBombDrop;
extern int gmsgCZCareer;

extern BOOL gInitHUD;

#endif // PLAYER_H
