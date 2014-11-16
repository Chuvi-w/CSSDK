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

class CHalfLifeTraining : public CHalfLifeMultiplay 
{
public:
	CHalfLifeTraining();

public: 
	static void HostageDied();
	static bool PlayerCanBuy(CBasePlayer *);

public:
	virtual BOOL IsMultiplayer          (void);
	virtual BOOL IsDeathmatch           (void);
	virtual void InitHUD                (CBasePlayer *);
	virtual edict_t * GetPlayerSpawnSpot(CBasePlayer *);
	virtual void PlayerSpawn            (CBasePlayer *);
	virtual BOOL ItemShouldRespawn      (CItem *);
	virtual BOOL FPlayerCanRespawn      (CBasePlayer *);
	virtual void PlayerThink            (CBasePlayer *);
	virtual void PlayerKilled           (CBasePlayer *, entvars_t *, entvars_t *);
	virtual void CheckWinConditions     (void);
	virtual void CheckMapConditions     (void);

private:
	float                      FillAccountTime;      /*   708     4 */
	float                      ServerRestartTime;    /*   712     4 */
	BOOL                       fInBuyArea;           /*   716     4 */
	BOOL                       fVisitedBuyArea;      /*   720     4 */
	bool                       fVGUIMenus;           /*   724     1 */

	/* vtable has 11 entries: {
	[6]  = IsMultiplayer     
	[7]  = IsDeathmatch      
	[12] = InitHUD           
	[22] = GetPlayerSpawnSpot
	[18] = PlayerSpawn       
	[38] = ItemShouldRespawn 
	[20] = FPlayerCanRespawn 
	[19] = PlayerThink       
	[28] = PlayerKilled      
	[65] = CheckWinConditions
	[62] = CheckMapConditions
	} */
	/* size: 728, cachelines: 12, members: 6 */
	/* sum members: 17, holes: 1, sum holes: 708 */
	/* padding: 3 */
	/* last cacheline: 24 bytes */
};
