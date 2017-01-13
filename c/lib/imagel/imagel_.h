////////////////////////////////////////////////////////////////////////
//
//										IMAGEL_.H
//
//				Implementation include file for image processing library
//
////////////////////////////////////////////////////////////////////////

#ifndef	IMAGEL__H
#define	IMAGEL__H

// Includes
#include "imagel.h"
#include "../../lib/nspcl/nspcl.h"

// OpenCV - Currently expecting 3.1+
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>

// OpenCL
#include <opencv2/core/ocl.hpp>

// CUDA
#include <opencv2/cudafeatures2d.hpp>
#include <opencv2/cudaimgproc.hpp>
#include <opencv2/cudaarithm.hpp>
#include <opencv2/cudawarping.hpp>

// Relevant math operations from 'mathl_.h'

// Operations
#define	MATHOP_NOP		-1

// Arithmetic
#define	MATHOP_ADD		1
#define	MATHOP_SUB		2
#define	MATHOP_MUL		3
#define	MATHOP_DIV		4

// Bitwise
#define	MATHOP_AND		10
#define	MATHOP_OR		11
#define	MATHOP_XOR		12

// Comparison
#define	MATHOP_EQ		60
#define	MATHOP_GT		61
#define	MATHOP_GE		62
#define	MATHOP_LT		63
#define	MATHOP_LE		64
#define	MATHOP_NE		65

///////////
// Objects
///////////

//
// Class - cvMatRef.  Object to cache reference counted OpenCV matrices.
//

class cvMatRef :
	public CCLObject										// Base class
	{
	public :
	cvMatRef ( void );									// Constructor
	virtual ~cvMatRef ( void );						// Destructor

	// Utilities
	bool isGPU	( void ) { return (gpumat != NULL); }
	bool isUMat ( void ) { return (umat != NULL); }
	bool isMat ( void ) { return (mat != NULL); }
	S32	rows ( void )
			{ return (mat != NULL) ? mat->rows :
						(umat != NULL) ? umat->rows :
						(gpumat != NULL) ? gpumat->rows : 0; }
	S32	cols ( void )
			{ return (mat != NULL) ? mat->cols :
						(umat != NULL) ? umat->cols :
						(gpumat != NULL) ? gpumat->cols : 0; }
	S32	channels ( void )
			{ return (mat != NULL) ? mat->channels() :
						(umat != NULL) ? umat->channels() :
						(gpumat != NULL) ? gpumat->channels() : 0; }

	// Run-time data
	cv::Mat				*mat;								// CPU matrix
	cv::cuda::GpuMat	*gpumat;							// GPU matrix
	cv::UMat				*umat;							// Universal/OpenCL matrix

	// CCL
	CCL_OBJECT_BEGIN_INT(cvMatRef)
	CCL_OBJECT_END()
	};

/////////
// Nodes
/////////

//
// Class - At.  Node to address individual pixels.
//

class At :
	public CCLObject,										// Base class
	public Behaviour										// Interface
	{
	public :
	At ( void );											// Constructor

	// Run-time data
	IDictionary	*pImg;									// Image dictionary
	adtInt		iX,iY;									// Positions
	adtValue		vAt;										// Value

	// CCL
	CCL_OBJECT_BEGIN(At)
		CCL_INTF(IBehaviour)
	CCL_OBJECT_END()

	// Connections
	DECLARE_EMT(Error)
	DECLARE_RCP(Image)
	DECLARE_CON(Load)
	DECLARE_RCP(X)
	DECLARE_CON(Store)
	DECLARE_RCP(Y)
	DECLARE_RCP(Value)
	BEGIN_BEHAVIOUR()
		DEFINE_EMT(Error)
		DEFINE_RCP(Image)
		DEFINE_CON(Load)
		DEFINE_RCP(X)
		DEFINE_CON(Store)
		DEFINE_RCP(Y)
		DEFINE_RCP(Value)
	END_BEHAVIOUR_NOTIFY()
	};

//
// Class - Binary.  Perform a binary operation involving images.
//

class Binary :
	public CCLObject,										// Base class
	public Behaviour										// Interface
	{
	public :
	Binary ( void );										// Constructor

	// Run-time data
	adtValue		vL,vR;									// Parameters
	adtValue		vRes;										// Result
	int			iOp;										// Math operation

	// CCL
	CCL_OBJECT_BEGIN(Binary)
		CCL_INTF(IBehaviour)
	CCL_OBJECT_END()

	// Connections
	DECLARE_CON(Fire)
	DECLARE_RCP(Left)
	DECLARE_RCP(Right)
	DECLARE_EMT(Error)
	BEGIN_BEHAVIOUR()
		DEFINE_CON(Fire)
		DEFINE_RCP(Left)
		DEFINE_RCP(Right)
		DEFINE_EMT(Error)
	END_BEHAVIOUR_NOTIFY()
	};

