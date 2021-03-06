////////////////////////////////////////////////////////////////////////
//
//										CLONE.CPP
//
//					Implementation of the clone value node
//
////////////////////////////////////////////////////////////////////////

#include "miscl_.h"

Clone :: Clone ( void )
	{
	////////////////////////////////////////////////////////////////////////
	//
	//	PURPOSE
	//		-	Constructor for the node
	//
	////////////////////////////////////////////////////////////////////////
	}	// Clone

HRESULT Clone :: onAttach ( bool bAttach )
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
		adtValue		v;

		// Defaults
		if (pnDesc->load ( adtString(L"Value"), v ) == S_OK)
			adtValue::copy ( v, vClone );
		}	// if

	return hr;
	}	// onAttach

HRESULT Clone :: onReceive ( IReceptor *pr, const ADTVALUE &v )
	{
	////////////////////////////////////////////////////////////////////////
	//
	//	FROM	IBehaviour
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

	// Fire
	if (_RCP(Fire))
		{
		adtValue	vUse;

		// Value to clone
		CCLTRY ( adtValue::copy ( (!adtValue::empty(vClone)) ? vClone : v,
											vUse ) );

		// Non-object
		if (hr == S_OK && adtValue::type(vUse) != VTYPE_UNK)
			hr = adtValue::copy ( vUse, vRes );

		// Object
		else if (hr == S_OK && adtValue::type(vUse) == VTYPE_UNK)
			{
			ICloneable	*pC	= NULL;
			IUnknown		*pUnk	= NULL;

			// Must support self-cloning
			CCLTRY ( _QISAFE((unkV=vUse),IID_ICloneable,&pC));
			CCLTRY ( pC->clone ( &pUnk ) );

			// Result
			CCLTRY ( adtValue::copy ( adtIUnknown(pUnk), vRes ) );

			// Clean up
			_RELEASE(pUnk);
			_RELEASE(pC);
			}	// else if

		else
			hr = E_UNEXPECTED;

		// Result
		if (hr == S_OK)	_EMT(Fire,vRes);
		else					_EMT(Error,adtInt(hr));
		}	// if

	// State
	else if (_RCP(Value))
		hr = adtValue::copy ( v, vClone );
	else
		hr = ERROR_NO_MATCH;

	return hr;
	}	// receive

