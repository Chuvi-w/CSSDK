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
#ifndef WEAPONS_H
#define WEAPONS_H

#include "effects.h"

class CBasePlayer;
extern int gmsgWeapPickup;

// Contact Grenade / Timed grenade / Satchel Charge
class CGrenade : public CBaseMonster
{
public:
	virtual void Spawn( void );

	virtual int ObjectCaps( void ) { return m_bStartDefuse ? FCAP_IMPULSE_USE : FCAP_FORCE_TRANSITION; }	// CS
	virtual void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );		// CS
	virtual int Save( CSave &save );																		// CS
	virtual int Restore( CRestore &restore );																// CS

	typedef enum { SATCHEL_DETONATE = 0, SATCHEL_RELEASE } SATCHELCODE;

	static CGrenade *ShootTimed( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float time );										// CS (Flashbang Grenade)
	static CGrenade *ShootContact( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity );													
	static CGrenade *ShootSatchelCharge( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity );											// CS (C4 Bomb)
	static CGrenade *ShootTimed2(entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float time, int iTeam, short unsigned int usEvent ); // CS (He Grenade)
	static CGrenade *ShootSmokeGrenade( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float time, short unsigned int usEvent );		// CS (Smoke Grenade)

	static void UseSatchelCharges( entvars_t *pevOwner, SATCHELCODE code );

	void Explode( Vector vecSrc, Vector vecAim );				// CS (Flashbang Grenade)
	void Explode( TraceResult *pTrace, int bitsDamageType );	// CS (Flashbang Grenade)
	void Explode2( TraceResult *pTrace, int bitsDamageType );	// CS (C4 Bomb)
	void Explode3( TraceResult *pTrace, int bitsDamageType );	// CS (He Grenade)
	void SG_Explode( TraceResult *pTrace, int bitsDamageType );	// CS (Smoke Grenade)

	void EXPORT BounceTouch( CBaseEntity *pOther );
	void EXPORT SlideTouch( CBaseEntity *pOther );
	void EXPORT ExplodeTouch( CBaseEntity *pOther );

	void EXPORT C4Touch( CBaseEntity *pOther );					// CS (C4 bomb)
	void EXPORT C4Think( void );								// CS (C4 bomb)

	void EXPORT DangerSoundThink( void );

	void EXPORT Detonate( void );								// CS (Flashbang Grenade)
	void EXPORT Detonate2( void );								// CS (C4 Bomb)
	void EXPORT Detonate3( void );								// CS (He Grenade)
	void EXPORT DetonateUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value ); // CS (Flashbang Grenade)
	void EXPORT PreDetonate( void );
	
	void EXPORT SG_Detonate( void );							// CS (Smoke Grenade)
	void EXPORT SG_Smoke( void );								// CS (Smoke Grenade)
	void EXPORT SG_TumbleThink( void );							// CS (Smoke Grenade)

	void EXPORT Smoke( void );									// CS (Flashbang Grenade)
	void EXPORT Smoke2( void );									// CS (C4 Bomb)
	void EXPORT Smoke3_A( void );
	void EXPORT Smoke3_B( void );
	void EXPORT Smoke3_C( void );								// CS (He Grenade)

	void EXPORT TumbleThink( void );

	virtual void BounceSound( void );
	virtual int	BloodColor( void ) { return DONT_BLEED; }
	virtual void Killed( entvars_t *pevAttacker, int iGib );

	static TYPEDESCRIPTION m_SaveData[];

	// CS
	bool						m_bStartDefuse;       /*   404     1 */
	bool						m_bIsC4;              /*   405     1 */
	EHANDLE						m_pBombDefuser;       /*   408     8 */
	float						m_flDefuseCountDown;  /*   416     4 */
	float						m_flC4Blow;           /*   420     4 */
	float						m_flNextFreqInterval; /*   424     4 */
	float						m_flNextBeep;         /*   428     4 */
	float						m_flNextFreq;         /*   432     4 */
	char *						m_sBeepName;          /*   436     4 */
	float						m_fAttenu;            /*   440     4 */
	float						m_flNextBlink;        /*   444     4 */
	float						m_fNextDefuse;        /*   448     4 */
	bool						m_bJustBlew;          /*   452     1 */
	int							m_iTeam;              /*   456     4 */
	int							m_iCurWave;           /*   460     4 */
	edict_t *					m_pentCurBombTarget;  /*   464     4 */
	int							m_SGSmoke;            /*   468     4 */
	int							m_angle;              /*   472     4 */
	short unsigned int			m_usEvent;            /*   476     2 */
	bool						m_bLightSmoke;        /*   478     1 */
	bool						m_bDetonated;         /*   479     1 */
	Vector						m_vSmokeDetonate;     /*   480    12 */
	int							m_iBounceCount;       /*   492     4 */

	BOOL						m_fRegisteredSound;   /*   496     4 */  //whether or not this grenade has issued its DANGER sound to the world sound list yet.

	/* vtable has 8 entries: 
	{
	   [0]	= Spawn			(_ZN8CGrenade5SpawnEv), 
	   [4]	= Save			(_ZN8CGrenade4SaveER5CSave), 
	   [5]	= Restore		(_ZN8CGrenade7RestoreER8CRestore), 
	   [6]	= ObjectCaps	(_ZN8CGrenade10ObjectCapsEv), 
	   [14] = Killed		(_ZN8CGrenade6KilledEP9entvars_si), 
	   [15] = BloodColor	(_ZN8CGrenade10BloodColorEv), 
	   [46] = Use			(_ZN8CGrenade3UseEP11CBaseEntityS1_8USE_TYPEf), 
	   [76] = BounceSound	(_ZN8CGrenade11BounceSoundEv), 
	} */

	/* size: 500, cachelines: 8, members: 26 */
	/* sum members: 91, holes: 3, sum holes: 409 */
	/* padding: 260 */
	/* last cacheline: 52 bytes */

	/* BRAIN FART ALERT! 500 != 91 + 409(holes), diff = 0 */
};


// constant items
#define ITEM_HEALTHKIT		1
#define ITEM_ANTIDOTE		2
#define ITEM_SECURITY		3
#define ITEM_BATTERY		4

typedef enum 
{
    WEAPON_NONE = 0,
    WEAPON_P228,
    WEAPON_GLOCK,
    WEAPON_SCOUT,
    WEAPON_HEGRENADE,
    WEAPON_XM1014,
    WEAPON_C4,
    WEAPON_MAC10,
    WEAPON_AUG,
    WEAPON_SMOKEGRENADE,
    WEAPON_ELITE,
    WEAPON_FIVESEVEN,
    WEAPON_UMP45,
    WEAPON_SG550,
    WEAPON_GALIL,
    WEAPON_FAMAS,
    WEAPON_USP,
    WEAPON_GLOCK18,
    WEAPON_AWP,
    WEAPON_MP5N,
    WEAPON_M249,
    WEAPON_M3,
    WEAPON_M4A1,
    WEAPON_TMP,
    WEAPON_G3SG1,
    WEAPON_FLASHBANG,
    WEAPON_DEAGLE,
    WEAPON_SG552,
    WEAPON_AK47,
    WEAPON_KNIFE,
    WEAPON_P90,
    WEAPON_SHIELDGUN = 99,

} WeaponIdType;

#define WEAPON_ALLWEAPONS   ( ~( 1 << WEAPON_SUIT ) )
#define WEAPON_SUIT         31
#define MAX_WEAPONS         32

#define MAX_NORMAL_BATTERY	100
#define WEAPON_NOCLIP       -1

// Weapon weight factors (for auto-switching, -1 = noswitch)
// CS
#define AK47_WEIGHT         25
#define AUG_WEIGHT          25
#define AWP_WEIGHT          30
#define C4_WEIGHT           3
#define DEAGLE_WEIGHT       7
#define ELITE_WEIGHT        5
#define FAMAS_WEIGHT        75
#define FIVESEVEN_WEIGHT    5
#define FLASHBANG_WEIGHT    1
#define G3SG1_WEIGHT        20
#define GALIL_WEIGHT        25
#define GLOCK18_WEIGHT      5
#define HEGRENADE_WEIGHT    2
#define KNIFE_WEIGHT        0
#define M249_WEIGHT         25
#define M3_WEIGHT           20
#define M4A1_WEIGHT         25
#define MAC10_WEIGHT        25
#define MP5NAVY_WEIGHT      25
#define P228_WEIGHT         5
#define P90_WEIGHT          26
#define SCOUT_WEIGHT        30
#define SG550_WEIGHT        20
#define SG552_WEIGHT        25
#define SMOKEGRENADE_WEIGHT 1
#define TMP_WEIGHT          25
#define UMP45_WEIGHT        25
#define USP_WEIGHT          5
#define XM1014_WEIGHT       20

// CS
enum MaxAmmoType
{
    MAX_AMMO_BUCKSHOT   = 32,
    MAX_AMMO_9MM        = 120,
    MAX_AMMO_556NATO    = 90,
    MAX_AMMO_556NATOBOX = 200,
    MAX_AMMO_762NATO    = 90,
    MAX_AMMO_45ACP      = 100,
    MAX_AMMO_50AE       = 35,
    MAX_AMMO_338MAGNUM  = 30,
    MAX_AMMO_57MM       = 100,
    MAX_AMMO_357SIG     = 52
};

// weapon clip/carry ammo capacities
// CS
#define _9MM_MAX_CARRY          MAX_AMMO_9MM
#define BUCKSHOT_MAX_CARRY      MAX_AMMO_BUCKSHOT
#define _556NATO_MAX_CARRY      MAX_AMMO_556NATO
#define _556NATOBOX_MAX_CARRY   MAX_AMMO_556NATOBOX
#define _762NATO_MAX_CARRY      MAX_AMMO_762NATO
#define _45ACP_MAX_CARRY        MAX_AMMO_45ACP
#define _50AE_MAX_CARRY         MAX_AMMO_50AE
#define _338MAGNUM_MAX_CARRY    MAX_AMMO_338MAGNUM
#define _57MM_MAX_CARRY         MAX_AMMO_57MM
#define _357SIG_MAX_CARRY       MAX_AMMO_357SIG