//
// Class - Contours.  Enumerate contours in an image.
//

class Contours :
	public CCLObject,										// Base class
	public Behaviour										// Interface
	{
	public :
	Contours ( void );									// Constructor

	// Run-time data
	IDictionary	*pImg;									// Image dictionary
	std::vector<std::vector<cv::Point>>	
					contours;								// Active contour list
	U32			iIdx;										// Enumeration index
	
	// CCL
	CCL_OBJECT_BEGIN(Contours)
		CCL_INTF(IBehaviour)
	CCL_OBJECT_END()

	// Connections
	DECLARE_EMT(Error)
	DECLARE_RCP(First)
	DECLARE_EMT(End)
	DECLARE_RCP(Image)
	DECLARE_CON(Next)
	BEGIN_BEHAVIOUR()
		DEFINE_EMT(Error)
		DEFINE_RCP(First)
		DEFINE_EMT(End)
		DEFINE_RCP(Image)
		DEFINE_CON(Next)
	END_BEHAVIOUR_NOTIFY()
	};

//
// Class - Convert.  Convert between formats.
//

class Convert :
	public CCLObject,										// Base class
	public Behaviour										// Interface
	{
	public :
	Convert ( void );										// Constructor

	// Run-time data
	IDictionary			*pImg;							// Source image
	adtString			strTo;							// Convert 'to' format

	// Utilities
	static
	HRESULT convertTo	( cvMatRef *, cvMatRef *, U32 );
	static
	HRESULT convertTo	( cv::UMat *, 
								cv::UMat *, U32 );
	static
	HRESULT convertTo	( cv::cuda::GpuMat *, 
								cv::cuda::GpuMat *, U32 );

	// CCL
	CCL_OBJECT_BEGIN(Convert)
		CCL_INTF(IBehaviour)
	CCL_OBJECT_END()

	// Connections
	DECLARE_RCP(Color)
	DECLARE_CON(Fire)
	DECLARE_RCP(Image)
	DECLARE_EMT(Error)
	BEGIN_BEHAVIOUR()
		DEFINE_RCP(Color)
		DEFINE_CON(Fire)
		DEFINE_RCP(Image)
		DEFINE_EMT(Error)
	END_BEHAVIOUR_NOTIFY()
	};

//
// Class - Create.  Create a new image.
//

class Create :
	public CCLObject,										// Base class
	public Behaviour										// Interface
	{
	public :
	Create ( void );										// Constructor

	// Run-time data
	IDictionary	*pImg;									// Image dictionary
	adtString	strFmt;									// Format
	adtInt		iW,iH;									// Size
	adtBool		bCPU;										// Force CPU bound image
	adtString	strType;									// Image creation type

	// Utilities
	static
	HRESULT create	( IDictionary *, U32, U32, U32, cvMatRef **,
							bool = false );

	// CCL
	CCL_OBJECT_BEGIN(Create)
		CCL_INTF(IBehaviour)
	CCL_OBJECT_END()

	// Connections
	DECLARE_CON(Fire)
	DECLARE_RCP(Format)
	DECLARE_RCP(Height)
	DECLARE_RCP(Image)
	DECLARE_EMT(Error)
	DECLARE_RCP(Width)
	BEGIN_BEHAVIOUR()
		DEFINE_CON(Fire)
		DEFINE_RCP(Format)
		DEFINE_RCP(Height)
		DEFINE_RCP(Image)
		DEFINE_EMT(Error)
		DEFINE_RCP(Width)
	END_BEHAVIOUR_NOTIFY()
	};

//
// Class - Distance.  Image distance transform.
//

class Distance :
	public CCLObject,										// Base class
	public Behaviour										// Interface
	{
	public :
	Distance ( void );									// Constructor

	// Run-time data
	IDictionary	*pImg;									// Image dictionary

	// CCL
	CCL_OBJECT_BEGIN(Distance)
		CCL_INTF(IBehaviour)
	CCL_OBJECT_END()

	// Connections
	DECLARE_EMT(Error)
	DECLARE_CON(Fire)
	DECLARE_RCP(Image)
	BEGIN_BEHAVIOUR()
		DEFINE_EMT(Error)
		DEFINE_CON(Fire)
		DEFINE_RCP(Image)
	END_BEHAVIOUR_NOTIFY()
	};

