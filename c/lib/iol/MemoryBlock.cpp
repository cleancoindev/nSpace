////////////////////////////////////////////////////////////////////////
//
//									MEMBLCK.CPP
//
//					Implementation of the memory block object
//
////////////////////////////////////////////////////////////////////////

#define	INITGUID
#include "iol_.h"
#include <stdio.h>

#define	MEMBLOCK_DEFBLKSIZE		0x00400			// 'In process' memory (1K)
#define	MEMBLOCK_BIGBLKSIZE		0x10000			// 'File mapped' memory (64K)

MemoryBlock :: MemoryBlock ( void )
	{
	////////////////////////////////////////////////////////////////////////
	//
	//	OVERLOAD
	//	FROM		IMemoryMapped
	//
	//	PURPOSE
	//		-	Constructor for the object.
	//
	////////////////////////////////////////////////////////////////////////
	pcBlk		= NULL;
	szBlk		= 0;
	szAllocd	= 0;
	hMap		= NULL;
	}	// MemoryBlock

HRESULT MemoryBlock :: clone ( IUnknown **ppUnk )
	{
	////////////////////////////////////////////////////////////////////////
	//
	//	OVERLOAD
	//	FROM		ICloneable
	//
	//	PURPOSE
	//		-	Clones the object.
	//
	//	PARAMETERS
	//		-	ppUnk will receive the cloned object
	//
	//	RETURN VALUE
	//		S_OK if successful
	//
	////////////////////////////////////////////////////////////////////////
	HRESULT		hr			= S_OK;
	MemoryBlock	*pDst		= NULL;
	void			*pvDst	= NULL;

	// Cloning a memory block creates a copy of the data.

	// Create another instance of this object
	CCLTRYE	( (pDst = new MemoryBlock()) != NULL, E_OUTOFMEMORY );
	CCLOK		( pDst->AddRef(); )
	CCLTRY	( pDst->construct(); )

	// Valid size ?
	if (hr == S_OK && szBlk > 0)
		{
		// Copy bits
		CCLTRY	( pDst->setSize ( szBlk ) );
		CCLTRY	( pDst->lock ( 0, 0, &pvDst, NULL ) );
		CCLOK		( memcpy ( pvDst, pcBlk, szBlk ); )
		_UNLOCK  ( pDst, pvDst );
		}	// if

	// Result
	if (hr == S_OK)
		{
		(*ppUnk) = (IMemoryMapped *) pDst;
		_ADDREF((*ppUnk));
		}	// if

	// Clean up
	_RELEASE(pDst);

	return hr;
	}	// clone

void MemoryBlock :: destruct ( void )
	{
	////////////////////////////////////////////////////////////////////////
	//
	//	PURPOSE
	//		-	Called when the object is being destroyed.
	//
	////////////////////////////////////////////////////////////////////////
	if (hMap == NULL)
		{
		_FREEMEM(pcBlk);
		}	// if
	else
		{
		#if	defined(_WIN32)
		if (pcBlk != NULL) UnmapViewOfFile ( pcBlk );
		CloseHandle ( hMap );
		#endif
		}	// else

	}	// destruct

HRESULT MemoryBlock :: getSize ( U32 *psz )
	{
	////////////////////////////////////////////////////////////////////////
	//
	//	OVERLOAD
	//	FROM		IMemoryMapped
	//
	//	PURPOSE
	//		-	Returns the allocated size of the block.
	//
	//	PARAMETERS
	//		-	psz will receive the size
	//
	//	RETURN VALUE
	//		S_OK if successful
	//
	////////////////////////////////////////////////////////////////////////

	// Size
	*psz	= szBlk;

	return S_OK;
	}	// getSize

HRESULT MemoryBlock :: lock ( U32 uOff, U32 uSz, void **ppv, U32 *puSz )
	{
	////////////////////////////////////////////////////////////////////////
	//
	//	OVERLOAD
	//	FROM		IMemoryMapped
	//
	//	PURPOSE
	//		-	Locks access to a specific region of the block.
	//
	//	PARAMETERS
	//		-	uOff is the offset from the beginning
	//		-	uSz is the size of the window (0 = all)
	//		-	ppv will receive the ptr.
	//		-	puSz is the size of the block
	//
	//	RETURN VALUE
	//		S_OK if successful
	//
	////////////////////////////////////////////////////////////////////////
	HRESULT				hr				= S_OK;

	// Setup
	if (ppv != NULL) (*ppv)	= NULL;

	// Valid block ?
	CCLTRYE	( (pcBlk != NULL), E_UNEXPECTED );

	// Valid offset ?
	CCLTRYE	( (uOff < szBlk), E_INVALIDARG );

	// If size specified, valid ?
	CCLTRYE ( !uSz || (uOff+uSz <= szBlk), E_INVALIDARG );

	// Access location
	if (hr == S_OK && ppv != NULL)
		*ppv = &(pcBlk[uOff]);

	// Correct size
	if (puSz != NULL)	*puSz = (hr == S_OK) ? szBlk : 0;

	return hr;
	}	// lock