#define C4_MAX_CARRY            1
#define FLASHBANG_MAX_CARRY     2
#define HEGRENADE_MAX_CARRY     1
#define SMOKEGRENADE_MAX_CARRY  1

// The default amount of ammo given at weapon spawn.
// CS
#define AK47_DEFAULT_GIVE       30
#define AUG_DEFAULT_GIVE        30
#define AWP_DEFAULT_GIVE        10
#define C4_DEFAULT_GIVE         1
#define DEAGLE_DEFAULT_GIVE     7
#define ELITE_DEFAULT_GIVE      30
#define FAMAS_DEFAULT_GIVE      25
#define FIVESEVEN_DEFAULT_GIVE  20
#define FLASHBANG_DEFAULT_GIVE  1
#define G3SG1_DEFAULT_GIVE      20
#define GALIL_DEFAULT_GIVE      35
#define GLOCK18_DEFAULT_GIVE    20
#define HEGRENADE_DEFAULT_GIVE  1
#define M249_DEFAULT_GIVE       100
#define M3_DEFAULT_GIVE         8
#define M4A1_DEFAULT_GIVE       30
#define MAC10_DEFAULT_GIVE      30
#define MP5NAVY_DEFAULT_GIVE    30
#define P228_DEFAULT_GIVE       13
#define P90_DEFAULT_GIVE        50
#define SCOUT_DEFAULT_GIVE      10
#define SG550_DEFAULT_GIVE      30
#define SG552_DEFAULT_GIVE      30
#define MAC10_DEFAULT_GIVE      30
#define SMOKEGRENADE_DEFAULT_GIVE 1
#define TMP_DEFAULT_GIVE        30
#define UMP45_DEFAULT_GIVE      25
#define USP_DEFAULT_GIVE        12
#define XM1014_DEFAULT_GIVE     7

// The amount of ammo given to a player by an ammo item.
// CS
#define AMMO_9MM_GIVE           30
#define AMMO_BUCKSHOT_GIVE      8
#define AMMO_556NATO_GIVE       30
#define AMMO_556NATOBOX_GIVE    30
#define AMMO_762NATO_GIVE       30
#define AMMO_45ACP_GIVE         12
#define AMMO_50AE_GIVE          7
#define AMMO_338MAGNUM_GIVE     10
#define AMMO_57MM_GIVE          50
#define AMMO_357SIG_GIVE        13

// The maximum amount of ammo each weapon's clip can hold.
// CS
enum ClipSizeType
{
    P228_MAX_CLIP       = 13,
    GLOCK18_MAX_CLIP    = 20,
    SCOUT_MAX_CLIP      = 10,
    XM1014_MAX_CLIP     = 7,
    MAC10_MAX_CLIP      = 30,
    AUG_MAX_CLIP        = 30,
    ELITE_MAX_CLIP      = 30,
    FIVESEVEN_MAX_CLIP  = 20,
    UMP45_MAX_CLIP      = 25,
    SG550_MAX_CLIP      = 30,
    GALIL_MAX_CLIP      = 35,
    FAMAS_MAX_CLIP      = 25,
    USP_MAX_CLIP        = 12,
    AWP_MAX_CLIP        = 10,
    MP5N_MAX_CLIP       = 30,
    M249_MAX_CLIP       = 100,
    M3_MAX_CLIP         = 8,
    M4A1_MAX_CLIP       = 30,
    TMP_MAX_CLIP        = 30,
    G3SG1_MAX_CLIP      = 20,
    DEAGLE_MAX_CLIP     = 7,
    SG552_MAX_CLIP      = 30,
    AK47_MAX_CLIP       = 30,
    P90_MAX_CLIP        = 50,
};

// CS
typedef enum 
{
    PRIMARY_WEAPON_SLOT = 1,
    PISTOL_SLOT,
    KNIFE_SLOT,
    GRENADE_SLOT,
    C4_SLOT,

} InventorySlotType;

// CS
enum WeaponState
{
    WPNSTATE_USP_SILENCED       = ( 1 << 0 ),
    WPNSTATE_GLOCK18_BURST_MODE = ( 1 << 1 ),
    WPNSTATE_M4A1_SILENCED      = ( 1 << 2 ),
    WPNSTATE_ELITE_LEFT         = ( 1 << 3 ),
    WPNSTATE_FAMAS_BURST_MODE   = ( 1 << 4 ),
    WPNSTATE_SHIELD_DRAWN       = ( 1 << 5 ),
};

//CS
enum shieldgun_e
{
    SHIELDGUN_IDLE,
    SHIELDGUN_SHOOT1,
    SHIELDGUN_SHOOT2,
    SHIELDGUN_SHOOT_EMPTY,
    SHIELDGUN_RELOAD,
    SHIELDGUN_DRAW,
    SHIELDGUN_DRAWN_IDLE,
    SHIELDGUN_UP,
    SHIELDGUN_DOWN
};

// CS
enum shieldren_e
{
    SHIELDREN_IDLE = 4,
    SHIELDREN_UP,
    SHIELDREN_DOWN
};


// CS
#define AK47_DISTANCE           8192
#define AUG_DISTANCE            8192
#define AWP_DISTANCE            8192
#define DEAGLE_DISTANCE         4096
#define ELITE_DISTANCE          8192
#define FAMAS_DISTANCE          8192
#define FIVESEVEN_DISTANCE      4096
#define G3SG1_DISTANCE          8192
#define GALIL_DISTANCE          8192
#define GLOCK18_DISTANCE        8192
#define M249_DISTANCE           8192
#define M4A1_DISTANCE           8192
#define MAC10_DISTANCE          8192
#define MP5N_DISTANCE           8192
#define P228_DISTANCE           4096
#define P90_DISTANCE            8192
#define SCOUT_DISTANCE          8192
#define SG550_DISTANCE          8192
#define SG552_DISTANCE          8192
#define TMP_DISTANCE            8192
#define UMP45_DISTANCE          8192
#define USP_DISTANCE            4096

// CS
#define AK47_DAMAGE             36
#define AUG_DAMAGE              32
#define AWP_DAMAGE              115
#define DEAGLE_DAMAGE           54
#define ELITE_DAMAGE            36
#define FAMAS_DAMAGE            30
#define FAMAS_DAMAGE_BURST      34
#define FIVESEVEN_DAMAGE        20
#define G3SG1_DAMAGE            80
#define GALIL_DAMAGE            30
#define GLOCK18_DAMAGE          25
#define M249_DAMAGE             32
#define M4A1_DAMAGE             33
#define M4A1_DAMAGE_SIL         32
#define MAC10_DAMAGE            29
#define MP5N_DAMAGE             26
#define P228_DAMAGE             32
#define P90_DAMAGE              21
#define SCOUT_DAMAGE            75
#define SG550_DAMAGE            70
#define SG552_DAMAGE            33
#define TMP_DAMAGE              20
#define UMP45_DAMAGE            30
#define USP_DAMAGE              34
#define USP_DAMAGE_SIL          30

// CS
#define AK47_PENETRATION        2
#define AUG_PENETRATION         2
#define AWP_PENETRATION         3
#define DEAGLE_PENETRATION      2
#define ELITE_PENETRATION       1
#define FAMAS_PENETRATION       2
#define FIVESEVEN_PENETRATION   1
#define G3SG1_PENETRATION       3
#define GALIL_PENETRATION       2
#define GLOCK18_PENETRATION     1
#define M249_PENETRATION        2
#define M4A1_PENETRATION        2
#define MAC10_PENETRATION       1
#define MP5N_PENETRATION        1
#define P228_PENETRATION        1
#define P90_PENETRATION         1
#define SCOUT_PENETRATION       3
#define SG550_PENETRATION       2
#define SG552_PENETRATION       2
#define TMP_PENETRATION         1
#define UMP45_PENETRATION       1
#define USP_PENETRATION         1

// CS
#define AK47_RANGE_MODIFER      0.98
#define AUG_RANGE_MODIFER       0.96
#define AWP_RANGE_MODIFER       0.99
#define DEAGLE_RANGE_MODIFER    0.81
#define ELITE_RANGE_MODIFER     0.75
#define FAMAS_RANGE_MODIFER     0.96
#define FIVESEVEN_RANGE_MODIFER 0.885
#define G3SG1_RANGE_MODIFER     0.98
#define GALIL_RANGE_MODIFER     0.98
#define GLOCK18_RANGE_MODIFER   0.75
#define M249_RANGE_MODIFER      0.97
#define M4A1_RANGE_MODIFER      0.95
#define M4A1_RANGE_MODIFER_SIL  0.97
#define MAC10_RANGE_MODIFER     0.82
#define MP5N_RANGE_MODIFER      0.84
#define P228_RANGE_MODIFER      0.80
#define P90_RANGE_MODIFER       0.885
#define SCOUT_RANGE_MODIFER     0.98
#define SG550_RANGE_MODIFER     0.98
#define SG552_RANGE_MODIFER     0.955
#define TMP_RANGE_MODIFER       0.85
#define UMP45_RANGE_MODIFER     0.82
#define USP_RANGE_MODIFER       0.79

// bullet types
typedef	enum
{
	BULLET_NONE = 0,
	BULLET_PLAYER_9MM, 
	BULLET_PLAYER_MP5, 
	BULLET_PLAYER_357, 
	BULLET_PLAYER_BUCKSHOT, 
	BULLET_PLAYER_CROWBAR,

	BULLET_MONSTER_9MM,
	BULLET_MONSTER_MP5,
	BULLET_MONSTER_12MM,

	// CS
	BULLET_PLAYER_45ACP,
	BULLET_PLAYER_338MAG,
	BULLET_PLAYER_762MM,
	BULLET_PLAYER_556MM,
	BULLET_PLAYER_50AE,
	BULLET_PLAYER_57MM,
	BULLET_PLAYER_357SIG,

} Bullet;