//
// Class - Draw.  Shape drawing node.
//

class Draw :
	public CCLObject,										// Base class
	public Behaviour										// Interface
	{
	public :
	Draw ( void );											// Constructor

	// Run-time data
	IDictionary	*pImg;									// Image dictionary
	adtFloat		fR,fB,fG;								// Color
	adtFloat		fX0,fX1,fY0,fY1;						// Endpoints
	adtFloat		fW,fH;									// Width,height
	adtFloat		fA,fA0,fA1;								// Angles
	adtInt		iThick;									// Thickness
	adtFloat		fRad;										// Radius
	adtString	strShp;									// Shape to draw

	// CCL
	CCL_OBJECT_BEGIN(Draw)
		CCL_INTF(IBehaviour)
	CCL_OBJECT_END()

	// Connections
	DECLARE_RCP(Angle)
	DECLARE_RCP(Angle0)
	DECLARE_RCP(Angle1)
	DECLARE_EMT(Error)
	DECLARE_CON(Fire)
	DECLARE_RCP(Height)
	DECLARE_RCP(Image)
	DECLARE_RCP(Radius)
	DECLARE_RCP(Thick)
	DECLARE_RCP(Width)
	DECLARE_RCP(X0)
	DECLARE_RCP(Y0)
	DECLARE_RCP(X1)
	DECLARE_RCP(Y1)
	BEGIN_BEHAVIOUR()
		DEFINE_RCP(Angle)
		DEFINE_RCP(Angle0)
		DEFINE_RCP(Angle1)
		DEFINE_EMT(Error)
		DEFINE_CON(Fire)
		DEFINE_RCP(Height)
		DEFINE_RCP(Image)
		DEFINE_RCP(Thick)
		DEFINE_RCP(Radius)
		DEFINE_RCP(Width)
		DEFINE_RCP(X0)
		DEFINE_RCP(Y0)
		DEFINE_RCP(X1)
		DEFINE_RCP(Y1)
	END_BEHAVIOUR_NOTIFY()
	};

//
// Class - Features.  Feature extraction node.
//

class Features :
	public CCLObject,										// Base class
	public Behaviour										// Interface
	{
	public :
	Features ( void );									// Constructor

	// Run-time data
	IDictionary	*pImg;									// Image dictionary
	adtString	strType;									// Feature type

	// CCL
	CCL_OBJECT_BEGIN(Features)
		CCL_INTF(IBehaviour)
	CCL_OBJECT_END()

	// Connections
	DECLARE_EMT(Error)
	DECLARE_CON(Fire)
	DECLARE_RCP(Image)
	BEGIN_BEHAVIOUR()
		DEFINE_EMT(Error)
		DEFINE_CON(Fire)
		DEFINE_RCP(Image)
	END_BEHAVIOUR_NOTIFY()

	private :

	// Internal state
	cv::Ptr<cv::cuda::CannyEdgeDetector>	pgpuCanny;

	};

//
// Class - Flip.  Image flip node.
//

class Flip :
	public CCLObject,										// Base class
	public Behaviour										// Interface
	{
	public :
	Flip ( void );											// Constructor

	// Run-time data
	IDictionary	*pImg;									// Image dictionary
	adtBool		bHorz,bVert;							// Flip options

	// CCL
	CCL_OBJECT_BEGIN(Flip)
		CCL_INTF(IBehaviour)
	CCL_OBJECT_END()

	// Connections
	DECLARE_EMT(Error)
	DECLARE_CON(Fire)
	DECLARE_RCP(Image)
	DECLARE_RCP(Transpose)
	BEGIN_BEHAVIOUR()
		DEFINE_EMT(Error)
		DEFINE_CON(Fire)
		DEFINE_RCP(Image)
		DEFINE_RCP(Transpose)
	END_BEHAVIOUR_NOTIFY()
	};

//
// Class - FFT.  FFT processing node.
//

