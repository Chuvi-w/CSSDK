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

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "monsters.h"
#include "weapons.h"

struct WeaponAliasInfo // Last check: 2013, August 13
{
	const char			*alias;		/*     0     4 */
	WeaponIdType		id;			/*     4     4 */

	/* size: 8, cachelines: 1, members: 2 */
	/* last cacheline: 8 bytes */
};

struct WeaponBuyAliasInfo // Last check: 2013, August 13
{
	const char			*alias;		/*     0     4 */
	WeaponIdType		id;			/*     4     4 */
	const char			*failName;	/*     8     4 */

	/* size: 12, cachelines: 1, members: 3 */
	/* last cacheline: 12 bytes */
};

struct WeaponClassAliasInfo // Last check: 2013, August 13
{
	const char			*alias;		/*     0     4 */
	WeaponClassType		id;			/*     4     4 */

	/* size: 8, cachelines: 1, members: 2 */
	/* last cacheline: 8 bytes */
};

WeaponAliasInfo weaponAliasInfo[] = 
{
	{ "p228"	, WEAPON_P228		},
	{ "???"		, WEAPON_GLOCK		},
	{ "scout"	, WEAPON_SCOUT		},
	{ "hegren"	, WEAPON_HEGRENADE	},
	{ "xm1014"	, WEAPON_XM1014		},
	{ "c4"		, WEAPON_C4			},
	{ "mac10"	, WEAPON_MAC10		},
	{ "aug"		, WEAPON_AUG		},
	{ "sgren"	, WEAPON_SMOKEGRENADE },
	{ "elites"	, WEAPON_ELITE		},
	{ "fn57"	, WEAPON_FIVESEVEN	},
	{ "ump45"	, WEAPON_UMP45		},
	{ "sg550"	, WEAPON_SG550		},
	{ "galil"	, WEAPON_GALIL		},
	{ "famas"	, WEAPON_FAMAS		},
	{ "usp"		, WEAPON_USP		},
	{ "glock"	, WEAPON_GLOCK18	},
	{ "awp"		, WEAPON_AWP		},
	{ "mp5"		, WEAPON_MP5N		},
	{ "m249"	, WEAPON_M249		},
	{ "m3"		, WEAPON_M3			},
	{ "m4a1"	, WEAPON_M4A1		},
	{ "tmp"		, WEAPON_TMP		},
	{ "g3sg1"	, WEAPON_G3SG1		},
	{ "flash"	, WEAPON_FLASHBANG	},
	{ "deagle"	, WEAPON_DEAGLE		},
	{ "sg552"	, WEAPON_SG552		},
	{ "ak47"	, WEAPON_AK47		},
	{ "knife"	, WEAPON_KNIFE		},
	{ "p90"		, WEAPON_P90		},
	{ "shield"	, WEAPON_SHIELDGUN	},
	{ "none"	, WEAPON_NONE		},
	{ "grenade"	, WEAPON_HEGRENADE	},
	{ "hegrenade", WEAPON_HEGRENADE },
	{ "glock18"	, WEAPON_GLOCK18	},
	{ "elite"	, WEAPON_ELITE		},
	{ "fiveseven", WEAPON_FIVESEVEN },
	{ "mp5navy"	, WEAPON_MP5N		},
	{ NULL		, WEAPON_NONE		}
};

