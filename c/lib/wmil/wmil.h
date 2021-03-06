////////////////////////////////////////////////////////////////////////
//
//										WMIL.H
//
//					Windows Management Instrumentation library
//
////////////////////////////////////////////////////////////////////////

#ifndef	WMIL_H
#define	WMIL_H

// System includes
#include "../../lib/nspcl/nspcl.h"

//////////////
// Interfaces
//////////////

///////////
// Classes
///////////

DEFINE_GUID	(	CLSID_Enumerator, 0x2534d096, 0x8628, 0x11d2, 0x86, 0x8c,
					0x00, 0x60, 0x08, 0xad, 0xdf, 0xed );

DEFINE_GUID	(	CLSID_Instance, 0x2534d097, 0x8628, 0x11d2, 0x86, 0x8c,
					0x00, 0x60, 0x08, 0xad, 0xdf, 0xed );

DEFINE_GUID	(	CLSID_Locator, 0x2534d094, 0x8628, 0x11d2, 0x86, 0x8c,
					0x00, 0x60, 0x08, 0xad, 0xdf, 0xed );

DEFINE_GUID	(	CLSID_Service, 0x2534d095, 0x8628, 0x11d2, 0x86, 0x8c,
					0x00, 0x60, 0x08, 0xad, 0xdf, 0xed );

#endif

