/*
 * The contents of this file are subject to the MonetDB Public
 * License Version 1.0 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at 
 * http://monetdb.cwi.nl/Legal/MonetDBLicense-1.0.html
 * 
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 * 
 * The Original Code is the Monet Database System.
 * 
 * The Initial Developer of the Original Code is CWI.
 * Portions created by CWI are Copyright (C) 1997-2002 CWI.  
 * All Rights Reserved.
 * 
 * Contributor(s):
 * 		Martin Kersten <Martin.Kersten@cwi.nl>
 * 		Peter Boncz <Peter.Boncz@cwi.nl>
 * 		Niels Nes <Niels.Nes@cwi.nl>
 * 		Stefan Manegold  <Stefan.Manegold@cwi.nl>
 */

/**********************************************************************
 * SQLSetDescRec()
 * CLI Compliance: IOS 92
 **********************************************************************/

#include "ODBCGlobal.h"

SQLRETURN SQLSetDescRec(
	SQLHDESC	hDescriptorHandle,
	SQLSMALLINT	nRecordNumber,
	SQLSMALLINT	nType,
	SQLSMALLINT	nSubType,
	SQLINTEGER	nLength,
	SQLSMALLINT	nPrecision,
	SQLSMALLINT	nScale,
	SQLPOINTER	pData,
	SQLINTEGER *	pnStringLength,
	SQLINTEGER *	pnIndicator )
{
	(void) hDescriptorHandle;	/* Stefan: unused!? */
	(void) nRecordNumber;	/* Stefan: unused!? */
	(void) nType;	/* Stefan: unused!? */
	(void) nSubType;	/* Stefan: unused!? */
	(void) nLength;	/* Stefan: unused!? */
	(void) nPrecision;	/* Stefan: unused!? */
	(void) nScale;	/* Stefan: unused!? */
	(void) pData;	/* Stefan: unused!? */
	(void) pnStringLength;	/* Stefan: unused!? */
	(void) pnIndicator;	/* Stefan: unused!? */

	/* no Descriptors supported (yet) */
	/* no Descriptor handle support, so not possible to set an error */
	/* so return Invalid Handle */
	return SQL_INVALID_HANDLE;
}