#define ITEM_FLAG_SELECTONEMPTY		1
#define ITEM_FLAG_NOAUTORELOAD		2
#define ITEM_FLAG_NOAUTOSWITCHEMPTY	4
#define ITEM_FLAG_LIMITINWORLD		8
#define ITEM_FLAG_EXHAUSTIBLE		16 // A player can totally exhaust their ammo supply and lose this weapon

#define WEAPON_IS_ONTARGET 0x40

typedef struct
{
	int		iSlot;
	int		iPosition;
	const char	*pszAmmo1;	// ammo 1 type
	int		iMaxAmmo1;		// max ammo 1
	const char	*pszAmmo2;	// ammo 2 type
	int		iMaxAmmo2;		// max ammo 2
	const char	*pszName;
	int		iMaxClip;
	int		iId;
	int		iFlags;
	int		iWeight;// this value used to determine this weapon's importance in autoselection.
} ItemInfo;

typedef struct
{
	const char *pszName;
	int iId;
} AmmoInfo;

// Items that the player has in their inventory that they can use
class CBasePlayerItem : public CBaseAnimating
{
public:
    virtual void SetObjectCollisionBox( void );

    virtual int Save( CSave &save );
    virtual int Restore( CRestore &restore );

    virtual int AddToPlayer( CBasePlayer *pPlayer );                        // Return TRUE if the item you want the item added to the player inventory.
    virtual int AddDuplicate( CBasePlayerItem *pItem ) { return FALSE; }    // Return TRUE if you want your duplicate removed from world.

    void EXPORT DestroyItem( void );
    void EXPORT DefaultTouch( CBaseEntity *pOther );                        // Default weapon touch.
    void EXPORT FallThink( void );                                          // When an item is first spawned, this think is run to determine when the object has hit the ground.
    void EXPORT Materialize( void );                                        // Make a weapon visible and tangible.
    void EXPORT AttemptToMaterialize( void );                               // The weapon desires to become visible and tangible, if the game rules allow for it.

    virtual CBaseEntity* Respawn( void );                                   // Copy a weapon.

    void FallInit( void );
    void CheckRespawn( void );

    virtual int GetItemInfo( ItemInfo *p ) { return 0; };                    // Returns 0 if struct not filled out.

    virtual BOOL CanDeploy( void ) { return TRUE; }
    virtual BOOL CanDrop( void ) { return TRUE; }
    virtual BOOL Deploy( void ) { return TRUE; }
    virtual BOOL IsWeapon( void ) { return FALSE; }
    virtual BOOL CanHolster( void ) { return TRUE; };                        // Can this weapon be put away right now?

    virtual void Holster( int skiplocal = 0 );
    virtual void UpdateItemInfo( void ) { return; }

    virtual void ItemPreFrame( void )   { return; }                          // Called each frame by the player PreThink.
    virtual void ItemPostFrame( void )  { return; }                          // Called each frame by the player PostThink.

    virtual void Drop( void );
    virtual void Kill( void );
    virtual void AttachToPlayer( CBasePlayer *pPlayer );

    virtual int PrimaryAmmoIndex( void ) { return -1; }
    virtual int SecondaryAmmoIndex( void ) { return -1; }

    virtual int UpdateClientData( CBasePlayer *pPlayer ) { return 0; }

    virtual CBasePlayerItem* GetWeaponPtr( void ) { return NULL; };
    virtual float GetMaxSpeed( void ) { return 260.0; };

    virtual int iItemSlot( void ) { return 0; }

    int         iItemPosition( void )   { return ItemInfoArray[ m_iId ].iPosition;  }
    const char *pszAmmo1( void )        { return ItemInfoArray[ m_iId ].pszAmmo1;   }
    int         iMaxAmmo1( void )       { return ItemInfoArray[ m_iId ].iMaxAmmo1;  }
    const char *pszAmmo2( void )        { return ItemInfoArray[ m_iId ].pszAmmo2;   }
    int         iMaxAmmo2( void )       { return ItemInfoArray[ m_iId ].iMaxAmmo2;  }
    const char *pszName( void )         { return ItemInfoArray[ m_iId ].pszName;    }
    int         iMaxClip( void )        { return ItemInfoArray[ m_iId ].iMaxClip;   }
    int         iWeight( void )         { return ItemInfoArray[ m_iId ].iWeight;    }
    int         iFlags( void )          { return ItemInfoArray[ m_iId ].iFlags;     }

    static TYPEDESCRIPTION m_SaveData[];

	static ItemInfo ItemInfoArray[ MAX_WEAPONS ];
	static AmmoInfo AmmoInfoArray[ MAX_AMMO_SLOTS ];

	CBasePlayer	    *m_pPlayer;
	CBasePlayerItem *m_pNext;
	int		         m_iId;

    /* vtable has 25 entries:
    {
       [8]  = SetObjectCollisionBox(_ZN15CBasePlayerItem21SetObjectCollisionBoxEv),
       [4]  = Save(_ZN15CBasePlayerItem4SaveER5CSave),
       [5]  = Restore(_ZN15CBasePlayerItem7RestoreER8CRestore),
       [59] = AddToPlayer(_ZN15CBasePlayerItem11AddToPlayerEP11CBasePlayer),
       [60] = AddDuplicate(_ZN15CBasePlayerItem12AddDuplicateEPS_),
       [48] = Respawn(_ZN15CBasePlayerItem7RespawnEv),
       [61] = GetItemInfo(_ZN15CBasePlayerItem11GetItemInfoEP8ItemInfo),
       [62] = CanDeploy(_ZN15CBasePlayerItem9CanDeployEv),
       [63] = CanDrop(_ZN15CBasePlayerItem7CanDropEv),
       [64] = Deploy(_ZN15CBasePlayerItem6DeployEv),
       [65] = IsWeapon(_ZN15CBasePlayerItem8IsWeaponEv),
       [66] = CanHolster(_ZN15CBasePlayerItem10CanHolsterEv),
       [67] = Holster(_ZN15CBasePlayerItem7HolsterEi),
       [68] = UpdateItemInfo(_ZN15CBasePlayerItem14UpdateItemInfoEv),
       [69] = ItemPreFrame(_ZN15CBasePlayerItem12ItemPreFrameEv),
       [70] = ItemPostFrame(_ZN15CBasePlayerItem13ItemPostFrameEv),
       [71] = Drop(_ZN15CBasePlayerItem4DropEv),
       [72] = Kill(_ZN15CBasePlayerItem4KillEv),
       [73] = AttachToPlayer(_ZN15CBasePlayerItem14AttachToPlayerEP11CBasePlayer),
       [74] = PrimaryAmmoIndex(_ZN15CBasePlayerItem16PrimaryAmmoIndexEv),
       [75] = SecondaryAmmoIndex(_ZN15CBasePlayerItem18SecondaryAmmoIndexEv),
       [76] = UpdateClientData(_ZN15CBasePlayerItem16UpdateClientDataEP11CBasePlayer),
       [77] = GetWeaponPtr(_ZN15CBasePlayerItem12GetWeaponPtrEv),
       [78] = GetMaxSpeed(_ZN15CBasePlayerItem11GetMaxSpeedEv),
       [79] = iItemSlot(_ZN15CBasePlayerItem9iItemSlotEv),
    } */
    /* size: 192, cachelines: 3, members: 7 */
    /* BRAIN FART ALERT! 192 != 60 + 0(holes), diff = 132 */
};


// inventory items that 
class CBasePlayerWeapon : public CBasePlayerItem
{
public:
    virtual int Save( CSave &save );
    virtual int Restore( CRestore &restore );

    virtual int AddToPlayer( CBasePlayer *pPlayer );
    virtual int AddDuplicate( CBasePlayerItem *pItem );

    virtual int ExtractAmmo( CBasePlayerWeapon *pWeapon );                  // Return TRUE if you can add ammo to yourself when picked up.
    virtual int ExtractClipAmmo( CBasePlayerWeapon *pWeapon );              // Return TRUE if you can add ammo to yourself when picked up.

    virtual int AddWeapon( void ) { ExtractAmmo( this ); return TRUE; }     // Return TRUE if you want to add yourself to the player.

    BOOL AddPrimaryAmmo( int iCount, char *szName, int iMaxClip, int iMaxCarry );
    BOOL AddSecondaryAmmo( int iCount, char *szName, int iMaxCarry );

    virtual void UpdateItemInfo( void ) {};                                 // Updates HUD state.

    virtual BOOL PlayEmptySound( void );
    virtual void ResetEmptySound( void );

    virtual void SendWeaponAnim( int iAnim, int skiplocal = 1 );            // Skiplocal is 1 if client is predicting weapon animations.

    virtual BOOL CanDeploy( void );
    virtual BOOL IsUseable( void );
    BOOL DefaultDeploy( char *szViewModel, char *szWeaponModel, int iAnim, char *szAnimExt, int skiplocal = 0 );
    BOOL DefaultReload( int iClipSize, int iAnim, float fDelay );

    virtual void ItemPostFrame( void );                                     // Called each frame by the player PostThink.
    virtual void PrimaryAttack( void ) { return; }                          // Do "+ATTACK".
    virtual void SecondaryAttack( void ) { return; }                        // Do "+ATTACK2".
    virtual void Reload( void ) { return; }                                 // Do "+RELOAD".
    virtual void WeaponIdle( void ) { return; }                             // Called when no buttons pressed.
    virtual int UpdateClientData( CBasePlayer *pPlayer );                   // Sends hud info to client dll, if things have changed.
    virtual void RetireWeapon( void );
    virtual BOOL ShouldWeaponIdle( void ) { return FALSE; }
    virtual void Holster( int skiplocal = 0 );
    virtual BOOL UseDecrement( void ) { return FALSE; }

    virtual BOOL IsWeapon( void ) { return TRUE; }

    virtual int PrimaryAmmoIndex( void );
    virtual int SecondaryAmmoIndex( void );

    virtual CBasePlayerItem* GetWeaponPtr( void ) { return ( CBasePlayerItem* )this; };

