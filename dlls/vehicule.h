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

#ifndef VEHICULE_H
#define VEHICULE_H

#define VEHICLE_SPEED0_ACCELERATION  0.005000000000000000
#define VEHICLE_SPEED1_ACCELERATION  0.002142857142857143
#define VEHICLE_SPEED2_ACCELERATION  0.003333333333333334
#define VEHICLE_SPEED3_ACCELERATION  0.004166666666666667
#define VEHICLE_SPEED4_ACCELERATION  0.004000000000000000
#define VEHICLE_SPEED5_ACCELERATION  0.003800000000000000
#define VEHICLE_SPEED6_ACCELERATION  0.004500000000000000
#define VEHICLE_SPEED7_ACCELERATION  0.004250000000000000
#define VEHICLE_SPEED8_ACCELERATION  0.002666666666666667
#define VEHICLE_SPEED9_ACCELERATION  0.002285714285714286
#define VEHICLE_SPEED10_ACCELERATION 0.001875000000000000
#define VEHICLE_SPEED11_ACCELERATION 0.001444444444444444
#define VEHICLE_SPEED12_ACCELERATION 0.001200000000000000
#define VEHICLE_SPEED13_ACCELERATION 0.000916666666666666
#define VEHICLE_SPEED14_ACCELERATION 0.001444444444444444

class CFuncVehicle : public CBaseEntity 
{
public:

	void Spawn(void); 
	void Precache(void); 
	void Blocked(CBaseEntity *pOther);
	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	void KeyValue(KeyValueData *pkvd);

	void EXPORT Next(void);
	void EXPORT Find(void);
	void EXPORT NearestPath(void);
	void EXPORT DeadEnd(void);

	void NextThink(float thinkTime, BOOL alwaysThink);

	int Classify(void);

	void CollisionDetection(void);
	void TerrainFollowing(void);
	void CheckTurning(void);

	void SetTrack(CPathTrack *track);
	void SetControls(entvars_t *pevControls);

	BOOL OnControls(entvars_t *pev);

	void StopSound(void);
	void UpdateSound(void);

public:

	static CFuncVehicle *Instance(edict_t *);

public:

	int Save(CSave &save); 
	int Restore(CRestore &restore);
	int ObjectCaps(void);
	void OverrideReset(void);
	void Restart(void);

public:

	TYPEDESCRIPTION     m_SaveData[12];        /*     0   192 */

public:	
	
	CPathTrack *        m_ppath;               /*   152     4 */
	float               m_length;              /*   156     4 */
	float               m_width;               /*   160     4 */
	float               m_height;              /*   164     4 */
	float               m_speed;               /*   168     4 */
	float               m_dir;                 /*   172     4 */
	float               m_startSpeed;          /*   176     4 */
	Vector              m_controlMins;         /*   180    12 */
	Vector              m_controlMaxs;         /*   192    12 */
	int                 m_soundPlaying;        /*   204     4 */
	int                 m_sounds;              /*   208     4 */
	int                 m_acceleration;        /*   212     4 */
	float               m_flVolume;            /*   216     4 */
	float               m_flBank;              /*   220     4 */
	float               m_oldSpeed;            /*   224     4 */
	int                 m_iTurnAngle;          /*   228     4 */
	float               m_flSteeringWheelDecay;/*   232     4 */
	float               m_flAcceleratorDecay;  /*   236     4 */
	float               m_flTurnStartTime;     /*   240     4 */
	float               m_flLaunchTime;        /*   244     4 */
	float               m_flLastNormalZ;       /*   248     4 */
	float               m_flCanTurnNow;        /*   252     4 */
	float               m_flUpdateSound;       /*   256     4 */
	Vector              m_vFrontLeft;          /*   260    12 */
	Vector              m_vFront;              /*   272    12 */
	Vector              m_vFrontRight;         /*   284    12 */
	Vector              m_vBackLeft;           /*   296    12 */
	Vector              m_vBack;               /*   308    12 */
	Vector              m_vBackRight;          /*   320    12 */
	Vector              m_vSurfaceNormal;      /*   332    12 */
	Vector              m_vVehicleDirection;   /*   344    12 */
	CBasePlayer *       m_pDriver;             /*   356     4 */

private:

	short unsigned int  m_usAdjustPitch;       /*   360     2 */


	/* vtable has 12 entries: {
	[0]  = Spawn
	[1]  = Precache
	[47] = Blocked
	[46] = Use
	[3]  = KeyValue
	[9]  = Classify
	[33] = OnControls
	[4]  = Save
	[5]  = Restore
	[6]  = ObjectCaps
	[28] = OverrideReset
	[2]  = Restart
	} */
	/* size: 364, cachelines: 6, members: 35 */
	/* padding: 2 */
	/* last cacheline: 44 bytes */
};

#endif // VEHICULE_H