class FFT :
	public CCLObject,										// Base class
	public Behaviour										// Interface
	{
	public :
	FFT ( void );											// Constructor

	// Run-time data
	IDictionary	*pImg;									// Image dictionary
	adtString	strWnd;									// Window function
	cvMatRef		*pWnd;									// Window function
	adtBool		bRows;									// FFT by rows

	// CCL
	CCL_OBJECT_BEGIN(FFT)
		CCL_INTF(IBehaviour)
	CCL_OBJECT_END()

	// Connections
	DECLARE_EMT(Error)
	DECLARE_CON(Fire)
	DECLARE_RCP(Image)
	DECLARE_RCP(Window)
	BEGIN_BEHAVIOUR()
		DEFINE_EMT(Error)
		DEFINE_CON(Fire)
		DEFINE_RCP(Image)
		DEFINE_RCP(Window)
	END_BEHAVIOUR_NOTIFY()

	private :

	// NOTE: Keep a single copy of GPU bound matrices to use
	// for FFTs.  If created on the stack every time it seems to 
	// re-initialize things every time so every FFT is as slow as
	// the first one.
	cv::cuda::GpuMat			gpuPad,gpuPlanes[2],gpuCmplx,gpuMag;
	cv::UMat						umatPad,umatCmplx,umatMag;
	std::vector<cv::UMat>	umatPlanes;

	// Internal utilities
	HRESULT fft		(	cv::cuda::GpuMat *, 	cv::cuda::GpuMat *, bool );
	HRESULT fft		(	cv::UMat *, cv::UMat *, bool );
	HRESULT fft		(	cv::Mat *, 	cv::Mat *, bool );
	HRESULT window ( cvMatRef * );
	};

//
// Class - Gradient.  Image grandient node.
//

class Gradient :
	public CCLObject,										// Base class
	public Behaviour										// Interface
	{
	public :
	Gradient ( void );									// Constructor

	// Run-time data
	IDictionary	*pImg;									// Image dictionary
	adtString	strType;									// Gradient type
	adtInt		iDx,iDy;									// Order of derivatives
	adtInt		iSzk;										// Kernel size

	// CCL
	CCL_OBJECT_BEGIN(Gradient)
		CCL_INTF(IBehaviour)
	CCL_OBJECT_END()

	// Connections
	DECLARE_EMT(Error)
	DECLARE_CON(Fire)
	DECLARE_RCP(Image)
	BEGIN_BEHAVIOUR()
		DEFINE_EMT(Error)
		DEFINE_CON(Fire)
		DEFINE_RCP(Image)
	END_BEHAVIOUR_NOTIFY()

	private :

	// Run-time data
	cv::cuda::Filter					*pgpuSobel;
	cv::cuda::Filter					*pgpuScharr;
	cv::cuda::Filter					*pgpuLaplace;

	};

//
// Class - Match.  Template matching node.
//

class Match :
	public CCLObject,										// Base class
	public Behaviour										// Interface
	{
	public :
	Match ( void );										// Constructor

	// Run-time data
	IDictionary	*pImg;									// Reference image
	IDictionary *pTmp;									// Template image
	cv::Ptr<cv::cuda::TemplateMatching>
					pTm;										// Template matching object
	U32			tmType;									// Image type for template

	// CCL
	CCL_OBJECT_BEGIN(Match)
		CCL_INTF(IBehaviour)
	CCL_OBJECT_END()

	// Connections
	DECLARE_CON(Fire)
	DECLARE_RCP(Template)
	DECLARE_RCP(Image)
	DECLARE_EMT(Error)
	BEGIN_BEHAVIOUR()
		DEFINE_CON(Fire)
		DEFINE_RCP(Image)
		DEFINE_EMT(Error)
		DEFINE_RCP(Template)
	END_BEHAVIOUR_NOTIFY()
	};

//
// Class - Morph.  Image morphology node.
//

class Morph :
	public CCLObject,										// Base class
	public Behaviour										// Interface
	{
	public :
	Morph ( void );										// Constructor

	// Run-time data
	IDictionary			*pImg;							// Image dictionary
	cv::cuda::Filter	*pfOpen;							// Open filter
	cv::cuda::Filter 	*pfClose;						// Close filter
	cv::cuda::Filter	*pfDi;							// Dilate filter
	cv::cuda::Filter	*pfEr;							// Erosion filter
	adtString			strType;							// Feature type

	// CCL
	CCL_OBJECT_BEGIN(Morph)
		CCL_INTF(IBehaviour)
	CCL_OBJECT_END()

	// Connections
	DECLARE_EMT(Error)
	DECLARE_CON(Fire)
	DECLARE_RCP(Image)
//	DECLARE_RCP(Kernel)
	BEGIN_BEHAVIOUR()
		DEFINE_EMT(Error)
		DEFINE_CON(Fire)
		DEFINE_RCP(Image)
//		DEFINE_RCP(Kernel)
	END_BEHAVIOUR_NOTIFY()
	};

