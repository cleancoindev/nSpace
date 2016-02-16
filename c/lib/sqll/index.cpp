////////////////////////////////////////////////////////////////////////
//
//									INDEX.CPP
//
//					Implementation of the SQL table index node
//
////////////////////////////////////////////////////////////////////////

/*
   Copyright (c) 2003, nSpace, LLC
   All right reserved

   Redistribution and use in source and binary forms, with or without 
   modification, are permitted provided that the following conditions
   are met:

      - Redistributions of source code must retain the above copyright 
        notice, this list of conditions and the following disclaimer.
      - nSpace, LLC as the copyright holder reserves the right to 
        maintain, update, and provide future releases of the source code.
      - The copyrights to all improvements and modifications to this 
        distribution shall reside in nSpace, LLC.
      - Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in 
        the documentation and/or other materials provided with the 
        distribution.
      - Neither the name of nSpace, LLC nor the names of its contributors 
        may be used to endorse or promote products derived from this 
        software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT 
   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSBILITY OF SUCH DAMAGE.
*/

#include "sqln_.h"
#include <stdio.h>

#define	SIZE_SQLBFR		1024

#ifdef	USE_ODBC

// Globals

SQL2Index :: SQL2Index ( void )
	{
	////////////////////////////////////////////////////////////////////////
	//
	//	PURPOSE
	//		-	Constructor for the node
	//
	////////////////////////////////////////////////////////////////////////
	pFlds			= NULL;
	pConn			= NULL;
	hConn			= NULL;
	pQryBfr		= NULL;
	pwQryBfr		= NULL;
	}	// SQL2Index

HRESULT SQL2Index :: construct ( void )
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
	HRESULT		hr			= S_OK;

	// SQL query string buffer
	CCLTRY ( COCREATEINSTANCE ( CLSID_MemoryBlock, IID_IMemoryMapped, &pQryBfr ) );
	CCLTRY ( pQryBfr->setSize ( SIZE_SQLBFR ) );
	CCLTRY ( pQryBfr->lock ( 0, 0, (PVOID *) &pwQryBfr, NULL ) );

	return hr;
	}	// construct

void SQL2Index :: destruct ( void )
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
	_RELEASE(pFlds);
	_RELEASE(pConn);
	_UNLOCK(pQryBfr,pwQryBfr);
	_RELEASE(pQryBfr);
	}	// destruct

HRESULT SQL2Index :: receive ( IReceptor *pR, const adtValue &v )
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

	// Create
	if (prCr == pR)
		{
		SQLHANDLE	hStmt	= NULL;
		IADTIt		*pIt	= NULL;
		IADTInIt		*pIn	= NULL;
		adtString	sStr;

		// State check
		CCLTRYE ( (hConn != NULL),		ERROR_INVALID_STATE );
		CCLTRYE ( pFlds != NULL,		ERROR_INVALID_STATE );
		if (hr == S_OK && sTableName.length() == 0)
			hr = pnAttr->load ( strRefTableName, sTableName );
		CCLTRYE ( (sTableName.length() > 0),	ERROR_INVALID_STATE );
		if (hr == S_OK && sIndexName.length() == 0)
			hr = pnAttr->load ( strRefIndexName, sIndexName );
		CCLTRYE ( (sIndexName.length() > 0),	ERROR_INVALID_STATE );

		// Create index
		CCLOK ( swprintf ( pwQryBfr, L"CREATE UNIQUE INDEX \"%s\" ON %s (",
									(LPCWSTR)(sIndexName), (LPCWSTR)(sTableName) ); )

		// Field names
		CCLTRY(pFlds->iterate ( &pIt ));
		CCLTRY(_QI(pIt,IID_IADTInIt,&pIn));
		CCLTRY(pIn->begin());
		while (hr == S_OK && pIn->read ( sStr ) == S_OK)
			{
			// Add
			CCLOK ( wcscat ( pwQryBfr, sStr ); )
			CCLOK ( wcscat ( pwQryBfr, L"," ); )

			// Clean up
			pIn->next();
			}	// while

		// Terminate
		CCLOK ( pwQryBfr[wcslen(pwQryBfr)-1] = WCHAR(')'); )

		// Execute
		CCLTRY ( SQLSTMT(hStmt,(SQLAllocHandle ( SQL_HANDLE_STMT, hConn, &hStmt ))) );
		CCLOK  ( OutputDebugString ( pwQryBfr ); )
		CCLOK  ( OutputDebugString ( L"\n" ); )
		CCLOK  ( SQLSTMT(hStmt,(SQLExecDirect ( hStmt, pwQryBfr, (SQLINTEGER)wcslen(pwQryBfr) )));)
		SQLFREESTMT(hStmt);

		// Result
		CCLOK  ( peFire->emit ( adtInt() ); )

		// Clean up
		_RELEASE(pIn);
		_RELEASE(pIt);
		}	// if

	// Connection
	else if (prConn == pR)
		{
		HRESULT		hr = S_OK;
		adtIUnknown unkV(v);
		adtLong		lTmp;

		// Previous connection
		_RELEASE(pConn);
		hConn = NULL;

		// New connection
		CCLTRY(_QISAFE(unkV,IID_IHaveValue,&pConn));
		CCLTRY(pConn->getValue ( lTmp ));
		CCLOK (hConn = (SQLHANDLE)(U64)lTmp;)
		}	// else if

	// Fields
	else if (prFlds == pR)
		{
		HRESULT		hr = S_OK;
		adtIUnknown unkV(v);
		_RELEASE(pFlds);
		CCLTRY(_QISAFE(unkV,IID_IADTContainer,&pFlds));
		}	// else if

	// State
	else if (prTbl == pR)
		hr = adtValueImpl::copy ( sTableName, adtString(v) );

	return hr;
	}	// receive

#endif