    void FireRemaining( int &shotsFired, float &shootTime, BOOL bIsGlock );
    void KickBack( float up_base, float lateral_base, float up_modifier, float lateral_modifier, float up_max, float lateral_max, int direction_change );
    void EjectBrassLate( void );
    void MakeBeam( void );
    void BeamUpdate( void );
    void ReloadSound( void );
    float GetNextAttackDelay( float delay );
    bool HasSecondaryAttack( void );
    BOOL IsPistol( void ) { return FALSE; }

    void SetPlayerShieldAnim( void );
    void ResetPlayerShieldAnim( void );
    bool ShieldSecondaryFire( int iUpAnim, int iDownAnim );

    static	TYPEDESCRIPTION m_SaveData[];

    int                         m_iPlayEmptySound;          /*   192     4 */ 
    int                         m_fFireOnEmpty;             /*   196     4 */ // True when the gun is empty and the player is still holding down the

    float                       m_flNextPrimaryAttack;      /*   200     4 */ // Soonest time ItemPostFrame will call PrimaryAttack.
    float                       m_flNextSecondaryAttack;    /*   204     4 */ // Soonest time ItemPostFrame will call SecondaryAttack.
    float                       m_flTimeWeaponIdle;         /*   208     4 */ // Soonest time ItemPostFrame will call WeaponIdle.

    int                         m_iPrimaryAmmoType;         /*   212     4 */ // "primary" ammo index into players m_rgAmmo[]
    int                         m_iSecondaryAmmoType;       /*   216     4 */ // "secondary" ammo index into players m_rgAmmo[]

    int                         m_iClip;                    /*   220     4 */ // Number of shots left in the primary weapon clip, -1 it not used.
    int                         m_iClientClip;              /*   224     4 */ // The last version of m_iClip sent to hud dll.
    int                         m_iClientWeaponState;       /*   228     4 */ // The last version of the weapon state sent to hud dll (is current weapon, is on target).

    int                         m_fInReload;                /*   232     4 */ // Are we in the middle of a reload.
    int                         m_fInSpecialReload;         /*   236     4 */ // Are we in the middle of a reload for the shotguns.

    int                         m_iDefaultAmmo;             /*   240     4 */ // How much ammo you get when you pick up this weapon as placed by a level designer.
    int                         m_iShellId;                 /*   244     4 */
    float                       m_fMaxSpeed;                /*   248     4 */

    bool                        m_bDelayFire;               /*   252     1 */
    int                         m_iDirection;               /*   256     4 */

    bool                        m_bSecondarySilencerOn;     /*   260     1 */

    float                       m_flAccuracy;               /*   264     4 */
    float                       m_flLastFire;               /*   268     4 */
    int                         m_iShotsFired;              /*   272     4 */
    Vector                      m_vVecAiming;               /*   276    12 */

    string_t                    model_name;                 /*   288     4 */

    float                       m_flGlock18Shoot;           /*   292     4 */
    int                         m_iGlock18ShotsFired;       /*   296     4 */
    float                       m_flFamasShoot;             /*   300     4 */
    int                         m_iFamasShotsFired;         /*   304     4 */
    float                       m_fBurstSpread;             /*   308     4 */

    int                         m_iWeaponState;             /*   312     4 */
    float                       m_flNextReload;             /*   316     4 */
    float                       m_flDecreaseShotsFired;     /*   320     4 */

    short unsigned int          m_usFireGlock18;            /*   324     2 */
    short unsigned int          m_usFireFamas;              /*   326     2 */

    float                       m_flPrevPrimaryAttack;      /*   328     4 */
    float                       m_flLastFireTime;           /*   332     4 */

    /* vtable has 27 entries:
    {
       [4] = Save(_ZN17CBasePlayerWeapon4SaveER5CSave),
       [5] = Restore(_ZN17CBasePlayerWeapon7RestoreER8CRestore),
       [59] = AddToPlayer(_ZN17CBasePlayerWeapon11AddToPlayerEP11CBasePlayer),
       [60] = AddDuplicate(_ZN17CBasePlayerWeapon12AddDuplicateEP15CBasePlayerItem),
       [80] = ExtractAmmo(_ZN17CBasePlayerWeapon11ExtractAmmoEPS_),
       [81] = ExtractClipAmmo(_ZN17CBasePlayerWeapon15ExtractClipAmmoEPS_),
       [82] = AddWeapon(_ZN17CBasePlayerWeapon9AddWeaponEv),
       [68] = UpdateItemInfo(_ZN17CBasePlayerWeapon14UpdateItemInfoEv),
       [83] = PlayEmptySound(_ZN17CBasePlayerWeapon14PlayEmptySoundEv),
       [84] = ResetEmptySound(_ZN17CBasePlayerWeapon15ResetEmptySoundEv),
       [85] = SendWeaponAnim(_ZN17CBasePlayerWeapon14SendWeaponAnimEii),
       [62] = CanDeploy(_ZN17CBasePlayerWeapon9CanDeployEv),
       [86] = IsUseable(_ZN17CBasePlayerWeapon9IsUseableEv),
       [70] = ItemPostFrame(_ZN17CBasePlayerWeapon13ItemPostFrameEv),
       [87] = PrimaryAttack(_ZN17CBasePlayerWeapon13PrimaryAttackEv),
       [88] = SecondaryAttack(_ZN17CBasePlayerWeapon15SecondaryAttackEv),
       [89] = Reload(_ZN17CBasePlayerWeapon6ReloadEv),
       [90] = WeaponIdle(_ZN17CBasePlayerWeapon10WeaponIdleEv),
       [76] = UpdateClientData(_ZN17CBasePlayerWeapon16UpdateClientDataEP11CBasePlayer),
       [91] = RetireWeapon(_ZN17CBasePlayerWeapon12RetireWeaponEv),
       [92] = ShouldWeaponIdle(_ZN17CBasePlayerWeapon16ShouldWeaponIdleEv),
       [67] = Holster(_ZN17CBasePlayerWeapon7HolsterEi),
       [93] = UseDecrement(_ZN17CBasePlayerWeapon12UseDecrementEv),
       [65] = IsWeapon(_ZN17CBasePlayerWeapon8IsWeaponEv),
       [74] = PrimaryAmmoIndex(_ZN17CBasePlayerWeapon16PrimaryAmmoIndexEv),
       [75] = SecondaryAmmoIndex(_ZN17CBasePlayerWeapon18SecondaryAmmoIndexEv),
       [77] = GetWeaponPtr(_ZN17CBasePlayerWeapon12GetWeaponPtrEv),
    } */
    /* size: 336, cachelines: 6, members: 37 */
    /* sum members: 330, holes: 3, sum holes: 86 */
    /* last cacheline: 16 bytes */
    /* BRAIN FART ALERT! 336 != 330 + 86(holes), diff = -80 */
};


class CBasePlayerAmmo : public CBaseEntity
{
public:
	virtual void Spawn( void );
	void EXPORT DefaultTouch( CBaseEntity *pOther ); // default weapon touch
	virtual BOOL AddAmmo( CBaseEntity *pOther ) { return TRUE; };

	CBaseEntity* Respawn( void );
	void EXPORT Materialize( void );
};


extern DLL_GLOBAL	short	g_sModelIndexLaser;// holds the index for the laser beam
extern DLL_GLOBAL	const char *g_pModelNameLaser;

extern DLL_GLOBAL	short	g_sModelIndexLaserDot;// holds the index for the laser beam dot
extern DLL_GLOBAL	short	g_sModelIndexFireball;// holds the index for the fireball
extern DLL_GLOBAL	short	g_sModelIndexSmoke;// holds the index for the smoke cloud
extern DLL_GLOBAL	short	g_sModelIndexWExplosion;// holds the index for the underwater explosion
extern DLL_GLOBAL	short	g_sModelIndexBubbles;// holds the index for the bubbles model
extern DLL_GLOBAL	short	g_sModelIndexBloodDrop;// holds the sprite index for blood drops
extern DLL_GLOBAL	short	g_sModelIndexBloodSpray;// holds the sprite index for blood spray (bigger)

// CS
extern DLL_GLOBAL	short	g_sModelIndexFireball2;
extern DLL_GLOBAL	short	g_sModelIndexFireball3;
extern DLL_GLOBAL	short	g_sModelIndexFireball4;
extern DLL_GLOBAL   short   g_sModelIndexC4Glow;
extern DLL_GLOBAL   short   g_sModelIndexRadio;
extern DLL_GLOBAL   short   g_sModelIndexSmokePuff;
extern DLL_GLOBAL   short   g_sModelIndexCTGhost;
extern DLL_GLOBAL   short   g_sModelIndexTGhost;

extern void ClearMultiDamage(void);
extern void ApplyMultiDamage(entvars_t* pevInflictor, entvars_t* pevAttacker );
extern void AddMultiDamage( entvars_t *pevInflictor, CBaseEntity *pEntity, float flDamage, int bitsDamageType);

extern void DecalGunshot( TraceResult *pTrace, int iBulletType, bool ClientOnly, entvars_t *pShooter, bool bHitMetal );
extern void SpawnBlood(Vector vecSpot, int bloodColor, float flDamage);
extern int DamageDecal( CBaseEntity *pEntity, int bitsDamageType );
extern void RadiusDamage( Vector vecSrc, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, float flRadius, int iClassIgnore, int bitsDamageType );

typedef struct 
{
	CBaseEntity		*pEntity;
	float			amount;
	int				type;
} MULTIDAMAGE;

extern MULTIDAMAGE gMultiDamage;


#define LOUD_GUN_VOLUME			1000
#define NORMAL_GUN_VOLUME		600
#define QUIET_GUN_VOLUME		200

#define	BRIGHT_GUN_FLASH		512
#define NORMAL_GUN_FLASH		256
#define	DIM_GUN_FLASH			128

#define BIG_EXPLOSION_VOLUME	2048
#define NORMAL_EXPLOSION_VOLUME	1024
#define SMALL_EXPLOSION_VOLUME	512

#define	WEAPON_ACTIVITY_VOLUME	64