WeaponBuyAliasInfo weaponBuyAliasInfo[] =
{
	{ "galil"		, WEAPON_GALIL		, "#Galil"	},
	{ "defender"	, WEAPON_GALIL		, "#Galil"	},
	{ "ak47"		, WEAPON_AK47		, "#AK47"	},
	{ "cv47"		, WEAPON_AK47		, "#AK47"	},
	{ "scout"		, WEAPON_SCOUT		, NULL		},
	{ "sg552"		, WEAPON_SG552		, "#SG552"	},
	{ "krieg552"	, WEAPON_SG552		, "#SG552"	},
	{ "awp"			, WEAPON_AWP		, NULL		},
	{ "magnum"		, WEAPON_AWP		, NULL		},
	{ "g3sg1"		, WEAPON_G3SG1		, "#G3SG1"	},
	{ "d3au1"		, WEAPON_G3SG1		, "#G3SG1"	},
	{ "famas"		, WEAPON_FAMAS		, "#Famas"	},
	{ "clarion"		, WEAPON_FAMAS		, "#Famas"	},
	{ "m4a1"		, WEAPON_M4A1		, "#M4A1"	},
	{ "aug"			, WEAPON_AUG		, "#Aug"	},
	{ "bullpup"		, WEAPON_AUG		, "#Aug"	},
	{ "sg550"		, WEAPON_SG550		, "#SG550"	},
	{ "krieg550"	, WEAPON_SG550		, "#SG550"	},
	{ "glock"		, WEAPON_GLOCK18	, NULL		},
	{ "9x19mm"		, WEAPON_GLOCK18	, NULL		},
	{ "usp"			, WEAPON_USP		, NULL		},
	{ "km45"		, WEAPON_USP		, NULL		},
	{ "p228"		, WEAPON_P228		, NULL		},
	{ "228compact"	, WEAPON_P228		, NULL		},
	{ "deagle"		, WEAPON_DEAGLE		, NULL		},
	{ "nighthaw"	, WEAPON_DEAGLE		, NULL		},
	{ "elites"		, WEAPON_ELITE		, "#Beretta96G" },
	{ "fn57"		, WEAPON_FIVESEVEN	, "#FiveSeven"	},
	{ "fiveseven"	, WEAPON_FIVESEVEN	, "#FiveSeven"	},
	{ "m3"			, WEAPON_M3			, NULL		},
	{ "12gauge"		, WEAPON_M3			, NULL		},
	{ "xm1014"		, WEAPON_XM1014		, NULL		},
	{ "autoshotgun"	, WEAPON_XM1014		, NULL		},
	{ "mac10"		, WEAPON_MAC10		, "#Mac10" },
	{ "tmp"			, WEAPON_TMP		, "#tmp"	},
	{ "mp"			, WEAPON_TMP		, "#tmp"	},
	{ "mp5"			, WEAPON_MP5N		, NULL		},
	{ "smg"			, WEAPON_MP5N		, NULL		},
	{ "ump45"		, WEAPON_UMP45		, NULL		},
	{ "p90"			, WEAPON_P90		, NULL		},
	{ "c90"			, WEAPON_P90		, NULL		},
	{ "m249"		, WEAPON_M249		, NULL		},
	{ NULL			, WEAPON_NONE		, NULL		}
};

