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
 * SQLGetStmtAttr()
 * CLI Compliance: ISO 92
 *
 * Author: Martin van Dinther
 * Date  : 30 aug 2002
 *
 **********************************************************************/

#include "ODBCGlobal.h"
#include "ODBCStmt.h"


SQLRETURN
SQLGetStmtAttr_(ODBCStmt *stmt, SQLINTEGER Attribute, SQLPOINTER Value,
		SQLINTEGER BufferLength, SQLINTEGER *StringLength)
{
	if (!isValidStmt(stmt))
		 return SQL_INVALID_HANDLE;

	clearStmtErrors(stmt);

	/* TODO: check parameters: Value, BufferLength and StringLength */

	switch (Attribute) {
	case SQL_ATTR_APP_PARAM_DESC:
		* (SQLHANDLE *) Value = stmt->ApplParamDescr;
		return SQL_SUCCESS;
	case SQL_ATTR_APP_ROW_DESC:
		* (SQLHANDLE *) Value = stmt->ApplRowDescr;
		return SQL_SUCCESS;
	case SQL_ATTR_ASYNC_ENABLE:
		* (SQLUINTEGER *) Value = SQL_ASYNC_ENABLE_OFF;
		break;
	case SQL_ATTR_CONCURRENCY:
		* (SQLUINTEGER *) Value = SQL_CONCUR_READ_ONLY;
		break;
	case SQL_ATTR_CURSOR_SCROLLABLE:
		* (SQLUINTEGER *) Value = stmt->cursorScrollable;
		break;
	case SQL_ATTR_CURSOR_SENSITIVITY:
		* (SQLUINTEGER *) Value = SQL_INSENSITIVE;
		break;
	case SQL_ATTR_CURSOR_TYPE:
		* (SQLUINTEGER *) Value = stmt->cursorType;
		break;
	case SQL_ATTR_IMP_PARAM_DESC:
		* (SQLHANDLE *) Value = stmt->ImplParamDescr;
		return SQL_SUCCESS;
	case SQL_ATTR_IMP_ROW_DESC:
		* (SQLHANDLE *) Value = stmt->ImplRowDescr;
		return SQL_SUCCESS;
	case SQL_ATTR_MAX_LENGTH:
		* (SQLUINTEGER *) Value = 0;
		break;
	case SQL_ATTR_MAX_ROWS:
		* (SQLUINTEGER *) Value = 0;
		break;
	case SQL_ATTR_PARAM_BIND_OFFSET_PTR:
		return SQLGetDescField_(stmt->ApplParamDescr, 0,
					SQL_DESC_BIND_OFFSET_PTR,
					Value, BufferLength, StringLength);
	case SQL_ATTR_PARAM_BIND_TYPE:
		return SQLGetDescField_(stmt->ApplParamDescr, 0,
					SQL_DESC_BIND_TYPE,
					Value, BufferLength, StringLength);
	case SQL_ATTR_PARAM_OPERATION_PTR:
		return SQLGetDescField_(stmt->ApplParamDescr, 0,
					SQL_DESC_ARRAY_STATUS_PTR,
					Value, BufferLength, StringLength);
	case SQL_ATTR_PARAM_STATUS_PTR:
		return SQLGetDescField_(stmt->ImplParamDescr, 0,
					SQL_DESC_ARRAY_STATUS_PTR,
					Value, BufferLength, StringLength);
	case SQL_ATTR_PARAMS_PROCESSED_PTR:
		return SQLGetDescField_(stmt->ImplParamDescr, 0,
					SQL_DESC_ROWS_PROCESSED_PTR,
					Value, BufferLength, StringLength);
	case SQL_ATTR_PARAMSET_SIZE:
		return SQLGetDescField_(stmt->ApplParamDescr, 0,
					SQL_DESC_ARRAY_SIZE,
					Value, BufferLength, StringLength);
	case SQL_ATTR_RETRIEVE_DATA:
		* (SQLUINTEGER *) Value = stmt->retrieveData;
		break;
	case SQL_ATTR_ROW_ARRAY_SIZE:
		return SQLGetDescField_(stmt->ApplRowDescr, 0,
					SQL_DESC_ARRAY_SIZE,
					Value, BufferLength, StringLength);
	case SQL_ATTR_ROW_BIND_OFFSET_PTR:
		return SQLGetDescField_(stmt->ApplRowDescr, 0,
					SQL_DESC_BIND_OFFSET_PTR,
					Value, BufferLength, StringLength);
	case SQL_ATTR_ROW_BIND_TYPE:
		return SQLGetDescField_(stmt->ApplRowDescr, 0,
					SQL_DESC_BIND_TYPE,
					Value, BufferLength, StringLength);
	case SQL_ATTR_ROW_NUMBER:
		* (SQLUINTEGER *) Value = stmt->currentRow;
		break;
	case SQL_ATTR_ROW_OPERATION_PTR:
		return SQLGetDescField_(stmt->ApplRowDescr, 0,
					SQL_DESC_ARRAY_STATUS_PTR,
					Value, BufferLength, StringLength);
	case SQL_ATTR_ROW_STATUS_PTR:
		return SQLGetDescField_(stmt->ImplRowDescr, 0,
					SQL_DESC_ARRAY_STATUS_PTR,
					Value, BufferLength, StringLength);
	case SQL_ATTR_ROWS_FETCHED_PTR:
		return SQLGetDescField_(stmt->ImplRowDescr, 0,
					SQL_DESC_ROWS_PROCESSED_PTR,
					Value, BufferLength, StringLength);

	/* TODO: implement requested behavior */
	case SQL_ATTR_ENABLE_AUTO_IPD:
	case SQL_ATTR_FETCH_BOOKMARK_PTR:
	case SQL_ATTR_KEYSET_SIZE:
	case SQL_ATTR_METADATA_ID:
	case SQL_ATTR_NOSCAN:
	case SQL_ATTR_QUERY_TIMEOUT:
	case SQL_ATTR_SIMULATE_CURSOR:
	case SQL_ATTR_USE_BOOKMARKS:
		/* return error: Optional feature not supported */
		addStmtError(stmt, "HYC00", NULL, 0);
		return SQL_ERROR;
	default:
		/* return error: Invalid option/attribute identifier */
		addStmtError(stmt, "HY092", NULL, 0);
		return SQL_ERROR;
	}

	return SQL_SUCCESS;
}

SQLRETURN
SQLGetStmtAttr(SQLHSTMT hStmt, SQLINTEGER Attribute, SQLPOINTER Value,
	       SQLINTEGER BufferLength, SQLINTEGER *StringLength)
{
#ifdef ODBCDEBUG
	ODBCLOG("SQLGetStmtAttr %d\n", Attribute);
#endif

	return SQLGetStmtAttr_((ODBCStmt *) hStmt, Attribute, Value,
			       BufferLength, StringLength);
}
