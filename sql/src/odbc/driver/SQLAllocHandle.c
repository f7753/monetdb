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
 * SQLAllocHandle
 * CLI compliance: ISO 92
 *
 * Note: This function also implements the deprecated ODBC functions
 * SQLAllocEnv(), SQLAllocConnect() and SQLAllocStmt()
 * Those functions are simply mapped to this function.
 * All checks and allocation is done in this function.
 *
 * Author: Martin van Dinther
 * Date  : 30 Aug 2002
 *
 **********************************************************************/

#include "ODBCGlobal.h"
#include "ODBCEnv.h"
#include "ODBCDbc.h"
#include "ODBCStmt.h"
#include "ODBCError.h"

SQLRETURN
SQLAllocHandle_(SQLSMALLINT nHandleType, SQLHANDLE nInputHandle,
		SQLHANDLE *pnOutputHandle)
{
	/* Check parameters nHandleType and nInputHandle */
	if (nInputHandle == NULL && nHandleType != SQL_HANDLE_ENV) {
		/* can not set an error message because the handle is NULL */
		return SQL_INVALID_HANDLE;
	}

	switch (nHandleType) {
	case SQL_HANDLE_ENV:
		/* there is no parent handle to test */
		if (pnOutputHandle == NULL)
			return SQL_INVALID_HANDLE;
		*pnOutputHandle = (SQLHANDLE *) newODBCEnv();
		return *pnOutputHandle == NULL ? SQL_ERROR : SQL_SUCCESS;
	case SQL_HANDLE_DBC:
		if (!isValidEnv((ODBCEnv *) nInputHandle))
			return SQL_INVALID_HANDLE;
		clearEnvErrors((ODBCEnv *) nInputHandle);
		if (((ODBCEnv *) nInputHandle)->sql_attr_odbc_version == 0) {
			addEnvError((ODBCEnv *) nInputHandle, "HY010",
				    NULL, 0);
			return SQL_ERROR;
		}
		if (pnOutputHandle == NULL) {
			addEnvError((ODBCEnv *) nInputHandle, "HY009",
				    NULL, 0);
			return SQL_ERROR;
		}
		*pnOutputHandle = (SQLHANDLE *) newODBCDbc((ODBCEnv *) nInputHandle);
		return *pnOutputHandle == NULL ? SQL_ERROR : SQL_SUCCESS;
	case SQL_HANDLE_STMT:
	case SQL_HANDLE_DESC:
		if (!isValidDbc((ODBCDbc *) nInputHandle))
			return SQL_INVALID_HANDLE;
		clearDbcErrors((ODBCDbc *) nInputHandle);
		if (!((ODBCDbc *) nInputHandle)->Connected) {
			addDbcError((ODBCDbc *) nInputHandle, "08003", NULL, 0);
			return SQL_ERROR;
		}
		if (pnOutputHandle == NULL) {
			addDbcError((ODBCDbc *) nInputHandle, "HY009",
				    NULL, 0);
			return SQL_ERROR;
		}
		if (nHandleType == SQL_HANDLE_STMT) {
			*pnOutputHandle = (SQLHANDLE *) newODBCStmt((ODBCDbc *) nInputHandle);
			return *pnOutputHandle == NULL ? SQL_ERROR : SQL_SUCCESS;
		} else {
			*pnOutputHandle = (SQLHANDLE *) newODBCDesc((ODBCDbc *) nInputHandle);
			return *pnOutputHandle == NULL ? SQL_ERROR : SQL_SUCCESS;
		}
	default:
		/* we cannot set an error because we do not know
		   the handle type of the possibly non-null handle */
		return SQL_INVALID_HANDLE;
	}
}

SQLRETURN
SQLAllocHandle(SQLSMALLINT nHandleType,	/* type to be allocated */
	       SQLHANDLE nInputHandle,	/* context for new handle */
	       SQLHANDLE *pnOutputHandle) /* ptr for allocated handle struct */
{
#ifdef ODBCDEBUG
	ODBCLOG("SQLAllocHandle %s\n",
		nHandleType == SQL_HANDLE_ENV ? "Env" :
		nHandleType == SQL_HANDLE_DBC ? "Dbc" :
		nHandleType == SQL_HANDLE_STMT ? "Stmt" : "Desc");
#endif

	return SQLAllocHandle_(nHandleType, nInputHandle, pnOutputHandle);
}