#define VECTOR_CONE_1DEGREES	Vector( 0.00873, 0.00873, 0.00873 )
#define VECTOR_CONE_2DEGREES	Vector( 0.01745, 0.01745, 0.01745 )
#define VECTOR_CONE_3DEGREES	Vector( 0.02618, 0.02618, 0.02618 )
#define VECTOR_CONE_4DEGREES	Vector( 0.03490, 0.03490, 0.03490 )
#define VECTOR_CONE_5DEGREES	Vector( 0.04362, 0.04362, 0.04362 )
#define VECTOR_CONE_6DEGREES	Vector( 0.05234, 0.05234, 0.05234 )
#define VECTOR_CONE_7DEGREES	Vector( 0.06105, 0.06105, 0.06105 )
#define VECTOR_CONE_8DEGREES	Vector( 0.06976, 0.06976, 0.06976 )
#define VECTOR_CONE_9DEGREES	Vector( 0.07846, 0.07846, 0.07846 )
#define VECTOR_CONE_10DEGREES	Vector( 0.08716, 0.08716, 0.08716 )
#define VECTOR_CONE_15DEGREES	Vector( 0.13053, 0.13053, 0.13053 )
#define VECTOR_CONE_20DEGREES	Vector( 0.17365, 0.17365, 0.17365 )


class CAK47 : public CBasePlayerWeapon 
{
    public :

	    void Spawn( void );
	    void Precache( void );
	    int iItemSlot( void );
	    int GetItemInfo( ItemInfo *p );

	    void PrimaryAttack( void );
	    void SecondaryAttack( void );
	    void AK47Fire( float flSpread, float flCycleTime, BOOL fUseAutoAim );
	    BOOL Deploy( void );
	    void Reload( void );
	    void WeaponIdle( void );
	    float GetMaxSpeed( void );
	    BOOL UseDecrement( void );

    public :

        int                 m_iShell;       /*   336     4 */
        int                 iShellOn;       /*   340     4 */

    private:

        short unsigned int  m_usFireAK47;   /*   344     2 */

    /* vtable has 11 entries: 
    {
	   [0]  = Spawn
	   [1]  = Precache
	   [79] = iItemSlot
	   [61] = GetItemInfo
	   [87] = PrimaryAttack
	   [88] = SecondaryAttack
	   [64] = Deploy
	   [89] = Reload
	   [90] = WeaponIdle
	   [78] = GetMaxSpeed
	   [93] = UseDecrement
	} */
	/* size: 348, cachelines: 6, members: 4      */
	/* sum members: 10, holes: 1, sum holes: 336 */
	/* padding: 2                                */
	/* last cacheline: 28 bytes                  */
};

class CAUG : public CBasePlayerWeapon 
{
    public:

        void Spawn( void );
        void Precache( void );
        int iItemSlot( void );
        int GetItemInfo( ItemInfo *p );

        void PrimaryAttack( void );
        void SecondaryAttack( void );
        void AUGFire( float flSpread, float flCycleTime, BOOL fUseAutoAim );
        BOOL Deploy( void );
        void Reload( void );
        void WeaponIdle( void );
        float GetMaxSpeed( void );
        BOOL UseDecrement( void );

    public:

        int                        m_iShell;             /*   336     4 */
        int                        iShellOn;             /*   340     4 */

    private:

        short unsigned int         m_usFireAug;          /*   344     2 */

    /* vtable has 11 entries: 
    {
	   [0]  = Spawn
	   [1]  = Precache
	   [79] = iItemSlot
	   [61] = GetItemInfo
	   [87] = PrimaryAttack
	   [88] = SecondaryAttack
	   [64] = Deploy
	   [89] = Reload
	   [90] = WeaponIdle
	   [78] = GetMaxSpeed
	   [93] = UseDecrement
	} */
	/* size: 348, cachelines: 6, members: 4      */
	/* sum members: 10, holes: 1, sum holes: 336 */
	/* padding: 2                                */
	/* last cacheline: 28 bytes                  */
};

class CAWP : public CBasePlayerWeapon 
{
    public:

        void Spawn( void );
        void Precache( void );
        int iItemSlot( void );
        int GetItemInfo( ItemInfo *p );

        void PrimaryAttack( void );
        void SecondaryAttack( void );
        void AWPFire( float flSpread, float flCycleTime, BOOL fUseAutoAim );
        BOOL Deploy( void );
        void Reload( void );
        void WeaponIdle( void );
        float GetMaxSpeed( void );
        BOOL UseDecrement( void );

    public:

        int                     m_iShell;       /*   336     4 */

    private:
            
        short unsigned int      m_usFireAWP;    /*   340     2 */

    /* vtable has 11 entries: 
    {
        [0]  = Spawn
        [1]  = Precache
        [79] = iItemSlot
        [61] = GetItemInfo
        [87] = PrimaryAttack
        [88] = SecondaryAttack
        [64] = Deploy
        [89] = Reload
        [90] = WeaponIdle
        [78] = GetMaxSpeed
        [93] = UseDecrement
    } */
    /* size: 344, cachelines: 6, members: 3     */
    /* sum members: 6, holes: 1, sum holes: 336 */
    /* padding: 2                               */
    /* last cacheline: 24 bytes                 */
};

class CC4 : public CBasePlayerWeapon 
{
    public:

        void Spawn( void );
        void Precache( void );
        int iItemSlot( void );
        int GetItemInfo( ItemInfo *p );

        void PrimaryAttack( void );
        BOOL Deploy( void );
        void Holster( int skiplocal );
        void WeaponIdle( void );
        float GetMaxSpeed( void );
        BOOL UseDecrement( void );

        void KeyValue( KeyValueData *pkvd );
        void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

    public:

        bool    m_bStartedArming;       /*   336     1 */
        bool    m_bBombPlacedAnimation; /*   337     1 */
        float   m_fArmedTime;           /*   340     4 */

    private:

        bool    m_bHasShield;           /*   344     1 */

    /* vtable has 12 entries: 
    {
        [0]  = Spawn
        [1]  = Precache
        [79] = iItemSlot
        [61] = GetItemInfo
        [87] = PrimaryAttack
        [64] = Deploy
        [67] = Holster
        [90] = WeaponIdle
        [78] = GetMaxSpeed
        [93] = UseDecrement
        [3]  = KeyValue
        [46] = Use
    } */
    /* size: 348, cachelines: 6, members: 5     */
    /* sum members: 7, holes: 2, sum holes: 338 */
    /* padding: 3                               */
    /* last cacheline: 28 bytes                 */
};

class CDEAGLE : public CBasePlayerWeapon 
{
    public:

        void Spawn( void );
        void Precache( void );
        int iItemSlot( void );
        int GetItemInfo( ItemInfo *p );

        void PrimaryAttack( void );
        void SecondaryAttack( void );
        void DEAGLEFire( float flSpread, float flCycleTime, BOOL fUseAutoAim );
        BOOL Deploy( void );
        void Reload( void );
        void WeaponIdle( void );

        void MakeBeam( void );   /* unused */
        void BeamUpdate( void ); /* unused */

        float GetMaxSpeed( void );
        BOOL UseDecrement( void );
        BOOL IsPistol( void );

    public:

        int                 m_iShell;             /*   336     4 */

    private:

        short unsigned int  m_usFireDeagle;       /*   340     2 */

    /* vtable has 12 entries: 
    {
       [0]  = Spawn
       [1]  = Precache
       [79] = iItemSlot
       [61] = GetItemInfo
       [87] = PrimaryAttack
       [88] = SecondaryAttack
       [64] = Deploy
       [89] = Reload
       [90] = WeaponIdle
       [78] = GetMaxSpeed
       [93] = UseDecrement
       [94] = IsPistol
    } */
    /* size: 344, cachelines: 6, members: 3     */
    /* sum members: 6, holes: 1, sum holes: 336 */
    /* padding: 2                               */
    /* last cacheline: 24 bytes                 */
};

class CELITE : public CBasePlayerWeapon 
{
    public:

        void Spawn( void );
        void Precache( void );
        int iItemSlot( void );
        int GetItemInfo( ItemInfo *p );

        void PrimaryAttack( void );
        void ELITEFire( float flSpread, float flCycleTime, BOOL fUseAutoAim );
        BOOL Deploy( void );
        void Reload( void );
        void WeaponIdle( void );
        float GetMaxSpeed( void );
        BOOL UseDecrement( void );
        BOOL IsPistol( void );

    public:

        int                     m_iShell;             /*   336     4 */

    private:

        short unsigned int      m_usFireELITE_LEFT;   /*   340     2 */
        short unsigned int      m_usFireELITE_RIGHT;  /*   342     2 */

    /* vtable has 11 entries: 
    {
        [0]  = Spawn
        [1]  = Precache
        [79] = iItemSlot
        [61] = GetItemInfo
        [87] = PrimaryAttack
        [64] = Deploy
        [89] = Reload
        [90] = WeaponIdle
        [78] = GetMaxSpeed
        [93] = UseDecrement
        [94] = IsPistol
    } */
    /* size: 344, cachelines: 6, members: 4     */
    /* sum members: 8, holes: 1, sum holes: 336 */
    /* last cacheline: 24 bytes                 */
};

class CFamas : public CBasePlayerWeapon 
{
    public:

        void Spawn( void );
        void Precache( void );
        int iItemSlot( void );
        int GetItemInfo( ItemInfo *p );

        void PrimaryAttack( void );
        void SecondaryAttack( void );
        void FamasFire( float flSpread, float flCycleTime, BOOL fUseAutoAim, BOOL FireBurst );
        BOOL Deploy( void );
        void Reload( void );
        void WeaponIdle( void );
        float GetMaxSpeed( void );
        BOOL UseDecrement( void );

    public:

        int     m_iShell;             /*   336     4 */
        int     iShellOn;             /*   340     4 */

    /* vtable has 11 entries: 
    {
        [0]  = Spawn
        [1]  = Precache
        [79] = iItemSlot
        [61] = GetItemInfo
        [87] = PrimaryAttack
        [88] = SecondaryAttack
        [64] = Deploy
        [89] = Reload
        [90] = WeaponIdle
        [78] = GetMaxSpeed
        [93] = UseDecrement
    } */
    /* size: 344, cachelines: 6, members: 3     */
    /* sum members: 8, holes: 1, sum holes: 336 */
    /* last cacheline: 24 bytes                 */
};

