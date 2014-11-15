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
// nodes.h
//=========================================================

#ifndef	NODES_H
#define	NODES_H

#define	bits_NODE_GROUP_REALM	1

class CLink
{
public:
	entvars_t	*m_pLinkEnt;// the entity that blocks this connection (doors, etc)
};


class CGraph
{
public:
	BOOL	m_fGraphPresent;// is the graph in memory?
	BOOL	m_fGraphPointersSet;// are the entity pointers for the graph all set?

	int		m_cLinks;// total number of links
	CLink	*m_pLinkPool;// big list of all node connections

	void	InitGraph( void );
	int		AllocNodes ( void );
	
	int		CheckNODFile(char *szMapName);
	int		FLoadGraph(char *szMapName);
	int		FSetGraphPointers(void);
	void	ShowNodeConnections ( int iNode );
	int		FindNearestNode ( const Vector &vecOrigin, CBaseEntity *pEntity );
	int		FindNearestNode ( const Vector &vecOrigin, int afNodeTypes );

};

extern CGraph WorldGraph;

#endif	// NODES_H
