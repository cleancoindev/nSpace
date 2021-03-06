////////////////////////////////////////////////////////////////////////
//
//									CONTOURS.CPP
//
//				Implementation of the image contours node.
//
////////////////////////////////////////////////////////////////////////

#include "imagel_.h"
#include <stdio.h>

// Globals

Contours :: Contours ( void )
	{
	////////////////////////////////////////////////////////////////////////
	//
	//	PURPOSE
	//		-	Constructor for the object
	//
	////////////////////////////////////////////////////////////////////////
	pImg	= NULL;
	iIdx	= 0;
	}	// Contours

HRESULT Contours :: onAttach ( bool bAttach )
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
//		if (pnDesc->load ( adtString(L"Size"), vL ) == S_OK)
//			iSz = vL;
		}	// if

	// Detach
	else
		{
		// Shutdown
		_RELEASE(pImg);
//		_RELEASE(pKer);
		}	// else

	return hr;
	}	// onAttach

HRESULT Contours :: onReceive ( IReceptor *pr, const ADTVALUE &v )
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

	// First/next
	if (_RCP(First) || _RCP(Next))
		{
		IDictionary	*pImgUse = NULL;
		IDictionary	*pImgC	= NULL;
		cvMatRef		*pMat		= NULL;
		adtIUnknown	unkV(v);
//		cvMatRef

		// Expecting dictionary to contain next contour
		CCLTRY ( _QISAFE(unkV,IID_IDictionary,&pImgC) );

		// Obtain image refence
		CCLTRY ( Prepare::extract ( pImg, v, &pImgUse, &pMat ) );

		// First
		if (hr == S_OK && _RCP(First))
			{
			// Find contours likes to crash readily
			try
				{
				// Execute (warning, findContours can be crashy)
				if (pMat->isMat())
					cv::findContours (	*(pMat->mat), contours, CV_RETR_EXTERNAL, 
												CV_CHAIN_APPROX_SIMPLE );
				#ifdef	HAVE_OPENCV_UMAT
				else if (pMat->isUMat())
					cv::findContours (	*(pMat->umat), contours, CV_RETR_EXTERNAL, 
												CV_CHAIN_APPROX_SIMPLE );
				#endif
				#ifdef	HAVE_OPENCV_CUDA
				else if (pMat->isGPU())
					{
					cv::Mat		matNoGpu;

					// Currently no GPU based version
					pMat->gpumat->download ( matNoGpu );

					// Execute
					cv::findContours (	matNoGpu, contours, CV_RETR_EXTERNAL, 
												CV_CHAIN_APPROX_SIMPLE );

					// No need to restore back up to GPU as purpose of function
					// is not to modify image even if it is destructive
					}	// if
				#endif

				// Debug
				lprintf ( LOG_INFO, L"Contours size %d\r\n", contours.size() );
				for (U32 c = 0;c < contours.size();++c)
					lprintf ( LOG_INFO, L"%d) Size %d\r\n", c, contours[c].size() );
				}	// try
			catch ( cv::Exception & )
				{
				lprintf ( LOG_INFO, L"findContours threw an exception\r\n" );
				hr = S_FALSE;
				}	// catch

			// Reset enumeration
			iIdx = 0;
			}	// if

		// More ?
		CCLTRYE ( iIdx < contours.size(), ERROR_NOT_FOUND );

		// Next
		if (hr == S_OK)
			{
			cvMatRef	*pMatC	= NULL;

			// Create matrix reference for result
			CCLTRYE ( (pMatC = new cvMatRef()) != NULL, E_OUTOFMEMORY );
			CCLTRYE ( (pMatC->mat = new cv::Mat(contours[iIdx])) != NULL,
							E_OUTOFMEMORY );

			// Store in result dictionary
			CCLTRY ( pImgC->store (	adtString(L"cvMatRef"), adtIUnknown(pMatC) ) );

			// For next enumeration
			++iIdx;
			}	// if

		// Debug
		if (hr != S_OK)
			lprintf ( LOG_DBG, L"findContours failed : 0x%x\r\n", hr );

		// Result
		if (hr == S_OK)
			_EMT(Next,adtIUnknown(pImgC));
		else
			_EMT(End,adtInt(hr));

		// Clean up
		_RELEASE(pImgC);
		_RELEASE(pMat);
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