//
// Class - Normalize.  Image normalization node.
//

class Normalize :
	public CCLObject,										// Base class
	public Behaviour										// Interface
	{
	public :
	Normalize ( void );									// Constructor

	// Run-time data
	IDictionary	*pImg;									// Image dictionary
	adtValue		vFrom,vTo;								// Normalization range

	// CCL
	CCL_OBJECT_BEGIN(Normalize)
		CCL_INTF(IBehaviour)
	CCL_OBJECT_END()

	// Connections
	DECLARE_EMT(Error)
	DECLARE_CON(Fire)
	DECLARE_RCP(Image)
	BEGIN_BEHAVIOUR()
		DEFINE_EMT(Error)
		DEFINE_CON(Fire)
		DEFINE_RCP(Image)
	END_BEHAVIOUR_NOTIFY()
	};

//
// Class - Persist.  Image persistence node.
//

class PersistImage :
	public CCLObject,										// Base class
	public Behaviour										// Interface
	{
	public :
	PersistImage ( void );								// Constructor

	// Run-time data
	IDictionary	*pDctImg;								// Image dictionary
	adtString	strLoc;									// Persistence location

	// CCL
	CCL_OBJECT_BEGIN(PersistImage)
		CCL_INTF(IBehaviour)
	CCL_OBJECT_END()

	// Connections
	DECLARE_CON(Load)
	DECLARE_CON(Save)
	DECLARE_RCP(Image)
	DECLARE_RCP(Location)
	DECLARE_EMT(Error)
	BEGIN_BEHAVIOUR()
		DEFINE_CON(Load)
		DEFINE_CON(Save)
		DEFINE_RCP(Image)
		DEFINE_RCP(Location)
		DEFINE_EMT(Error)
	END_BEHAVIOUR_NOTIFY()
	};

//
// Class - Prepare.  Image preparation for processing.
//

class Prepare :
	public CCLObject,										// Base class
	public Behaviour										// Interface
	{
	public :
	Prepare ( void );										// Constructor

	// Run-time data
	IDictionary	*pImg;									// Image dictionary
	adtBool		bCPU;										// Force CPU bound image
	adtBool		bRel;										// Release only on download

	// Utilities
	static
	HRESULT extract ( IDictionary *, const ADTVALUE &,
							IDictionary **, cvMatRef ** = NULL );
	static 
	HRESULT gpuInit ( void );

	// CCL
	CCL_OBJECT_BEGIN(Prepare)
		CCL_INTF(IBehaviour)
	CCL_OBJECT_END()

	// Connections
	DECLARE_RCP(Image)
	DECLARE_CON(Download)
	DECLARE_CON(Upload)
	DECLARE_EMT(Error)
	BEGIN_BEHAVIOUR()
		DEFINE_RCP(Image)
		DEFINE_CON(Download)
		DEFINE_CON(Upload)
		DEFINE_EMT(Error)
	END_BEHAVIOUR_NOTIFY()

	private :
	};

//
// Class - Resize.  Resize an image.
//

class Resize :
	public CCLObject,										// Base class
	public Behaviour										// Interface
	{
	public :
	Resize ( void );										// Constructor

	// Run-time data
	IDictionary	*pImg;									// Image dictionary
	adtInt		iW,iH;									// Size

	// CCL
	CCL_OBJECT_BEGIN(Resize)
		CCL_INTF(IBehaviour)
	CCL_OBJECT_END()

	// Connections
	DECLARE_CON(Fire)
	DECLARE_RCP(Height)
	DECLARE_RCP(Image)
	DECLARE_EMT(Error)
	DECLARE_RCP(Width)
	BEGIN_BEHAVIOUR()
		DEFINE_CON(Fire)
		DEFINE_RCP(Height)
		DEFINE_RCP(Image)
		DEFINE_EMT(Error)
		DEFINE_RCP(Width)
	END_BEHAVIOUR_NOTIFY()
	};

//
// Class - Roi.  Region of interest node.
//

