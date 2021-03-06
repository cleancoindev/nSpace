////////////////////////////////////////////////////////////////////////
//
//									OPENCV.CPP
//
//		OpenCV implmenetation of needed image processing functionality.
//
////////////////////////////////////////////////////////////////////////

#include "imagel_.h"
#include <stdio.h>

// OpenCV
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#if		_MSC_VER >= 1900
#else
//#include <opencv2/gpu/gpu.hpp>
#endif
//#include <opencv2/highgui/highgui.hpp>
//#include <opencv2/features2d/features2d.hpp>

// Globals
using namespace cv;
static bool bInit = false;
static bool bGPU	= false;

// String references
static adtString	strRefWidth(L"Width");
static adtString	strRefHeight(L"Height");
static adtString	strRefBits(L"Bits");
static adtString	strRefFormat(L"Format");

HRESULT image_load ( const WCHAR *pwLoc, IDictionary *pImg )
	{
	////////////////////////////////////////////////////////////////////////
	//
	//	PURPOSE
	//		-	Load an image from disk.
	//
	//	PARAMETERS
	//		-	pwLoc is the source path/filename.
	//		-	pImg will receive the image data.
	//
	//	RETURN VALUE
	//		S_OK if successful
	//
	////////////////////////////////////////////////////////////////////////
	HRESULT		hr			= S_OK;
//	Mat			*pmImg	= NULL;
	char			*paLoc	= NULL;
	adtString	strLoc(pwLoc);

	// Using OpenCV, needs full, valid Windows path
	#ifdef	_WIN32
	CCLOK ( strLoc.replace ( '/', '\\' ); )
	#endif

	// OpenCV needs ASCII
	CCLTRY ( strLoc.toAscii ( &paLoc ) );

	// 'imread' does not seem as stable on OpenCV.
	//	TODO: Replace with own PNG,BMP,etc persistence

	// Convert image into dictionary
	if (hr == S_OK)
		{
		cvMatRef	matRef;

//		cv::UMat mat = cv::imread ( paLoc, CV_LOAD_IMAGE_UNCHANGED ).getUMat(cv::ACCESS_READ);
//		hr = image_from_mat ( &mat, pImg );
		IplImage		*plImg = NULL;

		// Attempt load
		CCLTRYE ( (plImg	= cvLoadImage ( paLoc, CV_LOAD_IMAGE_UNCHANGED )) != NULL,
						E_UNEXPECTED );

		// Wrap into matrix
		if (hr == S_OK)
			{
//			cv::Mat mat ( plImg, false );
			cv::Mat mat = cv::cvarrToMat(plImg);
			matRef.mat = &mat;
			hr = image_from_mat ( &matRef, pImg );
			matRef.mat = NULL;
			}	// if

		// Clean up
		if (plImg != NULL)
			cvReleaseImage(&plImg);
		}	// if

	// Clean up
	_FREEMEM(paLoc);

	return hr;
	}	// image_load

HRESULT image_save ( IDictionary *pImg, const WCHAR *pwLoc )
	{
	////////////////////////////////////////////////////////////////////////
	//
	//	PURPOSE
	//		-	Save an image to disk.
	//
	//	PARAMETERS
	//		-	pImg contains the image data.
	//		-	pwLoc is the destination path/filename.
	//
	//	RETURN VALUE
	//		S_OK if successful
	//
	////////////////////////////////////////////////////////////////////////
	HRESULT		hr			= S_OK;
	Mat			*pmImg	= NULL;
	char			*paLoc	= NULL;
	adtString	strLoc(pwLoc);
	int			ret		= 0;
	bool			b			= false;

	// Using OpenCV, needs full, valid Windows path
	#ifdef	_WIN32
	CCLOK ( strLoc.replace ( '/', '\\' ); )
	#endif

	// Convert image to OpenCV
	CCLTRY ( image_to_mat ( pImg, &pmImg ) );

	// OpenCV needs ASCII
	CCLTRY ( strLoc.toAscii ( &paLoc ) );

	// 'imwrite' does not seem to work, OpenCV is so flaky
	//	TODO: Replace with own PNG,BMP,etc persistence
//	CCLOK ( ret = cvSaveImage ( paLoc, &(IplImage(*pmImg)) ); )
	CCLOK ( b = imwrite ( paLoc, *pmImg ); )
	if (!b)
		lprintf ( LOG_INFO, L"cvSaveImage : %S : b %d errno %d\r\n", 
										paLoc, b, errno );

	// Clean up
	_FREEMEM(paLoc);
	if (pmImg != NULL)
		delete pmImg;

	return hr;
	}	// image_save

