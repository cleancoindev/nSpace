////////////////////////////////////////////////////////////////////////
//
//									NORMAL.CPP
//
//				Implementation of the image normalization node.
//
////////////////////////////////////////////////////////////////////////

#include "imagel_.h"
#include <stdio.h>

// Globals

Normalize :: Normalize ( void )
	{
	////////////////////////////////////////////////////////////////////////
	//
	//	PURPOSE
	//		-	Constructor for the object
	//
	////////////////////////////////////////////////////////////////////////
	pImg	= NULL;
	adtValue::copy ( adtInt(0), vFrom );
	adtValue::copy ( adtInt(0), vTo );
	}	// Normalize

HRESULT Normalize :: onAttach ( bool bAttach )
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
		if (pnDesc->load ( adtString(L"From"), vL ) == S_OK)
			adtValue::copy ( vL, vFrom );
		if (pnDesc->load ( adtString(L"To"), vL ) == S_OK)
			adtValue::copy ( vL, vTo );
		}	// if

	// Detach
	else
		{
		// Shutdown
		_RELEASE(pImg);
		}	// else

	return hr;
	}	// onAttach

HRESULT Normalize :: receive ( IReceptor *pr, const WCHAR *pl, const ADTVALUE &v )
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
		IDictionary	*pImgUse = pImg;
		cv::Mat		*pMat		= NULL;
		adtValue		vL;

		// Image to use
		if (pImgUse == NULL)
			{
			adtIUnknown unkV(v);
			CCLTRY(_QISAFE(unkV,IID_IDictionary,&pImgUse));
			}	// if
		else
			pImgUse->AddRef();

		// Image must be 'uploaded'
		CCLTRY ( pImgUse->load (	adtString(L"cv::Mat"), vL ) );
		CCLTRYE( (pMat = (cv::Mat *)(U64)adtLong(vL)) != NULL,
					ERROR_INVALID_STATE );

		// Perform operation
		CCLOK ( cv::normalize ( *pMat, *pMat, adtDouble(vFrom), adtDouble(vTo), cv::NORM_MINMAX ); )

		// Result
		if (hr == S_OK)
			_EMT(Fire,adtIUnknown(pImgUse));
		else
			_EMT(Error,adtInt(hr));

		// Clean up
		_RELEASE(pImgUse);
		}	// if

	// State
	else if (_RCP(Image))
		{
		adtIUnknown	unkV(v);
		_RELEASE(pImg);
		_QISAFE(unkV,IID_IDictionary,&pImg);
		}	// else if
	else
		hr = ERROR_NO_MATCH;

	return hr;
	}	// receive
