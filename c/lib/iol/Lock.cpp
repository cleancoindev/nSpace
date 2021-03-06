////////////////////////////////////////////////////////////////////////
//
//									LOCK.CPP
//
//						Implementation of the lock node
//
////////////////////////////////////////////////////////////////////////

#include "iol_.h"

Lock :: Lock ( void )
	{
	////////////////////////////////////////////////////////////////////////
	//
	//	PURPOSE
	//		-	Constructor for the object.
	//
	////////////////////////////////////////////////////////////////////////
	}	// Lock

HRESULT Lock :: close ( void )
	{
	////////////////////////////////////////////////////////////////////////
	//
	//	FROM	IResource
	//
	//	PURPOSE
	//		-	Close the resource.
	//
	//	RETURN VALUE
	//		S_OK if successful
	//
	////////////////////////////////////////////////////////////////////////
	return lock.leave() ? S_OK : E_UNEXPECTED;
	}	// close

HRESULT Lock :: getResId ( ADTVALUE &vId )
	{
	////////////////////////////////////////////////////////////////////////
	//
	//	FROM	IResource
	//
	//	PURPOSE
	//		-	Return an identifier for the resource.
	//
	//	PARAMETERS
	//		-	vId will receive the identifer.
	//
	//	RETURN VALUE
	//		S_OK if successful
	//
	////////////////////////////////////////////////////////////////////////
	HRESULT		hr			= S_OK;

	// Does this make sense for "resource Id" ?
	CCLTRY ( adtValue::copy ( adtLong ( (U64)&lock ), vId ) );
	
	return hr;
	}	// getResId

HRESULT Lock :: open ( IDictionary *pOpts )
	{
	////////////////////////////////////////////////////////////////////////
	//
	//	FROM	IResource
	//
	//	PURPOSE
	//		-	Open a byte stream on top of a file.
	//
	//	PARAMETERS
	//		-	pOpts contain options for the file.
	//
	//	RETURN VALUE
	//		S_OK if successful
	//
	////////////////////////////////////////////////////////////////////////
	return lock.enter() ? S_OK : E_UNEXPECTED;
	}	// open
