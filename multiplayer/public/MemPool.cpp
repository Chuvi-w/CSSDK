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

#include <windows.h>
#include <math.h>
#include "MemPool.h"

CMemoryPool::CMemoryPool( int blockSize, int numElements )
{
    _blocksPerBlob = numElements;
    _blockSize     = blockSize;
    _numBlobs      = 0;
    _numElements   = 0;

    AddNewBlob();

    _peakAlloc       = 0;
    _blocksAllocated = 0;
}

CMemoryPool::~CMemoryPool( void )
{
    for( int i = 0; i < _numBlobs; i++ )
    {
        free( _memBlob[ i ] );
    }
}

void *CMemoryPool::Alloc( unsigned int amount )
{
    void *returnBlock;

    if( amount > (unsigned int )_blockSize)
    {
        return NULL;
    }

    _blocksAllocated++;
    _peakAlloc = max( _peakAlloc, _blocksAllocated );

    if( _blocksAllocated >= _numElements )
    {
        AddNewBlob();
    }

    if( !_headOfFreeList )
    {
        DebugBreak();
    }

    returnBlock = _headOfFreeList;
    _headOfFreeList = *( ( void ** )_headOfFreeList );

    return returnBlock;
}

void CMemoryPool::Free( void *memblock )
{
    if( !memblock )
    {
        return;
    }

    #if defined(_DEBUG)
        memset( memblock, 0xDD, _blockSize );
    #endif

    _blocksAllocated--;
    *( ( void** )memblock ) = _headOfFreeList;
    _headOfFreeList = memblock;
}

void CMemoryPool::AddNewBlob( void )
{
    int sizeMultiplier = pow( 2.0, _numBlobs );
    int nElements = _blocksPerBlob * sizeMultiplier;
    int blobSize  = nElements * _blockSize;

    _memBlob[ _numBlobs ] = malloc( blobSize );

    if( !_memBlob[ _numBlobs ] )
    {
        DebugBreak();
    }

    _headOfFreeList = _memBlob[ _numBlobs ];

    if( !_headOfFreeList )
    {
        DebugBreak();
    }

    void **newBlob = ( void ** )_headOfFreeList;

    for( int j = 0; j < nElements - 1; j++ )
    {
        newBlob[0] = ( char* )newBlob + _blockSize;
        newBlob = ( void** )newBlob[0];
    }

    newBlob[0] = NULL;

    _numElements += nElements;
    _numBlobs++;

    if( _numBlobs >= MAX_BLOBS - 1 )
    {
        DebugBreak();
    }
}