class CFiveSeven : public CBasePlayerWeapon 
{
    public:

        void Spawn( void );
        void Precache( void );
        int iItemSlot( void );
        int GetItemInfo( ItemInfo *p );

        void PrimaryAttack( void );
        void SecondaryAttack( void );
        void FiveSevenFire( float flSpread, float flCycleTime, BOOL fUseAutoAim );
        BOOL Deploy( void );
        void Reload( void );
        void WeaponIdle( void );

        void MakeBeam( void );   /* unused */
        void BeamUpdate( void ); /* unused */

        float GetMaxSpeed( void );
        BOOL UseDecrement( void );
        BOOL IsPistol( void );

    public:

        int                        m_iShell;             /*   336     4 */

    private:

        short unsigned int         m_usFireFiveSeven;    /*   340     2 */

    /* vtable has 12 entries: 
    {
       [0]  = Spawn
       [1]  = Precache
       [79] = iItemSlot
       [61] = GetItemInfo
       [87] = PrimaryAttack
       [88] = SecondaryAttack
       [64] = Deploy
       [89] = Reload
       [90] = WeaponIdle
       [78] = GetMaxSpeed
       [93] = UseDecrement
       [94] = IsPistol
    } */
    /* size: 344, cachelines: 6, members: 3     */
    /* sum members: 6, holes: 1, sum holes: 336 */
    /* padding: 2                               */
    /* last cacheline: 24 bytes                 */
};

class CFlashbang : public CBasePlayerWeapon 
{
    public:

        void Spawn( void );
        void Precache( void );
        int iItemSlot( void );
        int GetItemInfo( ItemInfo *p );

        void PrimaryAttack( void );
        void SecondaryAttack( void );
        BOOL Deploy( void );
        void Holster( int skiplocal );
        void WeaponIdle( void );
        float GetMaxSpeed( void );
        BOOL CanDeploy( void );
        BOOL CanDrop( void );
        BOOL UseDecrement( void );
        BOOL IsPistol( void );
    
        bool ShieldSecondaryFire( int up_anim, int down_anim );
        void SetPlayerShieldAnim( void );
        void ResetPlayerShieldAnim( void );

    /* vtable has 14 entries: 
    {
        [0]  = Spawn
        [1]  = Precache
        [79] = iItemSlot
        [61] = GetItemInfo
        [87] = PrimaryAttack
        [88] = SecondaryAttack
        [64] = Deploy
        [67] = Holster
        [90] = WeaponIdle
        [78] = GetMaxSpeed
        [63] = CanDrop
        [62] = CanDeploy
        [93] = UseDecrement
        [94] = IsPistol
    } */
    /* size: 336, cachelines: 6, members: 1 */
    /* padding: 336                         */
    /* last cacheline: 16 bytes             */
};

class CG3SG1 : public CBasePlayerWeapon 
{
    public:

        void Spawn( void );
        void Precache( void );
        int iItemSlot( void );
        int GetItemInfo( ItemInfo *p );

        void PrimaryAttack( void );
        void SecondaryAttack( void );
        void G3SG1Fire( float flSpread, float flCycleTime, BOOL fUseAutoAim );
        BOOL Deploy( void );
        void Reload( void );
        void WeaponIdle( void );
        float GetMaxSpeed( void );
        BOOL UseDecrement( void );

    public:

        int                        m_iShell;             /*   336     4 */

    private:

        short unsigned int         m_usFireG3SG1;        /*   340     2 */

    /* vtable has 11 entries: 
    {
        [0]  = Spawn
        [1]  = Precache
        [79] = iItemSlot
        [61] = GetItemInfo
        [87] = PrimaryAttack
        [88] = SecondaryAttack
        [64] = Deploy
        [89] = Reload
        [90] = WeaponIdle
        [78] = GetMaxSpeed
        [93] = UseDecrement
    } */
    /* size: 344, cachelines: 6, members: 3     */
    /* sum members: 6, holes: 1, sum holes: 336 */
    /* padding: 2                               */
    /* last cacheline: 24 bytes                 */
};

class CGalil : public CBasePlayerWeapon 
{
    public:

        void Spawn( void );
        void Precache( void );
        int iItemSlot( void );
        int GetItemInfo( ItemInfo *p );

        void PrimaryAttack( void );
        void SecondaryAttack( void );
        void GalilFire( float flSpread, float flCycleTime, BOOL fUseAutoAim );
        BOOL Deploy( void );
        void Reload( void );
        void WeaponIdle( void );
        float GetMaxSpeed( void );
        BOOL UseDecrement( void );

    public:

        int                        m_iShell;             /*   336     4 */
        int                        iShellOn;             /*   340     4 */

    private:

        short unsigned int         m_usFireGalil;        /*   344     2 */


    /* vtable has 11 entries: 
    {
        [0]  = Spawn
        [1]  = Precache
        [79] = iItemSlot
        [61] = GetItemInfo
        [87] = PrimaryAttack
        [88] = SecondaryAttack
        [64] = Deploy
        [89] = Reload
        [90] = WeaponIdle
        [78] = GetMaxSpeed
        [93] = UseDecrement
    } */
    /* size: 348, cachelines: 6, members: 4      */
    /* sum members: 10, holes: 1, sum holes: 336 */
    /* padding: 2                                */
    /* last cacheline: 28 bytes                  */
};

class CGLOCK18 : public CBasePlayerWeapon 
{
    public:

        void Spawn( void );
        void Precache( void );
        int iItemSlot( void );
        int GetItemInfo( ItemInfo *p );

        void PrimaryAttack( void );
        void SecondaryAttack( void );
        void GLOCK18Fire( float flSpread, float flCycleTime, BOOL fUseBurstMode );
        BOOL Deploy( void );
        void Reload( void );
        void WeaponIdle( void );
        float GetMaxSpeed( void );
        BOOL UseDecrement( void );
        BOOL IsPistol( void );

    public:

        int                        m_iShell;             /*   336     4 */
        bool                       m_bBurstFire;         /*   340     1 */

    /* vtable has 12 entries: 
    {
       [0]  = Spawn
       [1]  = Precache
       [79] = iItemSlot
       [61] = GetItemInfo
       [87] = PrimaryAttack
       [88] = SecondaryAttack
       [64] = Deploy
       [89] = Reload
       [90] = WeaponIdle
       [78] = GetMaxSpeed
       [93] = UseDecrement
       [94] = IsPistol
    } */
    /* size: 344, cachelines: 6, members: 3     */
    /* sum members: 5, holes: 1, sum holes: 336 */
    /* padding: 3                               */
    /* last cacheline: 24 bytes                 */
};

class CHEGrenade : public CBasePlayerWeapon 
{
    public:

        void Spawn( void );
        void Precache( void );
        int iItemSlot( void );
        int GetItemInfo( ItemInfo *p );

        void PrimaryAttack( void );
        void SecondaryAttack( void );
        BOOL Deploy( void );
        void Holster( int skiplocal );
        void WeaponIdle( void );
        float GetMaxSpeed( void );
        BOOL CanDeploy( void );
        BOOL CanDrop( void );
        BOOL UseDecrement( void );

        bool ShieldSecondaryFire( int up_anim, int down_anim );
        void SetPlayerShieldAnim( void );
        void ResetPlayerShieldAnim( void );

    public:

        short unsigned int         m_usCreateExplosion;  /*   336     2 */

    /* vtable has 13 entries: 
    {
       [0]  = Spawn
       [1]  = Precache
       [79] = iItemSlot
       [61] = GetItemInfo
       [87] = PrimaryAttack
       [88] = SecondaryAttack
       [64] = Deploy
       [67] = Holster
       [90] = WeaponIdle
       [78] = GetMaxSpeed
       [63] = CanDrop
       [62] = CanDeploy
       [93] = UseDecrement
    } */
    /* size: 340, cachelines: 6, members: 2     */
    /* sum members: 2, holes: 1, sum holes: 336 */
    /* padding: 2                               */
    /* last cacheline: 20 bytes                 */
};

class CKnife : public CBasePlayerWeapon 
{
    public:

        void Spawn( void );
        void Precache( void );
        int GetItemInfo( ItemInfo *p );
        int iItemSlot( void );

        void EXPORT SwingAgain(  void );
        void EXPORT Smack( void );

        void PrimaryAttack( void );
        void SecondaryAttack( void );
        void WeaponAnimation( int iAnimation );
        int Swing( int fFirst );
        int Stab( int fFirst );
        BOOL Deploy( void );
        void Holster( int skiplocal );
        float GetMaxSpeed( void );
        void WeaponIdle( void );
        BOOL CanDrop( void );
        BOOL UseDecrement( void );

        bool ShieldSecondaryFire( int up_anim, int down_anim );
        void SetPlayerShieldAnim( void );
        void ResetPlayerShieldAnim( void );

    public:

        TraceResult                m_trHit;              /*   336    56 */
        short unsigned int         m_usKnife;            /*   392     2 */

    /* vtable has 12 entries: 
    {
       [0]  = Spawn
       [1]  = Precache
       [79] = iItemSlot
       [61] = GetItemInfo
       [87] = PrimaryAttack
       [88] = SecondaryAttack
       [64] = Deploy
       [67] = Holster
       [90] = WeaponIdle
       [78] = GetMaxSpeed
       [63] = CanDrop
       [93] = UseDecrement
    } */
    /* size: 396, cachelines: 7, members: 3      */
    /* sum members: 58, holes: 1, sum holes: 336 */
    /* padding: 2                                */
    /* last cacheline: 12 bytes                  */
};

class CM249 : public CBasePlayerWeapon 
{
    public:

        void Spawn( void );
        void Precache( void );
        int iItemSlot( void );
        int GetItemInfo( ItemInfo *p );

        void PrimaryAttack( void );
        void M249Fire( float flSpread, float flCycleTime, BOOL fUseAutoAim );
        BOOL Deploy( void );
        void Reload( void );
        void WeaponIdle( void );
        float GetMaxSpeed( void );
        BOOL UseDecrement( void );