HRESULT MemoryBlock :: setSize ( U32 sz )
	{
	////////////////////////////////////////////////////////////////////////
	//
	//	OVERLOAD
	//	FROM		IMemoryMapped
	//
	//	PURPOSE
	//		-	Specifies what size the memory block should be sized too.
	//
	//	PARAMETERS
	//		-	sz is the new size
	//
	//	RETURN VALUE
	//		S_OK if successful
	//
	////////////////////////////////////////////////////////////////////////
	HRESULT	hr		= S_OK;

	// Need more room ?
	if (sz > szAllocd)
		{
		U8	*pcBlkNew = NULL;

		// Allocate in blocks
		szAllocd = ((sz/MEMBLOCK_DEFBLKSIZE)+1)*MEMBLOCK_DEFBLKSIZE;

		// Reallocate buffer
		CCLTRYE ( (pcBlkNew = (U8 *) _REALLOCMEM ( (hMap == NULL) ? pcBlk : NULL, 
						szAllocd )) != NULL, E_OUTOFMEMORY );

		// New ptr.
		pcBlk = pcBlkNew;

		// Debug
//		dbgprintf ( L"MemoryBlock::setSize:%d:%p\r\n", sz, pcBlk );
		}	// if

	// New information
	szBlk = (hr == S_OK)	? sz : 0;

	return hr;
	}	// setSize

HRESULT MemoryBlock :: stream ( IByteStream **ppStm )
	{
	////////////////////////////////////////////////////////////////////////
	//
	//	OVERLOAD
	//	FROM		IMemoryMapped
	//
	//	PURPOSE
	//		-	Creates a byte stream in front of the memory block.
	//
	//	PARAMETERS
	//		-	ppStm is the new stream.
	//
	//	RETURN VALUE
	//		S_OK if successful
	//
	////////////////////////////////////////////////////////////////////////
	HRESULT			hr		= S_OK;
	StmMemory		*pStm	= NULL;

	// Setup
	(*ppStm) = NULL;

	// Create new stream and assign this object as the block
	CCLTRYE	( (pStm = new StmMemory()) != NULL, E_OUTOFMEMORY );
	CCLOK		( pStm->AddRef(); )
	CCLOK		( pStm->pBlock = this; )
	CCLTRY	( pStm->construct(); )

	// Done
	if (hr == S_OK)
		{
		(*ppStm) = pStm;
		_ADDREF((*ppStm));
		}	// if

	// Clean up
	_RELEASE(pStm);

	return hr;
	}	// stream

HRESULT MemoryBlock :: unlock ( void *pv )
	{
	////////////////////////////////////////////////////////////////////////
	//
	//	OVERLOAD
	//	FROM		IMemoryMapped
	//
	//	PURPOSE
	//		-	Unlocks a previously locked section.
	//
	//	PARAMETERS
	//		-	pv is the section
	//
	//	RETURN VALUE
	//		S_OK if successful
	//
	////////////////////////////////////////////////////////////////////////
	return S_OK;
	}	// unlock

		/*
		// NOTE: For Windows CE each process is limited to 32 MB.  Only way
		// around this limitation is to use file-mapping objects.  For 'large'
		// allocations, put memory outside process.  'Large' is arbitrary.
		// WARNING: Resizing large blocks slow due to non-resizable map objects.
		// TODO: Makes sense to do this for large blocks for 'any' OS ?
		#if	defined(_WIN32)
		if (sz >= MEMBLOCK_BIGBLKSIZE)
			{
			HANDLE	hMapNew		= NULL;
			U8			*pcBlkNew	= NULL;

			// Allocate in blocks
			szAllocd = ((sz/MEMBLOCK_BIGBLKSIZE)+1)*MEMBLOCK_BIGBLKSIZE;

			// Create a new file mapping backed by memory
			CCLTRYE ( (hMapNew = CreateFileMapping ( INVALID_HANDLE_VALUE,
							NULL, PAGE_READWRITE, 0, szAllocd, NULL ))
							!= NULL, GetLastError() );

			// Map a view info the file
			CCLTRYE ( (pcBlkNew = (U8 *) MapViewOfFile ( hMapNew,
							FILE_MAP_WRITE, 0, 0, 0 )) != NULL, GetLastError() );

			// If previous buffer exists, copy into new buffer
			if (hr == S_OK && pcBlk != NULL && szBlk)
				memcpy ( pcBlkNew, pcBlk, szBlk );

			// Release previous memory based on type
			if (hMap == NULL)
				{
				_FREEMEM(pcBlk);
				}	// if
			else
				{
				if (pcBlk != NULL) { UnmapViewOfFile ( pcBlk ); pcBlk = NULL; }
				CloseHandle ( hMap );
				hMap = NULL;
				}	// else

			// New block information
			if (hr == S_OK)
				{
				hMap	= hMapNew;
				pcBlk	= pcBlkNew;
				}	// if
			}	// if
		else
			{
			#endif

			#if	defined(_WIN32)
			// If decreasing in size from a large memory mapped file, release it
			if (hr == S_OK && hMap != NULL)
				{
				// Keep contents of block
				if (pcBlk != NULL)
					{
					if (szBlk) memcpy ( pcBlkNew, pcBlk, szBlk );
					UnmapViewOfFile ( pcBlk );
					}	// if
				CloseHandle ( hMap );
				hMap	= NULL;
				}	// if
			#endif
			#if	defined(_WIN32)	
			}	// else
			#endif


*/