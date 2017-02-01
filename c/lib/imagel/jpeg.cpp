////////////////////////////////////////////////////////////////////////
//
//										JPEG.CPP
//
//				Implementation of the JPEG (de)compression utilities
//
//				Copyright (c) nSpace, LLC
//				All right reserved
//
////////////////////////////////////////////////////////////////////////

#include "imagel_.h"
#include <stdio.h>

#ifdef	_WIN32
extern "C" {
#define	HAVE_INT32
#include "../ext/libjpeg/jpeglib.h"
}
#elif		__unix__
#include <jpeglib.h>
#include	<sys/time.h>
#endif

// For JPEG library...
#include <setjmp.h>
/*
//
// Structure - CTX_JPEG.	JPEG compression/decompression context.  We
//		want a big enough buffer for compression so that we will most likely
//		not need to resize the destination byte stream.
//
#define	JCOMPRESS_BLKSIZE			0x2000

typedef struct
	{
	IMemoryMapped	*pBitsDst;							// Destination bits
	VOID				*pvBitsDst;							// Destination buffer
	U32				szDst;								// Destination size
	HRESULT			hr;									// Context result functions
	jmp_buf			ljenv;								// Long jump environment
	} CTX_JPEG;

// Prototypes
void		jpeg_error_exit				( j_common_ptr );
boolean	jpeg_fill_input_buffer		( j_decompress_ptr );
void		jpeg_init_destination		( j_compress_ptr );
void		jpeg_init_source				( j_decompress_ptr );
boolean	jpeg_empty_output_buffer	( j_compress_ptr );
void		jpeg_skip_input_data			( j_decompress_ptr, long );
void		jpeg_term_destination		( j_compress_ptr );
void		jpeg_term_source				( j_decompress_ptr );

ImgJPEG :: ImgJPEG ( void )
	{
	////////////////////////////////////////////////////////////////////////
	//
	//	PURPOSE
	//		-	Constructor for the node
	//
	////////////////////////////////////////////////////////////////////////
	pDict	= NULL;
	iQ		= 75;

	// JPEG
	pccinfo[0]	= pccinfo[1]	= NULL;
	pcjerr[0]	= pcjerr[1]		= NULL;
	pcdstmgr[0]	= pcdstmgr[1]	= NULL;
	}	// ImgJPEG

HRESULT ImgJPEG :: compress ( U32 width, U32 height, U32 bpp,
										IMemoryMapped *pBitsSrc,
										IMemoryMapped *pBitsDst )
	{
	////////////////////////////////////////////////////////////////////////
	//
	//	PURPOSE
	//		-	Compresses an uncompressed image.
	//
	//	PARAMETERS
	//		-	width,height are the dimensions of the image
	//		-	bpp is the bits per pixel.  If bpp = 8 it is assumed the image
	//			is gray scale.
	//		-	pBitsSrc is the source bit stream
	//		-	pBitsDst is the destination bit stream
	//
	//	RETURN VALUE
	//		S_OK if successful
	//
	////////////////////////////////////////////////////////////////////////
	HRESULT								hr				= S_OK;
	U8										*pcBitsSrc	= NULL;
	U8										*pcRowBits	= NULL;
	U32									szRow			= 0;
	U32									i				= (bpp == 8) ? 0 : 1;
	struct jpeg_compress_struct	*c	= (struct jpeg_compress_struct *)	pccinfo[i];
	struct jpeg_destination_mgr	*d	= (struct jpeg_destination_mgr *)	pcdstmgr[i];
	JSAMPROW								row_ptr[1];
	CTX_JPEG								ctx;
	U32									sz;

	// Access source bits
	CCLTRY( pBitsSrc->lock ( 0, 0, (VOID **) &pcBitsSrc, &sz ) );

	// Destinartion bits
	CCLOK ( memset ( &ctx, 0, sizeof(ctx) ); )
	CCLOK ( ctx.pBitsDst		= pBitsDst; )
	CCLOK ( ctx.hr				= S_OK; )
	CCLOK ( c->client_data	= &ctx; )

	// Image parameters
	CCLOK ( c->image_width	= width; )
	CCLOK ( c->image_height	= height; )

	// Initialize the destination manager.
	CCLOK ( d->next_output_byte	= NULL; )
	CCLOK	( d->free_in_buffer		= 0; )
	CCLOK	( szRow						= width*(bpp/8); )
	CCLOK	( pcRowBits					= pcBitsSrc; )
	CCLOK	( c->input_components	= (bpp/8); )

	// Very ugly, because of the way the JPEG library works we cannot return
	// after an 'jpeg_error_exit'.
	if (hr == S_OK)
		{
		// Install handler
		int sj = setjmp ( (ctx.ljenv) );
		switch ( sj )
			{
			// Zero just means environment saved, continue processing
			case 0 :

				// Compression
				CCLOK		( jpeg_start_compress ( c, TRUE ); )
				CCLTRYE	( (ctx.hr == S_OK), hr );
				while (hr == S_OK && c->next_scanline < c->image_height)
					{
					// Compress next row
					CCLOK ( row_ptr[0] = pcRowBits; )
					CCLOK ( jpeg_write_scanlines ( c, row_ptr, 1 ); )

					// Next row
					CCLTRYE	( (ctx.hr == S_OK), hr );
					CCLOK		( pcRowBits += szRow; )
					}	// while
				CCLOK ( jpeg_finish_compress ( c ); )

				// Set the final size of the compressed image
				CCLTRY ( ctx.pBitsDst->setSize ( ctx.szDst ) );

				// Perform long jump ourselves to clean up 'setjmp'
				longjmp ( (ctx.ljenv), 1 );
				break;

			// One means success, done
			case 1 :
				hr = S_OK;
				break;

			// Anything else is an error code
			default :
				hr = sj;
				break;
			}	// switch
		}	// if

	// Clean up
	_UNLOCK(pBitsDst,ctx.pvBitsDst);
	_UNLOCK(pBitsSrc,pcBitsSrc);

	return hr;
	}	// compress

HRESULT ImgJPEG :: construct ( void )
	{
	////////////////////////////////////////////////////////////////////////
	//
	//	OVERLOAD
	//	FROM		CCLObject
	//
	//	PURPOSE
	//		-	Called when the object is being created.
	//
	//	RETURN VALUE
	//		S_OK if successful
	//
	////////////////////////////////////////////////////////////////////////
	HRESULT								hr		= S_OK;
	struct jpeg_compress_struct	*c		= NULL;
	struct jpeg_decompress_struct	*d		= NULL;
	struct jpeg_error_mgr			*e		= NULL;
	struct jpeg_destination_mgr	*dst	= NULL;
	struct jpeg_source_mgr			*src	= NULL;
	U32									i;

	// Initialize JPEG structures
	for (i = 0;i < 2;++i)
		{
		// Allocate JPEG structures
		CCLTRYE	( (pccinfo[i]	= (U8 *) _ALLOCMEM(sizeof(struct jpeg_compress_struct))) != NULL,	E_OUTOFMEMORY );
		CCLOK		( memset ( pccinfo[i], 0, sizeof(struct jpeg_compress_struct) ); )
		CCLTRYE	( (pcdinfo[i]	= (U8 *) _ALLOCMEM(sizeof(struct jpeg_decompress_struct))) != NULL,	E_OUTOFMEMORY );
		CCLOK		( memset ( pcdinfo[i], 0, sizeof(struct jpeg_decompress_struct) ); )
		CCLTRYE	( (pcjerr[i]	= (U8 *) _ALLOCMEM(sizeof(struct jpeg_error_mgr))) != NULL,			E_OUTOFMEMORY );
		CCLOK		( memset ( pcjerr[i], 0, sizeof(struct jpeg_error_mgr) ); )
		CCLTRYE	( (pcdstmgr[i]	= (U8 *) _ALLOCMEM(sizeof(struct jpeg_destination_mgr))) != NULL,	E_OUTOFMEMORY );
		CCLOK		( memset ( pcdstmgr[i], 0, sizeof(struct jpeg_destination_mgr) ); )
		CCLTRYE	( (pcsrcmgr[i]	= (U8 *) _ALLOCMEM(sizeof(struct jpeg_source_mgr))) != NULL,	E_OUTOFMEMORY );
		CCLOK		( memset ( pcsrcmgr[i], 0, sizeof(struct jpeg_source_mgr) ); )

		// Grayscale/color
		CCLOK		( c		= (struct jpeg_compress_struct *)	pccinfo[i]; )
		CCLOK		( d		= (struct jpeg_decompress_struct *)	pcdinfo[i]; )
		CCLOK		( e		= (struct jpeg_error_mgr *)			pcjerr[i]; )
		CCLOK		( dst		= (struct jpeg_destination_mgr *)	pcdstmgr[i]; )
		CCLOK		( src		= (struct jpeg_source_mgr *)			pcsrcmgr[i]; )

		CCLOK		( c->err							= jpeg_std_error(e); )
		CCLOK		( e->error_exit				= jpeg_error_exit; )
		CCLOK		( jpeg_create_compress ( c ); )

		CCLOK		( c->input_components		= (!i) ? 1 : 3; )
		CCLOK		( c->in_color_space			= (!i) ? JCS_GRAYSCALE : JCS_RGB; )
		CCLOK		( jpeg_set_defaults ( c ); )

		CCLOK		( d->err = c->err; )
		CCLOK		( jpeg_create_decompress ( d ); )

		CCLOK		( dst->init_destination		= jpeg_init_destination; )
		CCLOK		( dst->empty_output_buffer	= jpeg_empty_output_buffer; )
		CCLOK		( dst->term_destination		= jpeg_term_destination; )
		CCLOK		( c->dest						= dst; )
		CCLOK		( jpeg_set_quality ( c, iQ, true ); )

		CCLOK		( src->init_source			= jpeg_init_source; )
		CCLOK		( src->fill_input_buffer	= jpeg_fill_input_buffer; )
		CCLOK		( src->skip_input_data		= jpeg_skip_input_data; )
		CCLOK		( src->resync_to_restart	= jpeg_resync_to_restart; )
		CCLOK		( src->term_source			= jpeg_term_source; )
		CCLOK		( d->src							= src; )
		}	// for

	// For Windows XP, etc using GDI+ for decompression of newer JPEG files
	#if defined(_WIN32) && !defined(UNDER_CE)
	{
	Gdiplus::GdiplusStartupInput	si;
	GdiplusStartup ( &uToken, &si, NULL );
	}
	#endif

	return hr;
	}	// construct

HRESULT ImgJPEG :: decompress (	IMemoryMapped *pBitsSrc,
											IMemoryMapped *pBitsDst,
											U32 *width, U32 *height, U32 *bpp )
	{
	////////////////////////////////////////////////////////////////////////
	//
	//	PURPOSE
	//		-	Decompresses an compressed image.
	//
	//	PARAMETERS
	//		-	pBitsSrc is the source bit stream
	//		-	pBitsDst is the destination bit stream
	//		-	width,height,bpp will receive the image parameters
	//
	//	RETURN VALUE
	//		S_OK if successful
	//
	////////////////////////////////////////////////////////////////////////
	HRESULT								hr				= S_OK;
	U8										*pcBitsSrc	= NULL;
	U8										*pcBitsDst	= NULL;
	U8										*pcRowBits	= NULL;
	U32									szRow			= 0;
	struct jpeg_decompress_struct	*d	= (struct jpeg_decompress_struct *)	pcdinfo[0];
	struct jpeg_source_mgr			*s	= (struct jpeg_source_mgr *)			pcsrcmgr[0];
	JSAMPROW								row_ptr[1];
	CTX_JPEG								ctx;
	U32									idx,sz;

	// Access and size source bits
	CCLTRY( pBitsSrc->lock ( 0, 0, (VOID **) &pcBitsSrc, &sz ) );

	// Initialize the source manager
	CCLOK ( s->next_input_byte = (JOCTET *) pcBitsSrc; )
	CCLOK ( s->bytes_in_buffer	= sz; )

	// Context for own routines
	CCLOK ( memset ( &ctx, 0, sizeof(ctx) ); )
	CCLOK ( ctx.hr				= S_OK; )
	CCLOK ( d->client_data	= &ctx; )

	// Very ugly, because of the way the JPEG library works we cannot return
	// after an 'jpeg_error_exit'.
	if (hr == S_OK)
		{
		// Install handler
		int sj = setjmp ( (ctx.ljenv) );
		switch ( sj )
			{
			// Zero just means environment saved, continue processing
			case 0 :
				// Header
				CCLOK		( jpeg_read_header ( d, TRUE ); )

				// Image information
				CCLOK		( *width		= d->image_width; )
				CCLOK		( *height	= d->image_height; )
				CCLOK		( *bpp		= d->num_components*8; )

				// Size and access destination bits.
				CCLOK		( szRow = (*width)*((*bpp)/8); )
				CCLTRY	( pBitsDst->setSize ( (*height)*szRow ) );
				CCLTRY	( pBitsDst->lock ( 0, 0, (VOID **) &pcBitsDst, NULL ) );
				CCLOK		( pcRowBits			= pcBitsDst; )

				// Decompress each row
				CCLOK		( jpeg_start_decompress ( d ); )
				CCLTRYE	( (ctx.hr == S_OK), hr );
				for (idx = 0;hr == S_OK && idx < d->image_height;++idx)
					{
					// Decompress next row
					CCLOK ( row_ptr[0] = pcRowBits; )
					if (hr == S_OK && jpeg_read_scanlines ( d, row_ptr, 1 ) != 1)
						{
						dbgprintf ( L"ImgJPEG::decompress:jpeg_read_scanlines failed at index %d\r\n", idx );
						hr = E_UNEXPECTED;
						}	// if

					// Next row
					CCLTRYE	( (ctx.hr == S_OK), hr );
					CCLOK		( pcRowBits += szRow; )
					}	// for
				CCLOK ( jpeg_finish_decompress ( d ); )

				// Perform long jump ourselves to clean up 'setjmp'
				longjmp ( (ctx.ljenv), 1 );
				break;

			// One means success, done
			case 1 :
				hr = S_OK;
				break;

			// Anything else is an error code
			default :
				jpeg_abort_decompress ( d );
				hr = sj;
				break;
			}	// switch
		}	// if

	// Clean up
//	jpeg_finish_decompress ( d );
	_UNLOCK(pBitsDst,pcBitsDst);
	_UNLOCK(pBitsSrc,pcBitsSrc);

	return hr;
	}	// decompress

void ImgJPEG :: destruct ( void )
	{
	////////////////////////////////////////////////////////////////////////
	//
	//	OVERLOAD
	//	FROM		CCLObject
	//
	//	PURPOSE
	//		-	Called when the object is being destroyed
	//
	////////////////////////////////////////////////////////////////////////
	U32	i;

	// Clean up
	for (i = 0;i < 2;++i)
		{
		if (pccinfo[i] != NULL)
			{
			jpeg_destroy_compress ( (struct jpeg_compress_struct *) pccinfo[i] );
			_FREEMEM(pccinfo[i]);
			}	// if
		if (pcdinfo[i] != NULL)
			{
			jpeg_destroy_decompress ( (struct jpeg_decompress_struct *) pcdinfo[i] );
			_FREEMEM(pcdinfo[i]);
			}	// if
		if (pcjerr[i] != NULL)		_FREEMEM(pcjerr[i]);
		if (pcdstmgr[i] != NULL)	_FREEMEM(pcdstmgr[i]);
		if (pcsrcmgr[i] != NULL)	_FREEMEM(pcsrcmgr[i]);
		}	// for

	_RELEASE(pDict);

	// For Windows XP, etc using GDI+ for decompression of newer JPEG files
	#if defined(_WIN32) && !defined(UNDER_CE)
	Gdiplus::GdiplusShutdown(uToken);
	#endif
	}	// destruct

HRESULT ImgJPEG :: onAttach ( bool bAttach )
	{
	////////////////////////////////////////////////////////////////////////
	//
	//	PURPOSE
	//		-	Called when this behaviour is assigned to a node
	//
	//	PARAMETERS
	//		-	bAttach is true for attachment, false for detachment.
	//
	//	RETURN VALUE
	//		S_OK if successful
	//
	////////////////////////////////////////////////////////////////////////

	// State check
	if (!bAttach) return S_OK;

	// Default states
	pnAttr->load ( strRefQuality, iQ );

	return S_OK;
	}	// onAttach

HRESULT ImgJPEG :: receive ( IReceptor *pR, const adtValue &v )
	{
	////////////////////////////////////////////////////////////////////////
	//
	//	PURPOSE
	//		-	The node has received a value on the specified receptor.
	//
	//	PARAMETERS
	//		-	pR is the receptor
	//		-	v is the value
	//
	//	RETURN VALUE
	//		S_OK if successful
	//
	////////////////////////////////////////////////////////////////////////
	HRESULT	hr = S_OK;

	// Compress
	if (prC == pR)
		{
		IMemoryMapped	*pBitsSrc	= NULL;
		IMemoryMapped	*pBitsDst	= NULL;
		adtString		strFmt;
		adtIUnknown		unkV;
		adtInt			iWidth,iHeight,iBpp;

		// State check
		CCLTRYE	( pDict != NULL,	ERROR_INVALID_STATE );

		// Validate format
		if (hr == S_OK && pDict->load ( adtString(L"Format"), strFmt ) == S_OK)
			hr = (!WCASECMP(L"RGB",strFmt)) ? S_OK : E_INVALIDARG;

		// Image parameters
		CCLTRY	( pDict->load ( strRefWidth, iWidth ) );
		CCLTRY	( pDict->load ( strRefHeight, iHeight ) );
		CCLTRY	( pDict->load ( strRefBpp, iBpp ) );
		CCLTRY	( pDict->load ( strRefBits, unkV ) );
		CCLTRY	( _QISAFE(unkV,IID_IMemoryMapped,&pBitsSrc) );

		// JPEG library only supports 8 bit grayscale or 24 bit color
		CCLTRYE	( (iBpp == 8 || iBpp == 24), E_INVALIDARG );

		// Create destination memory
		CCLTRY ( COCREATEINSTANCE ( CLSID_MemoryBlock, IID_IMemoryMapped, &pBitsDst ) );

		// Compress
		if (hr == S_OK)
			{
			// Debug
			#ifdef	_WIN32
			U32		now,then;
			then	= 	GetTickCount();
			#endif
			#ifdef	__unix__
	//		struct timeval now,then;
	//		gettimeofday ( &then, NULL );
			#endif

			// Compress
			hr = compress ( iWidth, iHeight, iBpp, pBitsSrc, pBitsDst );

			// Debug
			#ifdef	_WIN32
			now = GetTickCount();
	//		dbgprintf ( L"ImgJPEG::receiveCompress:Time to compress %d ms, hr 0x%x\n", (now-then), hr );
			#endif
			#ifdef	__unix__
	//		gettimeofday ( &now, NULL );
	//		dbgprintf (	L"Compress %d ms\n",
	//						((now.tv_usec-then.tv_usec)/1000) + ((now.tv_sec-then.tv_sec)*1000) );
			#endif
			}	// if

		// Format
		CCLTRY ( pDict->store ( adtString(L"Format"), adtString(L"JPEG") ) );

		// Update  image properties and emit
		CCLTRY (	pDict->store ( adtString(L"Bits"), adtIUnknown(pBitsDst) ));
		CCLOK	 ( peC->emit ( adtIUnknown(pDict) ); )

		// Clean up
		_RELEASE(pBitsDst);
		_RELEASE(pBitsSrc);
		}	// if

	// Decompress
	else if (prD == pR)
		{
		IMemoryMapped		*pBitsSrc	= NULL;
		IMemoryMapped		*pBitsDst	= NULL;
		U32					uWidth		= 0;
		U32					uHeight		= 0;
		U32					uStride		= 0;
		U32					uBpp			= 0;
		adtIUnknown			unkV;

		// State check
		CCLTRYE	( pDict != NULL,	ERROR_INVALID_STATE );
		CCLTRY	( pDict->load ( strRefBits, unkV ) );
		CCLTRY	( _QISAFE(unkV,IID_IMemoryMapped,&pBitsSrc) );

		// Create memory block to receive the decompressed memory block
		CCLTRY ( COCREATEINSTANCE ( CLSID_MemoryBlock, IID_IMemoryMapped, &pBitsDst ) );

		// Decompress
		if (hr == S_OK)
			{
			// Debug
			#ifdef	_WIN32
			U32		now,then;
			then	= 	GetTickCount();
			#endif

			// The current 'jpeg6b' library cannot handle decompression of some newer libraries.
			// So for decompression perform platform specific algorithms.
			#if	defined(_WIN32)

			// Windows NT,etc
			#if		!defined(UNDER_CE)
			IByteStream				*pStmByte	= NULL;
			IStream					*pStm			= NULL;
			IHaveValue				*pValue		= NULL;
			void						*pvDstBits	= NULL;
			Gdiplus::Bitmap		*jpeg			= NULL;
			Gdiplus::BitmapData	bmp;

			// Setup
			bmp.Scan0	= NULL;

			// Since GDI+ needs an 'IStream' to load an image from memory, lay a
			// byte stream in front of the memory block then lay an IStream interface
			// on top of that
			CCLTRY ( COCREATEINSTANCE ( CLSID_SysStmOnByteStm, IID_IHaveValue, &pValue ) );
			CCLTRY ( pBitsSrc->stream ( &pStmByte ) );
			CCLTRY ( pValue->setValue ( adtIUnknown(pStmByte) ) );
			CCLTRY ( _QI(pValue,IID_IStream,&pStm) );

			// Construct a bitmap object from the provided JPEG stream
			CCLTRYE ( (jpeg = Gdiplus::Bitmap::FromStream ( pStm, FALSE )) != NULL,
							E_UNEXPECTED );

			// Lock bits so they can be copied into the destination memory block
			CCLOK		( uWidth		= jpeg->GetWidth(); )
			CCLOK		( uHeight	= jpeg->GetHeight(); )

			// Access image bits
			if (hr == S_OK)
				{
				// Entire area
				Gdiplus::Rect rect ( 0, 0, uWidth, uHeight );

				// Lock in read-only mode
				hr = (jpeg->LockBits ( &rect, Gdiplus::ImageLockModeRead, PixelFormat24bppRGB, &bmp ) == Gdiplus::Ok)
						? S_OK : E_UNEXPECTED;

				// Default to 24bpp
				CCLOK ( uBpp = 24; )
				}	// if
			CCLTRYE ( (bmp.Scan0 != NULL),	E_UNEXPECTED );
			CCLTRYE ( (bmp.Stride > 0),		E_UNEXPECTED );

			// Set size of destination block and copy bits
			CCLTRY ( pBitsDst->setSize ( bmp.Stride*bmp.Height ) );
			CCLTRY ( pBitsDst->lock ( 0, 0, &pvDstBits, NULL ) );
			if (hr == S_OK)
				{
				for (U32 h = 0,idx = 0;h < bmp.Height;++h,idx+=bmp.Stride)
					memcpy ( &(((U8 *)(pvDstBits))[idx]), &(((U8 *)(bmp.Scan0))[idx]), bmp.Stride );
				}	// if
			CCLOK	 ( uStride = bmp.Stride; )

			// Clean up
			_UNLOCK(pBitsDst,pvDstBits);
			if (jpeg != NULL)
				{
				if (bmp.Scan0 != NULL)
					jpeg->UnlockBits ( &bmp );
				delete jpeg;
				}	// if
			_RELEASE(pStm);
			_RELEASE(pStmByte);
			_RELEASE(pValue);

			// Windows CE
			#else
//			#error	"Implement this"
			// Decompress
			hr = decompress ( pBitsSrc, pBitsDst, &uWidth, &uHeight, &uBpp );
			#endif

			#else
			// Decompress
			hr = decompress ( pBitsSrc, pBitsDst, &uWidth, &uHeight, &uBpp );
			#endif

			// Debug
			#ifdef	_WIN32
			now = GetTickCount();
	//		dbgprintf ( L"ImgJPEG::receiveDecompress:Time to decompress %d ms, hr 0x%x\n", (now-then), hr );
			#endif
			}	// if

		// Image information
		CCLTRY ( pDict->store ( strRefWidth,	adtInt(uWidth) ) );
		CCLTRY ( pDict->store ( strRefHeight,	adtInt(uHeight) ) );
		CCLTRY ( pDict->store ( strRefStride,	adtInt(uStride) ) );
		CCLTRY ( pDict->store ( strRefBpp,		adtInt(uBpp) ) );
		CCLTRY ( pDict->store ( strRefFormat,	adtString(L"RGB") ) );

		// Update image bits
		CCLTRY ( pDict->store ( adtString(L"Bits"), adtIUnknown(pBitsDst) ));

		// Result
		if (hr == S_OK)
			peD->emit ( adtIUnknown(pDict) );
		else
			peErr->emit ( adtIUnknown(pDict) );

		// Clean up
		_RELEASE(pBitsDst);
		_RELEASE(pBitsSrc);
		}	// else if

	// State
	else if (prImg == pR)
		{
		adtIUnknown	unkV(v);
		_RELEASE(pDict);
		hr = _QISAFE(unkV,IID_IADTDictionary,&pDict);
		}	// else if
	else if (prQ == pR)
		{
		adtValueImpl::copy ( iQ, adtInt(v) );

		// Reset quality on compression structures
		jpeg_set_quality ( (j_compress_ptr) pccinfo[0], iQ, true );
		jpeg_set_quality ( (j_compress_ptr) pccinfo[1], iQ, true );
		}	// else if

	return hr;
	}	// receive

//
// JPEG support routines
//

boolean jpeg_empty_output_buffer ( j_compress_ptr pc )
	{
	////////////////////////////////////////////////////////////////////////
	//
	//	PURPOSE
	//		-	Called when the destination buffer fills up.
	//
	//	PARAMETERS
	//		-	pc is the compression information
	//
	//	RETURN VALUE
	//		TRUE if successsful
	//
	////////////////////////////////////////////////////////////////////////
	CTX_JPEG				*ctx	= (CTX_JPEG *) (pc->client_data);

	// Unlock current block
	if (ctx->hr == S_OK)
		{
		_UNLOCK(ctx->pBitsDst,ctx->pvBitsDst);
		}	// if

	// Another block has been used
	if (ctx->hr == S_OK) ctx->szDst += JCOMPRESS_BLKSIZE;

	// Resize for the next block
	if (ctx->hr == S_OK)
		ctx->hr = ctx->pBitsDst->setSize ( ctx->szDst + JCOMPRESS_BLKSIZE );

	// Acquire ptr. to next block
	if (ctx->hr == S_OK)
		ctx->hr = ctx->pBitsDst->lock ( ctx->szDst, JCOMPRESS_BLKSIZE,
													&(ctx->pvBitsDst), NULL );

	// Reset JPEG info.
	pc->dest->next_output_byte	= (JOCTET *) ctx->pvBitsDst;
	pc->dest->free_in_buffer	= JCOMPRESS_BLKSIZE;

	return (ctx->hr == S_OK);
	}	// jpeg_empty_output_buffer

void jpeg_error_exit ( j_common_ptr pc )
	{
	////////////////////////////////////////////////////////////////////////
	//
	//	PURPOSE
	//		-	Called when a fatal error is detected by the library.
	//
	//	PARAMETERS
	//		-	pc is the compression information
	//
	////////////////////////////////////////////////////////////////////////
	CTX_JPEG		*ctx	= (CTX_JPEG *) (pc->client_data);
	char			buffer[JMSG_LENGTH_MAX];
	adtString	sBfr;

	// Error message
	pc->err->format_message ( pc, buffer );
	dbgprintf	( L"jpeg_error_exit:" );
	sBfr = buffer;
	dbgprintf ( sBfr );
	dbgprintf ( L"\n" );

	// Exit
	longjmp ( ctx->ljenv, E_UNEXPECTED );
	}	// jpeg_error_exit

boolean jpeg_fill_input_buffer ( j_decompress_ptr pd )
	{
	////////////////////////////////////////////////////////////////////////
	//
	//	PURPOSE
	//		-	Called when library runs out of source data.
	//
	//	PARAMETERS
	//		-	pd is the decompression information
	//
	//	RETURN VALUE
	//		Nonzero if successful
	//
	////////////////////////////////////////////////////////////////////////
	// All data is available at start, error if this function called
	return 0;
	}	// jpeg_fill_input_buffer

void jpeg_init_destination ( j_compress_ptr pc )
	{
	////////////////////////////////////////////////////////////////////////
	//
	//	PURPOSE
	//		-	Called by jpeg_start_compress to initialize the destination
	//			buffers.
	//
	//	PARAMETERS
	//		-	pc is the compression information
	//
	////////////////////////////////////////////////////////////////////////
	CTX_JPEG				*ctx	= (CTX_JPEG *) (pc->client_data);

	// Pre-size and access destination bits/context
	if (ctx->hr == S_OK)
		ctx->hr = ctx->pBitsDst->setSize ( JCOMPRESS_BLKSIZE );
	if (ctx->hr == S_OK)
		ctx->hr = ctx->pBitsDst->lock ( 0, JCOMPRESS_BLKSIZE, &(ctx->pvBitsDst), NULL );
	if (ctx->hr == S_OK)
		ctx->szDst = 0;

	// Available output
	pc->dest->next_output_byte	= (JOCTET *) ctx->pvBitsDst;
	pc->dest->free_in_buffer	= JCOMPRESS_BLKSIZE;

	}	// jpeg_init_destination

void jpeg_init_source ( j_decompress_ptr pd )
	{
	////////////////////////////////////////////////////////////////////////
	//
	//	PURPOSE
	//		-	Called by jpeg_start_decompress to initialize the source buffers
	//
	//	PARAMETERS
	//		-	pd is the decompression information
	//
	////////////////////////////////////////////////////////////////////////
	}	// jpeg_init_source

void jpeg_skip_input_data ( j_decompress_ptr pd, long nskip )
	{
	////////////////////////////////////////////////////////////////////////
	//
	//	PURPOSE
	//		-	Called to skip over some of the input data.
	//
	//	PARAMETERS
	//		-	pd is the decompression information
	//		-	nskip is the # of bytes to skip
	//
	////////////////////////////////////////////////////////////////////////
	}	// jpeg_skip_input_data

void jpeg_term_destination ( j_compress_ptr pc )
	{
	////////////////////////////////////////////////////////////////////////
	//
	//	PURPOSE
	//		-	Called by jpeg_finish_compress to flush buffers.
	//
	//	PARAMETERS
	//		-	pc is the compression information
	//
	////////////////////////////////////////////////////////////////////////
	CTX_JPEG				*ctx	= (CTX_JPEG *) (pc->client_data);

	// Unlock current block
	if (ctx->hr == S_OK)
		{
		_UNLOCK(ctx->pBitsDst,ctx->pvBitsDst);
		}	// if

	// Part of another block has been used
	if (ctx->hr == S_OK) ctx->szDst += (U32)(JCOMPRESS_BLKSIZE-pc->dest->free_in_buffer);

	}	// jpeg_term_destination

void jpeg_term_source ( j_decompress_ptr pd )
	{
	////////////////////////////////////////////////////////////////////////
	//
	//	PURPOSE
	//		-	Called by jpeg_finish_decompress to flush buffers.
	//
	//	PARAMETERS
	//		-	pd is the decompression information
	//
	////////////////////////////////////////////////////////////////////////
	}	// jpeg_term_source
*/