WeaponClassAliasInfo weaponClassAliasInfo[] =
{
	{ "p228"		, WEAPONCLASS_PISTOL		},
	{ "???"			, WEAPONCLASS_PISTOL		},
	{ "scout"		, WEAPONCLASS_SNIPERRIFLE	},
	{ "hegren"		, WEAPONCLASS_GRENADE		},
	{ "xm1014"		, WEAPONCLASS_SHOTGUN		},
	{ "c4"			, WEAPONCLASS_GRENADE		},
	{ "mac10"		, WEAPONCLASS_SUBMACHINEGUN },
	{ "aug"			, WEAPONCLASS_RIFLE			},
	{ "sgren"		, WEAPONCLASS_GRENADE		},
	{ "elites"		, WEAPONCLASS_PISTOL		},
	{ "fn57"		, WEAPONCLASS_PISTOL		},
	{ "ump45"		, WEAPONCLASS_SUBMACHINEGUN },
	{ "sg550"		, WEAPONCLASS_SNIPERRIFLE	},
	{ "galil"		, WEAPONCLASS_RIFLE			},
	{ "famas"		, WEAPONCLASS_RIFLE			},
	{ "usp"			, WEAPONCLASS_PISTOL		},
	{ "glock"		, WEAPONCLASS_PISTOL		},
	{ "awp"			, WEAPONCLASS_SNIPERRIFLE	},
	{ "mp5"			, WEAPONCLASS_SUBMACHINEGUN },
	{ "m249"		, WEAPONCLASS_MACHINEGUN	},
	{ "m3"			, WEAPONCLASS_SHOTGUN		},
	{ "m4a1"		, WEAPONCLASS_RIFLE			},
	{ "tmp"			, WEAPONCLASS_SUBMACHINEGUN },
	{ "g3sg1"		, WEAPONCLASS_SNIPERRIFLE	},
	{ "flash"		, WEAPONCLASS_GRENADE		},
	{ "deagle"		, WEAPONCLASS_PISTOL		},
	{ "sg552"		, WEAPONCLASS_RIFLE			},
	{ "ak47"		, WEAPONCLASS_RIFLE			},
	{ "knife"		, WEAPONCLASS_KNIFE			},
	{ "p90"			, WEAPONCLASS_SUBMACHINEGUN },
	{ "shield"		, WEAPONCLASS_PISTOL		},
	{ "grenade"		, WEAPONCLASS_GRENADE		},
	{ "hegrenade"	, WEAPONCLASS_GRENADE		},
	{ "glock18"		, WEAPONCLASS_PISTOL		},
	{ "elite"		, WEAPONCLASS_PISTOL		},
	{ "fiveseven"	, WEAPONCLASS_PISTOL		},
	{ "mp5navy"		, WEAPONCLASS_SUBMACHINEGUN },
	{ "grenade"		, WEAPONCLASS_GRENADE		},
	{ "pistol"		, WEAPONCLASS_PISTOL		},
	{ "SMG"			, WEAPONCLASS_SUBMACHINEGUN },
	{ "machinegun"	, WEAPONCLASS_MACHINEGUN	},
	{ "shotgun"		, WEAPONCLASS_SHOTGUN		},
	{ "rifle"		, WEAPONCLASS_RIFLE			},
	{ "rifle"		, WEAPONCLASS_RIFLE			},
	{ "sniper"		, WEAPONCLASS_SNIPERRIFLE	},
	{ "none"		, WEAPONCLASS_NONE			},
	{ NULL			, WEAPONCLASS_NONE			}
};

WeaponIdType AliasToWeaponID( const char *alias ) // Last check: 2013, August 13.
{
	if( alias )
	{
		for( int i = WEAPON_NONE; weaponAliasInfo[i].alias != NULL; ++i )
		{
			if( !stricmp( weaponAliasInfo[i].alias, alias ) )
			{
				return weaponAliasInfo[i].id;
			}
		}
	}

	return WEAPON_NONE;
}

const char *BuyAliasToWeaponID( const char *alias, WeaponIdType &id ) // Last check: 2013, August 13.
{
	if( alias )
	{
		for( int i = WEAPON_NONE; weaponBuyAliasInfo[i].alias != NULL; ++i )
		{
			if( !stricmp( weaponBuyAliasInfo[i].alias, alias ) )
			{
				id = weaponBuyAliasInfo[i].id;

				return weaponBuyAliasInfo[i].failName;
			}
		}
	}

	id = WEAPON_NONE;

	return NULL;
}

const char *WeaponIDToAlias( int id ) // Last check: 2013, August 13.
{
	for( int i = WEAPON_NONE; weaponBuyAliasInfo[i].alias != NULL; ++i )
	{
		if( weaponAliasInfo[i].id == id )
		{
			return weaponAliasInfo[i].alias;
		}
	}

	return NULL;
}

WeaponClassType AliasToWeaponClass( const char *alias ) // Last check: 2013, August 13.
{
	if( alias )
	{
		for( int i = WEAPON_NONE; weaponBuyAliasInfo[i].alias != NULL; ++i )
		{
			if( !stricmp( weaponClassAliasInfo[i].alias, alias ) )
			{
				return weaponClassAliasInfo[i].id;
			}
		}
	}

	return WEAPONCLASS_NONE;
}

WeaponClassType WeaponIDToWeaponClass( int id ) // Last check: 2013, August 13.
{
	for( int i = WEAPON_NONE; weaponClassAliasInfo[i].alias != NULL; ++i )
	{
		if( weaponClassAliasInfo[i].id == id )
		{
			return AliasToWeaponClass( weaponAliasInfo[i].alias );
		}
	}

	return WEAPONCLASS_NONE;
}