HRESULT image_to_debug ( cvMatRef *pMat, const WCHAR *pwCtx, 
									const WCHAR *pwLoc )
	{
	////////////////////////////////////////////////////////////////////////
	//
	//	PURPOSE
	//		-	Output/save debug information about an image.
	//
	//	PARAMETERS
	//		-	pMat is the matrix object
	//		-	pwCtx is the context
	//		-	pwLoc is the destination for the image
	//
	//	RETURN VALUE
	//		S_OK if successful
	//
	////////////////////////////////////////////////////////////////////////
	HRESULT		hr			= S_OK;
	char			*paLoc	= NULL;
	U32			type		= 0;
	cv::Point	ptMin,ptMax;
	double		dMin,dMax;
	adtString	strLoc(pwLoc);
	cv::Mat		matSave;

	// Values and locations of min and max
	if (pMat->isMat())
		{
		cv::minMaxLoc ( *(pMat->mat), &dMin, &dMax, &ptMin, &ptMax );
		type = pMat->mat->type();
		}	// if
	#ifdef	HAVE_OPENCV_UMAT
	else if (pMat->isUMat())
		{
		cv::minMaxLoc ( *(pMat->umat), &dMin, &dMax, &ptMin, &ptMax );
		type = pMat->umat->type();
		}	// if
	#endif
	#ifdef	HAVE_OPENCV_CUDA
	else if (pMat->isGPU())
		{
		cv::cuda::minMaxLoc ( *(pMat->gpumat), &dMin, &dMax, &ptMin, &ptMax );
		type = pMat->gpumat->type();
		}	// if
	#endif
	dbgprintf ( L"%s (%s) : Min : %g @ (%d,%d) : Max : %g @ (%d,%d) : type %d\r\n",
						pwCtx, 
						(pMat->isGPU()) ? L"GPU" :
						(pMat->isUMat()) ? L"UMAT" : L"CPU",
						dMin, ptMin.x, ptMin.y, dMax, ptMax.x, ptMax.y,
						type );

	// Download
	if (pMat->isMat())
		pMat->mat->copyTo ( matSave );
	#ifdef	HAVE_OPENCV_UMAT
	else if (pMat->isUMat())
		pMat->umat->copyTo ( matSave );
	#endif
	#ifdef	HAVE_OPENCV_CUDA
	else if (pMat->isGPU())
		pMat->gpumat->download ( matSave );
	#endif

	// Some sample point values
	if (matSave.type() == CV_32FC1)
		dbgprintf ( L"%g %g %g %g - %g %g %g %g\r\n",
						matSave.at<float>(0,0),
						matSave.at<float>(0,1),
						matSave.at<float>(0,2),
						matSave.at<float>(0,3),
						matSave.at<float>(1,0),
						matSave.at<float>(1,1),
						matSave.at<float>(1,2),
						matSave.at<float>(1,3) );

	// Conversion for saving to image
	cv::normalize ( matSave, matSave, 0, 255, cv::NORM_MINMAX );
	matSave.convertTo ( matSave, CV_8UC1 );

	// Save to file
	if (strLoc.toAscii ( &paLoc ) == S_OK)
		cv::imwrite ( paLoc, matSave );
	
	// Clean up
	_FREEMEM(paLoc);

	return hr;
	}	// image_to_debug

HRESULT image_format ( cvMatRef *pM, adtString &strF )
	{
	////////////////////////////////////////////////////////////////////////
	//
	//	PURPOSE
	//		-	Determine string version of format for image.
	//
	//	PARAMETERS
	//		-	pM contains the image data
	//		-	strF will receive the format
	//
	//	RETURN VALUE
	//		S_OK if successful
	//
	////////////////////////////////////////////////////////////////////////
	HRESULT			hr			= S_OK;

	// Assign format, add new formats as needed.
	switch ( pM->channels() )
		{
		case 1:
			// Gray-scale
			switch ( (pM->type() & CV_MAT_DEPTH_MASK) )
				{
				case CV_8U:
					strF = L"U8x2";
					break;
				case CV_16U:
					strF = L"U16x2";
					break;
				case CV_16S:
					strF = L"S16x2";
					break;
				case CV_32F:
					strF = L"F32x2";
					break;
				default :
					hr = E_NOTIMPL;
				}	// switch
			break;
		case 3 :
			// Color
			switch ( (pM->type() & CV_MAT_DEPTH_MASK) )
				{
				case CV_8U :
					strF = L"B8G8R8";
					break;
				default :
					hr = E_NOTIMPL;
				}	// switch
			break;
		case 4 :
			// Color
			switch ( (pM->type() & CV_MAT_DEPTH_MASK) )
				{
				case CV_8U :
					strF = L"B8G8R8A8";
					break;
				default :
					hr = E_NOTIMPL;
				}	// switch
			break;
		default :
			lprintf ( LOG_WARN, L"Unsupported channels %d", pM->channels() );
			hr = E_NOTIMPL;
		}	// switch

	// Debug
	if (hr != S_OK)
		lprintf ( LOG_WARN, L"Failed 0x%x", hr );

	return hr;
	}	// image_format

