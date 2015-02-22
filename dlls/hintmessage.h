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
#ifndef HINTMESSAGE_H
#define HINTMESSAGE_H

#include "../public/UtlVector.h"

#define Hint_press_buy_to_purchase              ( 1<<1 )
#define Hint_prevent_hostage_rescue             ( 1<<2 )
#define Hint_rescue_the_hostages                ( 1<<2 )
#define Hint_press_use_so_hostage_will_follow   ( 1<<3 )
#define Hint_lead_hostage_to_rescue_point       ( 1<<4 )
#define Hint_careful_around_hostages            ( 1<<5 )
#define Hint_lost_money                         ( 1<<6 )
#define Hint_spotted_a_friend                   ( 1<<7 )
#define Hint_spotted_an_enemy                   ( 1<<8 )
#define Hint_try_not_to_injure_teammates        ( 1<<9 )
#define Hint_careful_around_teammates           ( 1<<10 )
#define Hint_win_round_by_killing_enemy         ( 1<<11 )
#define Hint_you_have_the_bomb                  ( 1<<12 )
#define Hint_out_of_ammo                        ( 1<<15 )
#define Hint_you_are_in_targetzone              ( 1<<16 )
#define Hint_hostage_rescue_zone                ( 1<<17 )
#define Hint_terrorist_escape_zone				( 1<<18 )
#define Hint_ct_vip_zone                        ( 1<<19 )
#define Hint_terrorist_vip_zone                 ( 1<<19 )
#define Hint_Only_CT_Can_Move_Hostages          ( 1<<21 )
#define Hint_Spec_Duck                          ( 1<<22 )

class CBaseEntity;
class CBasePlayer;

class CHintMessage
{
public:

	CHintMessage(const char *hintString, bool isHint, CUtlVector< const char* > *args, float duration);
	~CHintMessage(void);

	float GetDuration(void) const { return m_duration; }
	void Send(CBaseEntity *client);

public:

	const char                 *m_hintString;         /*     0     4 */
	bool                        m_isHint;             /*     4     1 */
	CUtlVector< char* >         m_args;               /*     8    20 */
	float                       m_duration;           /*    28     4 */

	/* size: 32, cachelines: 1, members: 4 */
	/* sum members: 29, holes: 1, sum holes: 3 */
	/* last cacheline: 32 bytes */
};

class CHintMessageQueue
{
public:

	void Reset(void);
	void Update(CBaseEntity *client);
	bool AddMessage(const char *message, float duration, bool isHint, CUtlVector< const char* > *args);
	bool IsEmpty() { return m_messages.Count() == 0; }

private:

	float                           m_tmMessageEnd;     /*     0     4 */
	CUtlVector< CHintMessage* >     m_messages;         /*     4    20 */

	/* size: 24, cachelines: 1, members: 2 */
	/* last cacheline: 24 bytes */
};

#endif // HINTMESSAGE_H
