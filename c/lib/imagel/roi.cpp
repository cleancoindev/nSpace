////////////////////////////////////////////////////////////////////////
//
//									ROI.CPP
//
//				Implementation of the image region of interest node.
//
////////////////////////////////////////////////////////////////////////

#include "imagel_.h"
#include <stdio.h>

// Globals

Roi :: Roi ( void )
	{
	////////////////////////////////////////////////////////////////////////
	//
	//	PURPOSE
	//		-	Constructor for the object
	//
	////////////////////////////////////////////////////////////////////////
	pSrc	= NULL;
	pDst	= NULL;
	bCopy	= false;
	}	// Roi

HRESULT Roi :: onAttach ( bool bAttach )
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
	HRESULT	hr = S_OK;

	// Attach
	if (bAttach)
		{
		adtValue		vL;

		// Defaults (optional)
		if (pnDesc->load ( adtString(L"Left"), vL ) == S_OK)
			iL = vL;
		if (pnDesc->load ( adtString(L"Top"), vL ) == S_OK)
			iT = vL;
		if (pnDesc->load ( adtString(L"Right"), vL ) == S_OK)
			iR = vL;
		if (pnDesc->load ( adtString(L"Bottom"), vL ) == S_OK)
			iB = vL;
		if (pnDesc->load ( adtString(L"Copy"), vL ) == S_OK)
			bCopy = vL;
		}	// if

	// Detach
	else
		{
		// Shutdown
		_RELEASE(pSrc);
		_RELEASE(pDst);
		}	// else

	return hr;
	}	// onAttach

HRESULT Roi :: receive ( IReceptor *pr, const WCHAR *pl, const ADTVALUE &v )
	{
	////////////////////////////////////////////////////////////////////////
	//
	//	PURPOSE
	//		-	The node has received a value on the specified receptor.
	//
	//	PARAMETERS
	//		-	pr is the receptor
	//		-	v is the value
	//
	//	RETURN VALUE
	//		S_OK if successful
	//
	////////////////////////////////////////////////////////////////////////
	HRESULT	hr = S_OK;

	// Execute
	if (_RCP(Fire))
		{
		IDictionary	*pSrcUse = NULL;
		cvMatRef		*pMatSrc	= NULL;
		cvMatRef		*pMatDst	= NULL;

		// State check
		CCLTRYE ( pDst != NULL, ERROR_INVALID_STATE );

		// Obtain image refence
		CCLTRY ( Prepare::extract ( pSrc, v, &pSrcUse, &pMatSrc ) );

		// Adjust Roi.  Could error out but be flexible.
		// Set limits so there is at least one pixel in each direction.
		if (hr == S_OK)
			{
			// Width
			if			(iL < 0)									iL = 0;
			else if	((int)iL >= pMatSrc->mat->cols)	iL = pMatSrc->mat->cols-1;
			if			(iR <= 0)								iR = 1;
			else if	((int)iR >= pMatSrc->mat->cols)	iR = pMatSrc->mat->cols;
			// Height
			if			(iT < 0)									iT = 0;
			else if	((int)iT >= pMatSrc->mat->rows)	iT = pMatSrc->mat->rows-1;
			if			(iB <= 0)								iB = 1;
			else if	((int)iB >= pMatSrc->mat->rows)	iB = pMatSrc->mat->rows;
			}	// if

		// Open CV uses exceptions
		try
			{
			// Create a new region of interest
			CCLTRYE( (pMatDst = new cvMatRef()) != NULL, E_OUTOFMEMORY );
			#if	CV_MAJOR_VERSION == 3
			CCLTRYE((pMatDst->mat = new cv::UMat ( *(pMatSrc->mat), cv::Rect(iL,iT,(iR-iL),(iB-iT)) ))
							!= NULL, E_OUTOFMEMORY);
			#else
			CCLTRYE((pMatDst->mat = new cv::Mat ( *(pMatSrc->mat), cv::Rect(iL,iT,(iR-iL),(iB-iT)) ))
							!= NULL, E_OUTOFMEMORY);
			#endif
			CCLTRY ( pDst->store (	adtString(L"cvMatRef"), adtIUnknown(pMatDst) ) );

			// Requesting own copy of Roi ?
			if (hr == S_OK && bCopy == true)
				*(pMatDst->mat) = pMatDst->mat->clone();
			}	// try
		catch ( cv::Exception ex )
			{
			lprintf ( LOG_ERR, L"OpenCV Exception" );
			hr = E_UNEXPECTED;
			}	// catch

		// Result
		if (hr == S_OK)
			_EMT(Fire,adtIUnknown(pDst));
		else
			_EMT(Error,adtInt(hr));

		// Clean up
		_RELEASE(pMatDst);
		_RELEASE(pMatSrc);
		_RELEASE(pSrcUse);
		}	// if

	// State
	else if (_RCP(Source))
		{
		adtIUnknown	unkV(v);
		_RELEASE(pSrc);
		_QISAFE(unkV,IID_IDictionary,&pSrc);
		}	// else if
	else if (_RCP(Destination))
		{
		adtIUnknown	unkV(v);
		_RELEASE(pDst);
		_QISAFE(unkV,IID_IDictionary,&pDst);
		}	// else if
	else if (_RCP(Left))
		iL = v;
	else if (_RCP(Right))
		iR = v;
	else if (_RCP(Top))
		iT = v;
	else if (_RCP(Bottom))
		iB = v;
	else
		hr = ERROR_NO_MATCH;

	return hr;
	}	// receive
