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
 * SQLEndTran()
 * CLI Compliance: ISO 92
 *
 * Note: commit or rollback all open connections on a given environment
 * handle is currently NOT supported, see TODO below.
 *
 * Author: Martin van Dinther
 * Date  : 30 aug 2002
 *
 **********************************************************************/

#include "ODBCGlobal.h"
#include "ODBCEnv.h"
#include "ODBCDbc.h"
#include "ODBCStmt.h"
#include "ODBCError.h"


SQLRETURN
SQLEndTran_(SQLSMALLINT nHandleType, SQLHANDLE nHandle,
	    SQLSMALLINT nCompletionType)
{
	ODBCEnv *env = (ODBCEnv *) nHandle;
	ODBCDbc *dbc = (ODBCDbc *) nHandle;
	SQLHANDLE hStmt;
	RETCODE rc;

	/* check parameters nHandleType and nHandle for validity */
	switch (nHandleType) {
	case SQL_HANDLE_DBC:
		if (!isValidDbc(dbc))
			return SQL_INVALID_HANDLE;
		clearDbcErrors(dbc);
		if (!dbc->Connected) {
			/* Connection not open */
			addDbcError(dbc, "08003", NULL, 0);
			return SQL_ERROR;
		}
		env = NULL;
		break;
	case SQL_HANDLE_ENV:
		if (!isValidEnv(env))
			return SQL_INVALID_HANDLE;
		clearEnvErrors(env);
		dbc = NULL;

		if (env->sql_attr_odbc_version == 0) {
			addEnvError(env, "HY010", NULL, 0);
			return SQL_ERROR;
		}
		break;
	case SQL_HANDLE_STMT:
		if (isValidStmt((ODBCStmt *) nHandle)) {
			clearStmtErrors((ODBCStmt *) nHandle);
			addStmtError((ODBCStmt *) nHandle, "HY092", NULL, 0);
			return SQL_ERROR;
		}
		return SQL_INVALID_HANDLE;
	case SQL_HANDLE_DESC:
		if (isValidDesc((ODBCDesc *) nHandle)) {
			clearDescErrors((ODBCDesc *) nHandle);
			addDescError((ODBCDesc *) nHandle, "HY092", NULL, 0);
			return SQL_ERROR;
		}
		return SQL_INVALID_HANDLE;
	default:
		return SQL_INVALID_HANDLE;
	}

	/* check parameter nCompletionType */
	if (nCompletionType != SQL_COMMIT && nCompletionType != SQL_ROLLBACK) {
		/* HY012 = invalid transaction operation code */
		if (nHandleType == SQL_HANDLE_DBC)
			addDbcError(dbc, "HY012", NULL, 0);
		else
			addEnvError(env, "HY012", NULL, 0);
		return SQL_ERROR;
	}

	if (nHandleType == SQL_HANDLE_ENV) {
		RETCODE rc1 = SQL_SUCCESS;

		for (dbc = env->FirstDbc; dbc; dbc = dbc->next) {
			assert(isValidDbc(dbc));
			if (!dbc->Connected)
				continue;
			rc = SQLEndTran_(SQL_HANDLE_DBC, dbc, nCompletionType);
			if (rc == SQL_ERROR)
				rc1 = SQL_ERROR;
			else if (rc == SQL_SUCCESS_WITH_INFO &&
				 rc1 != SQL_ERROR)
				rc1 = rc;
		}
		return rc1;
	}

	assert(nHandleType == SQL_HANDLE_DBC);

	if (dbc->sql_attr_autocommit == SQL_AUTOCOMMIT_ON) {
		/* nothing to do if in autocommit mode */
		return SQL_SUCCESS;
	}

	/* construct a statement object and excute a SQL COMMIT or ROLLBACK */
	rc = SQLAllocStmt_(dbc, &hStmt);
	if (rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO) {
		ODBCStmt *stmt = (ODBCStmt *) hStmt;
		rc = SQLExecDirect_(stmt,
				    nCompletionType == SQL_COMMIT ?
					(SQLCHAR *) "commit" :
					(SQLCHAR *) "rollback",
				    SQL_NTS);
		if (rc == SQL_ERROR || rc == SQL_SUCCESS_WITH_INFO) {
			/* get the error/warning and post in on the dbc handle */
			SQLCHAR sqlState[SQL_SQLSTATE_SIZE + 1];
			SQLINTEGER nativeErrCode;
			SQLCHAR msgText[SQL_MAX_MESSAGE_LENGTH + 1];

			(void) SQLGetDiagRec_(SQL_HANDLE_STMT, stmt, 1,
					      sqlState, &nativeErrCode,
					      msgText, sizeof(msgText), NULL);

			addDbcError(dbc, (char *) sqlState,
				    (char *) msgText + ODBCErrorMsgPrefixLength,
				    nativeErrCode);
		}
		/* clean up the statement handle */
		SQLFreeStmt_(stmt, SQL_CLOSE);
		ODBCFreeStmt_(stmt);
	} else {
		/* could not allocate a statement object */
		addDbcError(dbc, "HY013", NULL, 0);
		return SQL_ERROR;
	}

	return rc;
}

SQLRETURN SQL_API
SQLEndTran(SQLSMALLINT nHandleType, SQLHANDLE nHandle,
	   SQLSMALLINT nCompletionType)
{
#ifdef ODBCDEBUG
	ODBCLOG("SQLEndTran\n");
#endif

	return SQLEndTran_(nHandleType, nHandle, nCompletionType);
}
