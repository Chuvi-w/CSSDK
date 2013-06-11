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

#ifndef MEMPOOL_H
#define MEMPOOL_H

#ifdef _WIN32
    #pragma once
#endif

class CMemoryPool
{
    public:

        CMemoryPool( int blockSize, int numElements );
        ~CMemoryPool( void );

        void *Alloc( unsigned int amount );
        void Free( void *memblock );
        int Count( void ) { return _blocksAllocated; }
        void AddNewBlob( void );

        enum
        {
            MAX_BLOBS = 16
        };

    private :

        int     _blockSize;             /*     0     4 */
        int     _blocksPerBlob;         /*     4     4 */
        int     _numElements;           /*     8     4 */
        void   *_memBlob[ MAX_BLOBS ];  /*    12    64 */
        void   *_headOfFreeList;        /*    76     4 */
        int     _numBlobs;              /*    80     4 */
        int     _peakAlloc;             /*    84     4 */
        int     _blocksAllocated;       /*    88     4 */

    /* size: 92, cachelines: 2, members: 8 */
    /* last cacheline: 28 bytes */
};

#endif