    public:

        int                        m_iShell;             /*   336     4 */
        int                        iShellOn;             /*   340     4 */

    private:

        short unsigned int         m_usFireM249;         /*   344     2 */

    /* vtable has 10 entries:
    {
        [0]  = Spawn
        [1]  = Precache
        [79] = iItemSlot
        [61] = GetItemInfo
        [87] = PrimaryAttack
        [64] = Deploy
        [89] = Reload
        [90] = WeaponIdle
        [78] = GetMaxSpeed
        [93] = UseDecrement
    } */
    /* size: 348, cachelines: 6, members: 4      */
    /* sum members: 10, holes: 1, sum holes: 336 */
    /* padding: 2                                */
    /* last cacheline: 28 bytes                  */
};

class CM3 : public CBasePlayerWeapon 
{
    public:

        void Spawn( void );
        void Precache( void );
        int iItemSlot( void );
        int GetItemInfo( ItemInfo *p );

        void PrimaryAttack( void );
        BOOL Deploy( void );
        void Reload( void );
        void WeaponIdle( void );
        float GetMaxSpeed( void );
        BOOL UseDecrement( void );

    public:

        int                 m_iShell;             /*   336     4 */
        float               m_flPumpTime;         /*   340     4 */

    private:

        short unsigned int  m_usFireM3;           /*   344     2 */

    /* vtable has 10 entries: 
    {
        [0]  = Spawn
        [1]  = Precache
        [79] = iItemSlot
        [61] = GetItemInfo
        [87] = PrimaryAttack
        [64] = Deploy
        [89] = Reload
        [90] = WeaponIdle
        [78] = GetMaxSpeed
        [93] = UseDecrement
    } */
    /* size: 348, cachelines: 6, members: 4      */
    /* sum members: 10, holes: 1, sum holes: 336 */
    /* padding: 2                                */
    /* last cacheline: 28 bytes                  */
};

class CM4A1 : public CBasePlayerWeapon 
{
    public:

        void Spawn( void );
        void Precache( void );
        int iItemSlot( void );
        int GetItemInfo( ItemInfo *p );

        void PrimaryAttack( void );
        void SecondaryAttack( void );
        void M4A1Fire( float flSpread, float flCycleTime, BOOL fUseAutoAim );
        BOOL Deploy( void );
        void Reload( void );
        void WeaponIdle( void );
        float GetMaxSpeed( void );
        BOOL UseDecrement( void );

    public:

        int                     m_iShell;             /*   336     4 */
        int                     iShellOn;             /*   340     4 */

    private:

        short unsigned int      m_usFireM4A1;         /*   344     2 */

    /* vtable has 11 entries: 
    {
       [0]  = Spawn
       [1]  = Precache
       [79] = iItemSlot
       [61] = GetItemInfo
       [87] = PrimaryAttack
       [88] = SecondaryAttack
       [64] = Deploy
       [67] = Holster
       [90] = WeaponIdle
       [78] = GetMaxSpeed
       [93] = UseDecrement
    } */
    /* size: 348, cachelines: 6, members: 4      */
    /* sum members: 10, holes: 1, sum holes: 336 */
    /* padding: 2                                */
    /* last cacheline: 28 bytes                  */
};

class CMAC10 : public CBasePlayerWeapon 
{
    public:

        void Spawn( void );
        void Precache( void );
        int iItemSlot( void );
        int GetItemInfo( ItemInfo *p );

        void PrimaryAttack( void );
        void MAC10Fire( float flSpread, float flCycleTime, BOOL fUseAutoAim );
        BOOL Deploy( void );
        void Reload( void );
        void WeaponIdle( void );
        float GetMaxSpeed( void );
        BOOL UseDecrement( void );

    public:

        int                 m_iShell;             /*   336     4 */
        int                 iShellOn;             /*   340     4 */

    private:

        short unsigned int  m_usFireMAC10;        /*   344     2 */

    /* vtable has 10 entries: 
    {
       [0]  = Spawn
       [1]  = Precache
       [79] = iItemSlot
       [61] = GetItemInfo
       [87] = PrimaryAttack
       [64] = Deploy
       [67] = Holster
       [90] = WeaponIdle
       [78] = GetMaxSpeed
       [93] = UseDecrement
    } */
    /* size: 348, cachelines: 6, members: 4      */
    /* sum members: 10, holes: 1, sum holes: 336 */
    /* padding: 2                                */
    /* last cacheline: 28 bytes                  */
};

class CMP5N : public CBasePlayerWeapon 
{
    public:

        void Spawn( void );
        void Precache( void );
        int iItemSlot( void );
        int GetItemInfo( ItemInfo *p );

        void PrimaryAttack( void );
        void MP5NFire( float flSpread, float flCycleTime, BOOL fUseAutoAim );
        BOOL Deploy( void );
        void Reload( void );
        void WeaponIdle( void );
        float GetMaxSpeed( void );
        BOOL UseDecrement( void );

    public:

        int                 m_iShell;             /*   336     4 */
        int                 iShellOn;             /*   340     4 */

    private:

        short unsigned int  m_usFireMP5N;         /*   344     2 */

    /* vtable has 10 entries: 
    {
       [0]  = Spawn
       [1]  = Precache
       [79] = iItemSlot
       [61] = GetItemInfo
       [87] = PrimaryAttack
       [64] = Deploy
       [67] = Holster
       [90] = WeaponIdle
       [78] = GetMaxSpeed
       [93] = UseDecrement
    } */
    /* size: 348, cachelines: 6, members: 4      */
    /* sum members: 10, holes: 1, sum holes: 336 */
    /* padding: 2                                */
    /* last cacheline: 28 bytes                  */
};

class CP228 : public CBasePlayerWeapon 
{
    public:

        void Spawn( void );
        void Precache( void );
        int iItemSlot( void );
        int GetItemInfo( ItemInfo *p );

        void PrimaryAttack( void );
        void SecondaryAttack( void );
        void P228Fire( float flSpread, float flCycleTime, BOOL fUseAutoAim );
        BOOL Deploy( void );
        void Reload( void );
        void WeaponIdle( void );

        void MakeBeam( void );   /* unused */
        void BeamUpdate( void ); /* unused */

        float GetMaxSpeed( void );
        BOOL UseDecrement( void );
        BOOL IsPistol( void );

    public:

        int                 m_iShell;             /*   336     4 */

    private:

        short unsigned int  m_usFireP228;         /*   340     2 */

    /* vtable has 12 entries: 
    {
        [0]  = Spawn
        [1]  = Precache
        [79] = iItemSlot
        [61] = GetItemInfo
        [87] = PrimaryAttack
        [88] = SecondaryAttack
        [64] = Deploy
        [67] = Holster
        [90] = WeaponIdle
        [78] = GetMaxSpeed
        [93] = UseDecrement
        [94] = IsPistol
    } */
    /* size: 344, cachelines: 6, members: 3     */
    /* sum members: 6, holes: 1, sum holes: 336 */
    /* padding: 2                               */
    /* last cacheline: 24 bytes                 */
};

class CP90 : public CBasePlayerWeapon 
{
    public:

        void Spawn( void );
        void Precache( void );
        int iItemSlot( void );
        int GetItemInfo( ItemInfo *p );

        void PrimaryAttack( void );
        void P90Fire( float flSpread, float flCycleTime, BOOL fUseAutoAim );
        BOOL Deploy( void );
        void Reload( void );
        void WeaponIdle( void );
        float GetMaxSpeed( void );
        BOOL UseDecrement( void );

    public:

        int                 m_iShell;             /*   336     4 */
        int                 iShellOn;             /*   340     4 */
    private:

        short unsigned int  m_usFireP90;          /*   344     2 */

    /* vtable has 10 entries:
    {
       [0]  = Spawn
       [1]  = Precache
       [79] = iItemSlot
       [61] = GetItemInfo
       [87] = PrimaryAttack
       [64] = Deploy
       [67] = Holster
       [90] = WeaponIdle
       [78] = GetMaxSpeed
       [93] = UseDecrement
    } */
    /* size: 348, cachelines: 6, members: 4      */
    /* sum members: 10, holes: 1, sum holes: 336 */
    /* padding: 2                                */
    /* last cacheline: 28 bytes                  */
};

class CSCOUT : public CBasePlayerWeapon 
{
    public:

        void Spawn( void );
        void Precache( void );
        int iItemSlot( void );
        int GetItemInfo( ItemInfo *p );

        void PrimaryAttack( void );
        void SecondaryAttack( void );
        void SCOUTFire( float flSpread, float flCycleTime, BOOL fUseAutoAim );
        BOOL Deploy( void );
        void Reload( void );
        void WeaponIdle( void );

        float GetMaxSpeed( void );
        BOOL UseDecrement( void );

    public:

        int                 m_iShell;             /*   336     4 */

    private:

        short unsigned int  m_usFireScout;        /*   340     2 */

    /* vtable has 11 entries: 
    {
        [0]  = Spawn
        [1]  = Precache
        [79] = iItemSlot
        [61] = GetItemInfo
        [87] = PrimaryAttack
        [88] = SecondaryAttack
        [64] = Deploy
        [67] = Holster
        [90] = WeaponIdle
        [78] = GetMaxSpeed
        [93] = UseDecrement
    } */
    /* size: 344, cachelines: 6, members: 3     */
    /* sum members: 6, holes: 1, sum holes: 336 */
    /* padding: 2                               */
    /* last cacheline: 24 bytes                 */
};

class CSG550 : public CBasePlayerWeapon 
{
    public:

        void Spawn( void );
        void Precache( void );
        int iItemSlot( void );
        int GetItemInfo( ItemInfo *p );

        void PrimaryAttack( void );
        void SecondaryAttack( void );
        void SG550Fire( float flSpread, float flCycleTime, BOOL fUseAutoAim );
        BOOL Deploy( void );
        void Reload( void );
        void WeaponIdle( void );

        float GetMaxSpeed( void );
        BOOL UseDecrement( void );

