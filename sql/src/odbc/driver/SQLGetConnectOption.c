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
 * SQLGetConnectOption()
 * CLI Compliance: deprecated in ODBC 3.0 (replaced by SQLGetConnectAttr())
 * Provided here for old (pre ODBC 3.0) applications and driver managers.
 **********************************************************************/

#include "ODBCGlobal.h"
#include "ODBCDbc.h"
#include "ODBCUtil.h"

static SQLRETURN
SQLGetConnectOption_(ODBCDbc *dbc, SQLUSMALLINT nOption, SQLPOINTER pvParam)
{
	/* use mapping as described in ODBC 3 SDK Help file */
	switch (nOption) {
		/* connection attributes (ODBC 1 and 2 only) */
	case SQL_ACCESS_MODE:
	case SQL_AUTOCOMMIT:
	case SQL_LOGIN_TIMEOUT:
	case SQL_ODBC_CURSORS:
	case SQL_OPT_TRACE:
	case SQL_PACKET_SIZE:
	case SQL_QUIET_MODE:
	case SQL_TRANSLATE_OPTION:
	case SQL_TXN_ISOLATION:
		/* 32 bit integer argument */
		return SQLGetConnectAttr_(dbc, nOption, pvParam, 0, NULL);
	case SQL_CURRENT_QUALIFIER:
	case SQL_OPT_TRACEFILE:
	case SQL_TRANSLATE_DLL:
		/* null terminated string argument */
		return SQLGetConnectAttr_(dbc, nOption, pvParam,
					  SQL_MAX_OPTION_STRING_LENGTH, NULL);
	default:
		/* set error: Invalid attribute/option */
		addDbcError(dbc, "HY092", NULL, 0);
		return SQL_ERROR;
	}

	return SQL_ERROR;
}

SQLRETURN SQL_API
SQLGetConnectOption(SQLHDBC hDbc, SQLUSMALLINT nOption, SQLPOINTER pvParam)
{
	ODBCDbc *dbc = (ODBCDbc *) hDbc;

#ifdef ODBCDEBUG
	ODBCLOG("SQLGetConnectOption\n");
#endif

	if (!isValidDbc(dbc))
		return SQL_INVALID_HANDLE;
	clearDbcErrors(dbc);

	return SQLGetConnectOption_(dbc, nOption, pvParam);
}

SQLRETURN SQL_API
SQLGetConnectOptionW(SQLHDBC hDbc, SQLUSMALLINT nOption, SQLPOINTER pvParam)
{
	ODBCDbc * dbc = (ODBCDbc *) hDbc;
	SQLRETURN rc;
	SQLPOINTER ptr;

#ifdef ODBCDEBUG
	ODBCLOG("SQLGetConnectOptionW\n");
#endif

	if (!isValidDbc(dbc))
		return SQL_INVALID_HANDLE;

	clearDbcErrors(dbc);

	switch (nOption) {
	/* all string attributes */
	case SQL_CURRENT_QUALIFIER:
	case SQL_OPT_TRACEFILE:
	case SQL_TRANSLATE_DLL:
		ptr = malloc(SQL_MAX_OPTION_STRING_LENGTH);
		break;
	default:
		ptr = pvParam;
		break;
	}

	rc = SQLGetConnectOption_(dbc, nOption, ptr);

	if (ptr != pvParam) {
		SQLSMALLINT n = strlen((char *) ptr);
		SQLSMALLINT *nullp = NULL;

		fixWcharOut(rc, ptr, n, pvParam, SQL_MAX_OPTION_STRING_LENGTH, nullp, addDbcError, dbc);
	}

	return rc;
}
