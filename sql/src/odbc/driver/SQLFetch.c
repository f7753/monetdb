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
 * SQLFetch()
 * CLI Compliance: ISO 92
 *
 * Author: Martin van Dinther
 * Date  : 30 aug 2002
 *
 **********************************************************************/

#include "ODBCGlobal.h"
#include "ODBCStmt.h"


SQLRETURN
SQLFetch(SQLHSTMT hStmt)
{
	ODBCStmt *stmt = (ODBCStmt *) hStmt;
	int i;

	if (!isValidStmt(stmt))
		 return SQL_INVALID_HANDLE;

	clearStmtErrors(stmt);

	/* check statement cursor state, query should be executed */
	if (stmt->State != EXECUTED) {
		/* caller should have called SQLExecute or SQLExecDirect first */
		/* HY010 = Function sequence error */
		addStmtError(stmt, "HY010", NULL, 0);
		return SQL_ERROR;
	}

	stmt->retrieved = 0;
	stmt->currentCol = 0;

	if (mapi_fetch_row(stmt->Dbc->mid) == 0)
		return SQL_NO_DATA;

	for (i = 0; i < stmt->maxbindings; i++) {
		ODBCBIND *p = &stmt->bindings[i];

		if (p->pTargetValue && p->pszTargetStr) {
			strncpy(p->pTargetValue, p->pszTargetStr,
				p->nTargetValueMax);
			((char *) p->pTargetValue)[p->nTargetValueMax - 1] = '\0';
		}
		if (p->pnLengthOrIndicator && p->pszTargetStr)
			*p->pnLengthOrIndicator = strlen(p->pszTargetStr);
	}

	stmt->currentRow++;

	return SQL_SUCCESS;
}