    public:

        int                 m_iShell;             /*   336     4 */

    private:

        short unsigned int  m_usFireSG550;        /*   340     2 */

    /* vtable has 11 entries: 
    {
        [0]  = Spawn
        [1]  = Precache
        [79] = iItemSlot
        [61] = GetItemInfo
        [87] = PrimaryAttack
        [88] = SecondaryAttack
        [64] = Deploy
        [67] = Holster
        [90] = WeaponIdle
        [78] = GetMaxSpeed
        [93] = UseDecrement
    } */
    /* size: 344, cachelines: 6, members: 3     */
    /* sum members: 6, holes: 1, sum holes: 336 */
    /* padding: 2                               */
    /* last cacheline: 24 bytes                 */
};

class CSG552 : public CBasePlayerWeapon 
{
    public:

        void Spawn( void );
        void Precache( void );
        int iItemSlot( void );
        int GetItemInfo( ItemInfo *p );

        void PrimaryAttack( void );
        void SecondaryAttack( void );
        void SG552Fire( float flSpread, float flCycleTime, BOOL fUseAutoAim );
        BOOL Deploy( void );
        void Reload( void );
        void WeaponIdle( void );

        float GetMaxSpeed( void );
        BOOL UseDecrement( void );

    public:

        int                 m_iShell;             /*   336     4 */
        int                 iShellOn;             /*   340     4 */

    private:

        short unsigned int  m_usFireSG552;        /*   344     2 */

    /* vtable has 11 entries: 
    {
        [0]  = Spawn
        [1]  = Precache
        [79] = iItemSlot
        [61] = GetItemInfo
        [87] = PrimaryAttack
        [88] = SecondaryAttack
        [64] = Deploy
        [67] = Holster
        [90] = WeaponIdle
        [78] = GetMaxSpeed
        [93] = UseDecrement
    } */
    /* size: 348, cachelines: 6, members: 4      */
    /* sum members: 10, holes: 1, sum holes: 336 */
    /* padding: 2                                */
    /* last cacheline: 28 bytes                  */
};

class CSmokeGrenade : public CBasePlayerWeapon 
{
    public:

        void Spawn( void );
        void Precache( void );
        int iItemSlot( void );
        int GetItemInfo( ItemInfo *p );

        void PrimaryAttack( void );
        void SecondaryAttack( void );
        BOOL Deploy( void );
        void Holster( int skiplocal );
        void WeaponIdle( void );
        float GetMaxSpeed( void );
        BOOL CanDeploy( void );
        BOOL CanDrop( void );
        BOOL UseDecrement( void );

        bool ShieldSecondaryFire( int up_anim, int down_anim );
        void SetPlayerShieldAnim( void );
        void ResetPlayerShieldAnim( void );

    public:

        short unsigned int  m_usCreateSmoke;      /*   336     2 */

    /* vtable has 13 entries: 
    {
       [0]  = Spawn
       [1]  = Precache
       [79] = iItemSlot
       [61] = GetItemInfo
       [87] = PrimaryAttack
       [88] = SecondaryAttack
       [64] = Deploy
       [67] = Holster
       [90] = WeaponIdle
       [78] = GetMaxSpeed
       [63] = CanDrop
       [62] = CanDeploy
       [93] = UseDecrement
    } */
    /* size: 340, cachelines: 6, members: 2     */
    /* sum members: 2, holes: 1, sum holes: 336 */
    /* padding: 2                               */
    /* last cacheline: 20 bytes                 */
};

class CTMP : public CBasePlayerWeapon 
{
    public:

        void Spawn( void );
        void Precache( void );
        int iItemSlot( void );
        int GetItemInfo( ItemInfo *p );

        void PrimaryAttack( void );
        void TMPFire( float flSpread, float flCycleTime, BOOL fUseAutoAim );
        BOOL Deploy( void );
        void Reload( void );
        void WeaponIdle( void );
        float GetMaxSpeed( void );
        BOOL UseDecrement( void );

    public:

        int                 m_iShell;             /*   336     4 */
        int                 iShellOn;             /*   340     4 */

    private:

        short unsigned int  m_usFireTMP;          /*   344     2 */

    /* vtable has 10 entries: 
    {
       [0]  = Spawn
       [1]  = Precache
       [79] = iItemSlot
       [61] = GetItemInfo
       [87] = PrimaryAttack
       [64] = Deploy
       [67] = Holster
       [90] = WeaponIdle
       [78] = GetMaxSpeed
       [93] = UseDecrement
    } */
    /* size: 348, cachelines: 6, members: 4      */
    /* sum members: 10, holes: 1, sum holes: 336 */
    /* padding: 2                                */
    /* last cacheline: 28 bytes                  */
};

class CUMP45 : public CBasePlayerWeapon 
{
    public:

        void Spawn( void );
        void Precache( void );
        int iItemSlot( void );
        int GetItemInfo( ItemInfo *p );

        void PrimaryAttack( void );
        void UMP45Fire( float flSpread, float flCycleTime, BOOL fUseAutoAim );
        BOOL Deploy( void );
        void Reload( void );
        void WeaponIdle( void );
        float GetMaxSpeed( void );
        BOOL UseDecrement( void );

    public:

        int                 m_iShell;             /*   336     4 */
        int                 iShellOn;             /*   340     4 */

    private:

        short unsigned int  m_usFireUMP45;        /*   344     2 */

    /* vtable has 10 entries: 
    {
       [0]  = Spawn
       [1]  = Precache
       [79] = iItemSlot
       [61] = GetItemInfo
       [87] = PrimaryAttack
       [64] = Deploy
       [67] = Holster
       [90] = WeaponIdle
       [78] = GetMaxSpeed
       [93] = UseDecrement
    } */
    /* size: 348, cachelines: 6, members: 4      */
    /* sum members: 10, holes: 1, sum holes: 336 */
    /* padding: 2                                */
    /* last cacheline: 28 bytes                  */
};

class CUSP : public CBasePlayerWeapon 
{
    public:

        void Spawn( void );
        void Precache( void );
        int iItemSlot( void );
        int GetItemInfo( ItemInfo *p );

        void PrimaryAttack( void );
        void SecondaryAttack( void );
        void USPFire( float flSpread, float flCycleTime, BOOL fUseAutoAim );
        BOOL Deploy( void );
        void Reload( void );
        void WeaponIdle( void );

        void MakeBeam( void );   /* unused */
        void BeamUpdate( void ); /* unused */

        float GetMaxSpeed( void );
        BOOL UseDecrement( void );
        BOOL IsPistol( void );

    public:

        int                 m_iShell;             /*   336     4 */

    private:

        short unsigned int  m_usFireUSP;          /*   340     2 */

    /* vtable has 12 entries: 
    {
        [0]  = Spawn
        [1]  = Precache
        [79] = iItemSlot
        [61] = GetItemInfo
        [87] = PrimaryAttack
        [88] = SecondaryAttack
        [64] = Deploy
        [67] = Holster
        [90] = WeaponIdle
        [78] = GetMaxSpeed
        [93] = UseDecrement
        [94] = IsPistol
    } */
    /* size: 344, cachelines: 6, members: 3     */
    /* sum members: 6, holes: 1, sum holes: 336 */
    /* padding: 2                               */
    /* last cacheline: 24 bytes                 */
};

class CXM1014 : public CBasePlayerWeapon 
{
    public:

        void Spawn( void );
        void Precache( void );
        int iItemSlot( void );
        int GetItemInfo( ItemInfo *p );

        void PrimaryAttack( void );
        BOOL Deploy( void );
        void Reload( void );
        void WeaponIdle( void );

        float GetMaxSpeed( void );
        BOOL UseDecrement( void );

    public:

        int                 m_iShell;             /*   336     4 */
        float               m_flPumpTime;         /*   340     4 */

    private:

        short unsigned int  m_usFireXM1014;       /*   344     2 */

    /* vtable has 10 entries: 
    {
        [0]  = Spawn
        [1]  = Precache
        [79] = iItemSlot
        [61] = GetItemInfo
        [87] = PrimaryAttack
        [64] = Deploy
        [67] = Holster
        [90] = WeaponIdle
        [78] = GetMaxSpeed
        [93] = UseDecrement
    } */
    /* size: 348, cachelines: 6, members: 4      */
    /* sum members: 10, holes: 1, sum holes: 336 */
    /* padding: 2                                */
    /* last cacheline: 28 bytes                  */
};


//=========================================================
// CWeaponBox - a single entity that can store weapons
// and ammo. 
//=========================================================
class CWeaponBox : public CBaseEntity
{
	void Precache( void );
	void Spawn( void );
	void Touch( CBaseEntity *pOther );
	void KeyValue( KeyValueData *pkvd );
	BOOL IsEmpty( void );
	int  GiveAmmo( int iCount, char *szName, int iMax, int *pIndex = NULL );
	void SetObjectCollisionBox( void );

public:
	void EXPORT Kill ( void );
	int		Save( CSave &save );
	int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

	BOOL HasWeapon( CBasePlayerItem *pCheckItem );
	BOOL PackWeapon( CBasePlayerItem *pWeapon );
	BOOL PackAmmo( int iszName, int iCount );
	
	CBasePlayerItem	*m_rgpPlayerItems[MAX_ITEM_TYPES];// one slot for each 

	int m_rgiszAmmo[MAX_AMMO_SLOTS];// ammo names
	int	m_rgAmmo[MAX_AMMO_SLOTS];// ammo quantities

	int m_cAmmoTypes;// how many ammo types packed into this box (if packed by a level designer)
};

#ifdef CLIENT_DLL
bool bIsMultiplayer ( void );
void LoadVModel ( char *szViewModel, CBasePlayer *m_pPlayer );
#endif

class CLaserSpot : public CBaseEntity
{
	void Spawn( void );
	void Precache( void );

	int	ObjectCaps( void ) { return FCAP_DONT_SAVE; }

public:
	void Suspend( float flSuspendTime );
	void EXPORT Revive( void );
	
	static CLaserSpot *CreateSpot( void );
};

#endif // WEAPONS_H
