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
 * SQLGetData()
 * CLI Compliance: ISO 92
 *
 * Author: Martin van Dinther
 * Date  : 30 aug 2002
 *
 **********************************************************************/

#include "ODBCGlobal.h"
#include "ODBCStmt.h"

SQLRETURN
SQLGetData(SQLHSTMT hStmt, SQLUSMALLINT nCol, SQLSMALLINT nTargetType,
	   SQLPOINTER pTarget, SQLINTEGER nTargetLength,
	   SQLINTEGER *pnLengthOrIndicator)
{
	ODBCStmt *stmt = (ODBCStmt *) hStmt;

#ifdef ODBCDEBUG
	ODBCLOG("SQLGetData %d %d\n", nCol, nTargetType);
#endif

	if (!isValidStmt(stmt))
		 return SQL_INVALID_HANDLE;

	assert(stmt->Dbc);
	assert(stmt->Dbc->mid);
	assert(stmt->hdl);

	clearStmtErrors(stmt);

	/* check statement cursor state, query should be executed */
	if (stmt->State != EXECUTED) {
		/* caller should have called SQLExecute or SQLExecDirect first */
		/* HY010 = Function sequence error */
		addStmtError(stmt, "HY010", NULL, 0);
		return SQL_ERROR;
	}
	if (stmt->currentRow <= 0) {
		/* caller should have called SQLFetch first */
		/* HY010 = Function sequence error */
		addStmtError(stmt, "HY010", NULL, 0);
		return SQL_ERROR;
	}
	if (nCol <= 0 || nCol > stmt->ImplRowDescr->sql_desc_count) {
		/* 07009 = Invalid descriptor index */
		addStmtError(stmt, "07009", NULL, 0);
		return SQL_ERROR;
	}

	if (nCol != stmt->currentCol)
		stmt->retrieved = 0;
	stmt->currentCol = nCol;

	if (nTargetType == SQL_ARD_TYPE) {
		ODBCDesc *desc = stmt->ApplRowDescr;

		if (nCol > desc->sql_desc_count) {
			/* 07009: Invalid descriptor index */
			addStmtError(stmt, "07009", NULL, 0);
			return SQL_ERROR;
		}
		nTargetType = desc->descRec[nCol].sql_desc_concise_type;
	}
	return ODBCFetch(stmt, nCol, nTargetType, pTarget, nTargetLength,
			 pnLengthOrIndicator, pnLengthOrIndicator,
			 UNAFFECTED, UNAFFECTED, UNAFFECTED, 0);
}