bool IsPrimaryWeapon( int id ) // Last check: 2013, August 13.
{
	switch( id )
	{
		case WEAPON_SCOUT:
		case WEAPON_XM1014:
		case WEAPON_MAC10:
		case WEAPON_AUG:
		case WEAPON_UMP45:
		case WEAPON_SG550:
		case WEAPON_GALIL:
		case WEAPON_FAMAS:
		case WEAPON_AWP:
		case WEAPON_MP5N:
		case WEAPON_M249:
		case WEAPON_M3:
		case WEAPON_M4A1:
		case WEAPON_TMP:
		case WEAPON_G3SG1:
		case WEAPON_SG552:
		case WEAPON_AK47:
		case WEAPON_P90:
		case WEAPON_SHIELDGUN: 
		{
			return true;
		}
			
	}
	
	return false;
}

bool IsSecondaryWeapon( int id ) // Last check: 2013, August 13.
{
	switch( id )
	{
		case WEAPON_P228:
		case WEAPON_ELITE:
		case WEAPON_FIVESEVEN:
		case WEAPON_USP:
		case WEAPON_GLOCK18:
		case WEAPON_DEAGLE:
		{
			return true;
		}
	}

	return false;
}

bool CanBuyWeaponByMaptype( int playerTeam, WeaponIdType weaponID, bool useAssasinationRestrictions ) // Last check: 2013, August 13.
{
	if( useAssasinationRestrictions)
	{
		if( playerTeam == CT )
		{
			switch( weaponID )
			{
				case WEAPON_P228:
				case WEAPON_XM1014:
				case WEAPON_AUG:
				case WEAPON_FIVESEVEN:
				case WEAPON_UMP45:
				case WEAPON_SG550:
				case WEAPON_FAMAS:
				case WEAPON_USP:
				case WEAPON_GLOCK18:
				case WEAPON_MP5N:
				case WEAPON_M249:
				case WEAPON_M3:
				case WEAPON_M4A1:
				case WEAPON_TMP:
				case WEAPON_DEAGLE:
				case WEAPON_P90:
				case WEAPON_SHIELDGUN: 
				{
					return true;
				}
			}
		}
		else if( playerTeam == TERRORIST )
		{
			switch( weaponID )
			{
				case WEAPON_P228:
				case WEAPON_MAC10:
				case WEAPON_ELITE:
				case WEAPON_UMP45:
				case WEAPON_GALIL:
				case WEAPON_USP:
				case WEAPON_GLOCK18:
				case WEAPON_AWP:
				case WEAPON_DEAGLE:
				case WEAPON_AK47: 
				{
					return true;
				}
			}
		}

		return false;
	}

	if( playerTeam == CT )
	{
		switch( weaponID )
		{
			case WEAPON_P228:
			case WEAPON_SCOUT:
			case WEAPON_XM1014:
			case WEAPON_AUG:
			case WEAPON_FIVESEVEN:
			case WEAPON_UMP45:
			case WEAPON_SG550:
			case WEAPON_FAMAS:
			case WEAPON_USP:
			case WEAPON_GLOCK18:
			case WEAPON_AWP:
			case WEAPON_MP5N:
			case WEAPON_M249:
			case WEAPON_M3:
			case WEAPON_M4A1:
			case WEAPON_TMP:
			case WEAPON_DEAGLE:
			case WEAPON_P90:
			case WEAPON_SHIELDGUN: 
			{
				return true;
			}
		}
	}
	else if( playerTeam == TERRORIST )
	{
		switch( weaponID )
		{
			case WEAPON_P228:
			case WEAPON_SCOUT:
			case WEAPON_XM1014:
			case WEAPON_MAC10:
			case WEAPON_ELITE:
			case WEAPON_UMP45:
			case WEAPON_GALIL:
			case WEAPON_USP:
			case WEAPON_GLOCK18:
			case WEAPON_AWP:
			case WEAPON_MP5N:
			case WEAPON_M249:
			case WEAPON_M3:
			case WEAPON_G3SG1:
			case WEAPON_DEAGLE:
			case WEAPON_SG552:
			case WEAPON_AK47:
			case WEAPON_P90: 
			{
				return true;
			}
		}
	}

	return false;
}

