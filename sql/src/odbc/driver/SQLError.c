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
 * SQLError()
 * CLI Compliance: deprecated in ODBC 3.0 (replaced by SQLGetDiagRec())
 * Provided here for old (pre ODBC 3.0) applications and driver managers.
 *
 * Author: Martin van Dinther
 * Date  : 30 aug 2002
 *
 **********************************************************************/

#include "ODBCGlobal.h"
#include "ODBCEnv.h"
#include "ODBCDbc.h"
#include "ODBCStmt.h"
#include "ODBCUtil.h"

SQLRETURN SQL_API
SQLError(SQLHENV hEnv, SQLHDBC hDbc, SQLHSTMT hStmt, SQLCHAR *szSqlState,
	 SQLINTEGER *pfNativeError, SQLCHAR *szErrorMsg,
	 SQLSMALLINT nErrorMsgMax, SQLSMALLINT *pcbErrorMsg)
{
#ifdef ODBCDEBUG
	ODBCLOG("SQLError\n");
#endif

	/* use mapping as described in ODBC 3 SDK Help file */
	return SQLGetDiagRec_(hStmt ? SQL_HANDLE_STMT :
			      (hDbc ? SQL_HANDLE_DBC : SQL_HANDLE_ENV),
			      hStmt ? hStmt : (hDbc ? hDbc : hEnv),
			      (hStmt ? ++((ODBCStmt *)hStmt)->RetrievedErrors :
			      (hDbc ? ++((ODBCDbc *)hDbc)->RetrievedErrors :
			       ++((ODBCEnv *)hEnv)->RetrievedErrors)),
			      szSqlState, pfNativeError, szErrorMsg,
			      nErrorMsgMax, pcbErrorMsg);
}

SQLRETURN SQL_API
SQLErrorW(SQLHENV hEnv, SQLHDBC hDbc, SQLHSTMT hStmt, SQLWCHAR *szSqlState,
	  SQLINTEGER *pfNativeError, SQLWCHAR *szErrorMsg,
	  SQLSMALLINT nErrorMsgMax, SQLSMALLINT *pcbErrorMsg)
{
	SQLCHAR state[6];
	SQLRETURN rc;
	SQLSMALLINT n;
	SQLCHAR *errmsg;

#ifdef ODBCDEBUG
	ODBCLOG("SQLErrorW\n");
#endif

	errmsg = malloc(nErrorMsgMax * 4);

	/* use mapping as described in ODBC 3 SDK Help file */
	rc = SQLGetDiagRec_(hStmt ? SQL_HANDLE_STMT :
			    (hDbc ? SQL_HANDLE_DBC : SQL_HANDLE_ENV),
			    hStmt ? hStmt : (hDbc ? hDbc : hEnv),
			    (hStmt ? ++((ODBCStmt *)hStmt)->RetrievedErrors :
			     (hDbc ? ++((ODBCDbc *)hDbc)->RetrievedErrors :
			      ++((ODBCEnv *)hEnv)->RetrievedErrors)),
			    state, pfNativeError, errmsg,
			    nErrorMsgMax * 4, &n);

	if (SQL_SUCCEEDED(rc)) {
		char *e = ODBCutf82wchar(state, 5, szSqlState, 6, NULL);
		if (e)
			rc = SQL_ERROR;
	}

	if (SQL_SUCCEEDED(rc)) {
		char *e = ODBCutf82wchar(errmsg, n, szErrorMsg, nErrorMsgMax, &n);
		if (e)
			rc = SQL_ERROR;
		if (pcbErrorMsg)
			*pcbErrorMsg = n;
	}
	free(errmsg);

	return rc;
}
