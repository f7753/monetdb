/*
 * This code was created by Peter Harvey (mostly during Christmas 98/99).
 * This code is LGPL. Please ensure that this message remains in future
 * distributions and uses of this code (thats about all I get out of it).
 * - Peter Harvey pharvey@codebydesign.com
 * 
 * This file has been modified for the MonetDB project.  See the file
 * Copyright in this directory for more information.
 */

/**********************************************************************
 * SQLNumParams()
 * CLI Compliance: IOS 92
 *
 * Note: this function is not supported (yet), it returns an error.
 *
 * Author: Martin van Dinther
 * Date  : 30 aug 2002
 *
 **********************************************************************/

#include "ODBCGlobal.h"
#include "ODBCStmt.h"


SQLRETURN
SQLNumParams(SQLHSTMT hStmt, SQLSMALLINT *pnParamCount)
{
	ODBCStmt *stmt = (ODBCStmt *) hStmt;

	(void) pnParamCount;	/* Stefan: unused!? */

	if (!isValidStmt(stmt))
		 return SQL_INVALID_HANDLE;

	clearStmtErrors(stmt);

	/* check statement cursor state, query should be prepared or executed */
	if (stmt->State == INITED) {
		/* HY010 = Function sequence error */
		addStmtError(stmt, "HY010", NULL, 0);

		return SQL_ERROR;
	}


	/* TODO: retrieve the parameter information from stmt->bindParams */

	/* for now return error IM001: driver not capable */
	addStmtError(stmt, "IM001", NULL, 0);
	return SQL_ERROR;
}
