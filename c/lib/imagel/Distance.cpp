////////////////////////////////////////////////////////////////////////
//
//									DISTANCE.CPP
//
//				Implementation of the image distance transform node.
//
////////////////////////////////////////////////////////////////////////

#include "imagel_.h"
#include <stdio.h>

// Globals

Distance :: Distance ( void )
	{
	////////////////////////////////////////////////////////////////////////
	//
	//	PURPOSE
	//		-	Constructor for the object
	//
	////////////////////////////////////////////////////////////////////////
	pImg	= NULL;
	}	// Distance

HRESULT Distance :: onAttach ( bool bAttach )
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
		}	// else

	return hr;
	}	// onAttach

HRESULT Distance :: onReceive ( IReceptor *pr, const ADTVALUE &v )
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
		IDictionary	*pImgUse = NULL;
		cvMatRef		*pMat		= NULL;

		// Obtain image refence
		CCLTRY ( Prepare::extract ( pImg, v, &pImgUse, &pMat ) );

		// Perform operation
		if (hr == S_OK)
			{
			if (pMat->isMat())
				cv::distanceTransform ( *(pMat->mat), *(pMat->mat), CV_DIST_L2, 3 );
			#ifdef	HAVE_OPENCV_UMAT
			else if (pMat->isUMat())
				cv::distanceTransform ( *(pMat->umat), *(pMat->umat), CV_DIST_L2, 3 );
			#endif
			#ifdef	HAVE_OPENCV_CUDA
			else if (pMat->isGPU())
				{
				cv::Mat		matNoGpu;

				// Currently no GPU based version
				pMat->gpumat->download ( matNoGpu );

				// Execute
				cv::distanceTransform ( matNoGpu, matNoGpu, CV_DIST_L2, 3 );

				// Restore
				pMat->gpumat->upload ( matNoGpu );
				}	// if
			#endif
			}	// if

		// Result
		if (hr == S_OK)
			_EMT(Fire,adtIUnknown(pImgUse));
		else
			_EMT(Error,adtInt(hr));

		// Debug
		if (hr != S_OK)
			lprintf ( LOG_DBG, L"distanceTranform failed : 0x%x\r\n", hr );

		// Clean up
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