class Roi :
	public CCLObject,										// Base class
	public Behaviour										// Interface
	{
	public :
	Roi ( void );											// Constructor

	// Run-time data
	IDictionary	*pSrc,*pDst;							// Image dictionaries
	adtInt		iL,iT,iR,iB;							// ROI
	adtBool		bCopy;									// Copy ROI into own image

	// CCL
	CCL_OBJECT_BEGIN(Roi)
		CCL_INTF(IBehaviour)
	CCL_OBJECT_END()

	// Connections
	DECLARE_RCP(Bottom)
	DECLARE_RCP(Destination)
	DECLARE_EMT(Error)
	DECLARE_CON(Fire)
	DECLARE_RCP(Left)
	DECLARE_RCP(Right)
	DECLARE_RCP(Source)
	DECLARE_RCP(Top)
	BEGIN_BEHAVIOUR()
		DEFINE_RCP(Bottom)
		DEFINE_RCP(Destination)
		DEFINE_EMT(Error)
		DEFINE_CON(Fire)
		DEFINE_RCP(Left)
		DEFINE_RCP(Right)
		DEFINE_RCP(Source)
		DEFINE_RCP(Top)
	END_BEHAVIOUR_NOTIFY()
	};

//
// Class - Smooth.  Image smoothing node.
//

class Smooth :
	public CCLObject,										// Base class
	public Behaviour										// Interface
	{
	public :
	Smooth ( void );										// Constructor

	// Run-time data
	IDictionary	*pImg;									// Image dictionary
	adtString	strType;									// Smooth type
	adtInt		iSz;										// Kernel size

	// CCL
	CCL_OBJECT_BEGIN(Smooth)
		CCL_INTF(IBehaviour)
	CCL_OBJECT_END()

	// Connections
	DECLARE_EMT(Error)
	DECLARE_CON(Fire)
	DECLARE_RCP(Image)
	BEGIN_BEHAVIOUR()
		DEFINE_EMT(Error)
		DEFINE_CON(Fire)
		DEFINE_RCP(Image)
	END_BEHAVIOUR_NOTIFY()
	};

//
// Class - Stats.  Compute statistics about an image.
//

class Stats :
	public CCLObject,										// Base class
	public Behaviour										// Interface
	{
	public :
	Stats ( void );										// Constructor

	// Run-time data
	IDictionary	*pImg;									// Image dictionary
	adtBool		bEnt;										// Calculate entropy ?
	adtBool		bBoundRct;								// Calculate bounding rectangle ?
	adtBool		bNonZero;								// Calculate non-zero pixels ?
	adtBool		bSum;										// Calculate sum ?

	// CCL
	CCL_OBJECT_BEGIN(Stats)
		CCL_INTF(IBehaviour)
	CCL_OBJECT_END()

	// Connections
	DECLARE_EMT(Error)
	DECLARE_CON(Fire)
	DECLARE_RCP(Histogram)
	DECLARE_RCP(Image)
	BEGIN_BEHAVIOUR()
		DEFINE_EMT(Error)
		DEFINE_CON(Fire)
		DEFINE_RCP(Histogram)
		DEFINE_RCP(Image)
	END_BEHAVIOUR_NOTIFY()
	};

//
// Class - Threshold.  Thresholding node.
//

class Threshold :
	public CCLObject,										// Base class
	public Behaviour										// Interface
	{
	public :
	Threshold ( void );									// Constructor

	// Run-time data
	IDictionary	*pImg;									// Image dictionary
	adtValue		vT;										// Treshold value
	adtValue		vMax;										// Maximum value
	adtString	strOp;									// Operation

	// CCL
	CCL_OBJECT_BEGIN(Threshold)
		CCL_INTF(IBehaviour)
	CCL_OBJECT_END()

	// Connections
	DECLARE_EMT(Error)
	DECLARE_CON(Fire)
	DECLARE_RCP(Image)
	DECLARE_RCP(Value)
	BEGIN_BEHAVIOUR()
		DEFINE_EMT(Error)
		DEFINE_CON(Fire)
		DEFINE_RCP(Image)
		DEFINE_RCP(Value)
	END_BEHAVIOUR_NOTIFY()
	};


// Prototypes
//HRESULT image_fft			( cv::ocl::oclMat *, bool = false, bool = false );
HRESULT image_load		( const WCHAR *, IDictionary * );
HRESULT image_save		( IDictionary *, const WCHAR * );
HRESULT image_fft			( cv::Mat *, cv::Mat *, bool = false, bool = false );
HRESULT image_from_mat	( cv::Mat *, IDictionary * );
HRESULT image_to_mat		( IDictionary *, cv::Mat ** );
HRESULT image_to_debug	( cvMatRef *, const WCHAR *, const WCHAR * );

// From 'mathl'
HRESULT mathOp			( const WCHAR *, int * );

#endif