HRESULT image_from_mat ( cvMatRef *pMat, IDictionary *pImg )
	{
	////////////////////////////////////////////////////////////////////////
	//
	//	PURPOSE
	//		-	Convert an OpenCV mat object to an image dictionary.
	//
	//	PARAMETERS
	//		-	pMat contains the image data
	//		-	pImg will receive the image data
	//
	//	RETURN VALUE
	//		S_OK if successful
	//
	////////////////////////////////////////////////////////////////////////
	HRESULT			hr			= S_OK;
	IMemoryMapped	*pBits	= NULL;
	void				*pvBits	= NULL;
	adtString		strFmt;
	cv::Mat			mat;
	adtValue			vL;

	// Must download first
	if (pMat->isMat())
		mat = *(pMat->mat);
	#ifdef	HAVE_OPENCV_UMAT
	else if (pMat->isUMat())
		mat = pMat->umat->getMat(cv::ACCESS_READ);
	#endif
	#ifdef	HAVE_OPENCV_CUDA
	else if (pMat->isGPU())
		pMat->gpumat->download(mat);
	#endif

	// Image information
	CCLTRY ( pImg->store ( strRefWidth, adtInt(mat.cols) ) );
	CCLTRY ( pImg->store ( strRefHeight, adtInt(mat.rows) ) );

	// Does image already have a bits memory block that can be re-used ?
	if (hr == S_OK && pImg->load ( strRefBits, vL ) == S_OK)
		{
		CCLTRY ( _QISAFE(adtIUnknown(vL),IID_IMemoryMapped,&pBits) );
		}	// if
	else
		{
		// Create and size memory block for image
		CCLTRY ( COCREATE ( L"Io.MemoryBlock", IID_IMemoryMapped, &pBits ) );
		CCLTRY ( pImg->store ( strRefBits, adtIUnknown(pBits) ) );
		}	// else

	// Set size and copy bits
	CCLTRY ( pBits->setSize ( (U32)(mat.total()*mat.elemSize()) ) );
	CCLTRY ( pBits->lock ( 0, 0, &pvBits, NULL ) );
	CCLOK  ( memcpy ( pvBits, mat.data, mat.total()*mat.elemSize() ); )

	// Retreive equivalent system format
	CCLTRY ( image_format ( pMat, strFmt ) );
	CCLTRY ( pImg->store ( strRefFormat, strFmt ) );

	// Debug
	if (hr != S_OK)
		lprintf ( LOG_WARN, L"Failed 0x%x", hr );

	// Clean up
	_UNLOCK(pBits,pvBits);
	_RELEASE(pBits);

	return hr;
	}	// image_from_mat

HRESULT image_to_mat ( IDictionary *pImg, Mat **ppM )
	{
	////////////////////////////////////////////////////////////////////////
	//
	//	PURPOSE
	//		-	Wrap an image inside an OpenCV matrix.
	//
	//	PARAMETERS
	//		-	pImg contains the image data.
	//		-	ppM will receive the matrix.
	//
	//	RETURN VALUE
	//		S_OK if successful
	//
	////////////////////////////////////////////////////////////////////////
	HRESULT			hr			= S_OK;
	IMemoryMapped	*pBits	= NULL;
	void				*pvBits	= NULL;
	adtValue			vL;
	adtString		strFmt;
	adtIUnknown		unkV;
	U32				w,h,cvFmt;

	// Setup
	(*ppM) = NULL;

	// Access image information
	CCLTRY ( pImg->load ( strRefWidth, vL ) );
	CCLOK  ( w = adtInt(vL); )
	CCLTRY ( pImg->load ( strRefHeight, vL ) );
	CCLOK  ( h = adtInt(vL); )
	CCLTRY ( pImg->load ( strRefFormat, vL ) );
	CCLTRYE( (strFmt = vL).length() > 0, E_UNEXPECTED );
	CCLTRY ( pImg->load ( strRefBits, vL ) );
	CCLTRY ( _QISAFE((unkV=vL),IID_IMemoryMapped,&pBits) );
	CCLTRY ( pBits->lock ( 0, 0, &pvBits, NULL ) );

	// NOTE: Add needed formats over time
	if (hr == S_OK && !WCASECMP(strFmt,L"U16X2"))
		cvFmt = CV_16UC1;
	else if (hr == S_OK && !WCASECMP(strFmt,L"S16X2"))
		cvFmt = CV_16SC1;
	else if (hr == S_OK && !WCASECMP(strFmt,L"U8X2"))
		cvFmt = CV_8UC1;
	else if (hr == S_OK && !WCASECMP(strFmt,L"S8X2"))
		cvFmt = CV_8SC1;
	else if (hr == S_OK && !WCASECMP(strFmt,L"F32X2"))
		cvFmt = CV_32FC1;

	// Image formats
	else if (hr == S_OK && 
				(!WCASECMP(strFmt,L"R8G8B8") || !WCASECMP(strFmt,L"B8G8R8")))
		cvFmt = CV_8UC3;
	else 
		hr = ERROR_NOT_SUPPORTED;

	// Wrap bits
	CCLTRYE ( ((*ppM) = new Mat ( h, w, cvFmt, pvBits )) != NULL,
												E_OUTOFMEMORY );

	// Clean up
	_UNLOCK(pBits,pvBits);
	_RELEASE(pBits);

	return hr;
	}	// image_to_mat
