/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/
//=========================================================
// Scheduling 
//=========================================================

#ifndef	SCHEDULE_H
#define	SCHEDULE_H

#define	bits_COND_SEE_HATE				( 1 << 1 ) // see something that you hate
#define bits_COND_SEE_FEAR				( 1 << 2 ) // see something that you are afraid of
#define bits_COND_SEE_DISLIKE			( 1 << 3 ) // see something that you dislike
#define bits_COND_SEE_ENEMY				( 1 << 4 ) // target entity is in full view.
#define bits_COND_LIGHT_DAMAGE			( 1 << 8 ) // hurt a little 
#define bits_COND_HEAVY_DAMAGE			( 1 << 9 ) // hurt a lot
#define bits_COND_SEE_CLIENT			( 1 << 21) // see a client
#define bits_COND_SEE_NEMESIS			( 1 << 22) // see my nemesis


#endif	// SCHEDULE_H
