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
 * SQLBindParameter()
 * CLI Compliance: ODBC (Microsoft)
 *
 * Note: this function is not supported (yet), it returns an error.
 * So parametrized SQL commands are not possible!
 * TODO: implement this function and corresponding behavior in
 * SQLPrepare() and SQLExecute().
 *
 * Author: Martin van Dinther
 * Date  : 30 Aug 2002
 *
 **********************************************************************/

#include "ODBCGlobal.h"
#include "ODBCStmt.h"

SQLRETURN
SQLBindParameter_(ODBCStmt *stmt, SQLUSMALLINT ParameterNumber,
		  SQLSMALLINT InputOutputType, SQLSMALLINT ValueType,
		  SQLSMALLINT ParameterType, SQLUINTEGER ColumnSize,
		  SQLSMALLINT DecimalDigits, SQLPOINTER ParameterValuePtr,
		  SQLINTEGER BufferLength, SQLINTEGER *StrLen_or_IndPtr)
{
	ODBCDesc *apd, *ipd;
	ODBCDescRec *apdrec, *ipdrec;
	SQLRETURN rc;
	MapiMsg ret = MOK;

	if (!isValidStmt(stmt))
		 return SQL_INVALID_HANDLE;

	clearStmtErrors(stmt);

	/* check input parameters */
	if (ParameterNumber <= 0) {
		/* Invalid descriptor index */
		addStmtError(stmt, "07009", NULL, 0);
		return SQL_ERROR;
	}
	/* For safety: limit the maximum number of columns to bind */
	if (ParameterNumber > MONETDB_MAX_BIND_COLS) {
		/* HY000 = General Error */
		addStmtError(stmt, "HY000",
			     "Maximum number of bind columns (8192) exceeded",
			     0);
		return SQL_ERROR;
	}

	switch (InputOutputType) {
	case SQL_PARAM_INPUT:
		break;
	case SQL_PARAM_INPUT_OUTPUT:
	case SQL_PARAM_OUTPUT:
		/* Optional feature not implemented */
		addStmtError(stmt, "HYC00",
			     "Output parameters are not supported", 0);
		return SQL_ERROR;
	default:
		/* Invalid parameter type */
		addStmtError(stmt, "HY105", NULL, 0);
		return SQL_ERROR;
	}

	if (ParameterValuePtr == NULL && StrLen_or_IndPtr == NULL
	    /* && InputOutputType != SQL_PARAM_OUTPUT */ ) {
		/* Invalid use of null pointer */
		addStmtError(stmt, "HY009", NULL, 0);
		return SQL_ERROR;
	}

	if (BufferLength < 0) {
		/* Invalid string or buffer length */
		addStmtError(stmt, "HY090", NULL, 0);
		return SQL_ERROR;
	}

	/* can't let SQLSetDescField below do this check since it
	   returns the wrong error code if the type is incorrect */
	switch (ValueType) {
	case SQL_C_CHAR:
	case SQL_C_WCHAR:
	case SQL_C_BINARY:
	case SQL_C_BIT:
	case SQL_C_STINYINT:
	case SQL_C_UTINYINT:
	case SQL_C_TINYINT:
	case SQL_C_SSHORT:
	case SQL_C_USHORT:
	case SQL_C_SHORT:
	case SQL_C_SLONG:
	case SQL_C_ULONG:
	case SQL_C_LONG:
	case SQL_C_SBIGINT:
	case SQL_C_UBIGINT:
	case SQL_C_NUMERIC:
	case SQL_C_FLOAT:
	case SQL_C_DOUBLE:
	case SQL_C_TYPE_DATE:
	case SQL_C_TYPE_TIME:
	case SQL_C_TYPE_TIMESTAMP:
	case SQL_C_INTERVAL_YEAR:
	case SQL_C_INTERVAL_MONTH:
	case SQL_C_INTERVAL_YEAR_TO_MONTH:
	case SQL_C_INTERVAL_DAY:
	case SQL_C_INTERVAL_HOUR:
	case SQL_C_INTERVAL_MINUTE:
	case SQL_C_INTERVAL_SECOND:
	case SQL_C_INTERVAL_DAY_TO_HOUR:
	case SQL_C_INTERVAL_DAY_TO_MINUTE:
	case SQL_C_INTERVAL_DAY_TO_SECOND:
	case SQL_C_INTERVAL_HOUR_TO_MINUTE:
	case SQL_C_INTERVAL_HOUR_TO_SECOND:
	case SQL_C_INTERVAL_MINUTE_TO_SECOND:
	case SQL_C_GUID:
	case SQL_C_DEFAULT:
		break;
	default:
		/* Invalid application buffer type */
		addStmtError(stmt, "HY003", NULL, 0);
		return SQL_ERROR;
	}

	apd = stmt->ApplParamDescr;
	ipd = stmt->ImplParamDescr;
	apdrec = addODBCDescRec(apd, ParameterNumber);
	ipdrec = addODBCDescRec(ipd, ParameterNumber);

	switch (ParameterType) {
	case SQL_CHAR:
	case SQL_VARCHAR:
	case SQL_LONGVARCHAR:
	case SQL_BINARY:
	case SQL_VARBINARY:
	case SQL_LONGVARBINARY:
	case SQL_TYPE_DATE:
	case SQL_INTERVAL_MONTH:
	case SQL_INTERVAL_YEAR:
	case SQL_INTERVAL_YEAR_TO_MONTH:
	case SQL_INTERVAL_DAY:
	case SQL_INTERVAL_HOUR:
	case SQL_INTERVAL_MINUTE:
	case SQL_INTERVAL_DAY_TO_HOUR:
	case SQL_INTERVAL_DAY_TO_MINUTE:
	case SQL_INTERVAL_HOUR_TO_MINUTE:
		ipdrec->sql_desc_length = ColumnSize;
		break;
	case SQL_TYPE_TIME:
	case SQL_TYPE_TIMESTAMP:
	case SQL_INTERVAL_SECOND:
	case SQL_INTERVAL_DAY_TO_SECOND:
	case SQL_INTERVAL_HOUR_TO_SECOND:
	case SQL_INTERVAL_MINUTE_TO_SECOND:
		ipdrec->sql_desc_precision = DecimalDigits;
		ipdrec->sql_desc_length = ColumnSize;
		break;
	case SQL_DECIMAL:
	case SQL_NUMERIC:
		ipdrec->sql_desc_precision = (SQLSMALLINT) ColumnSize;
		ipdrec->sql_desc_scale = DecimalDigits;
		break;
	case SQL_FLOAT:
	case SQL_REAL:
	case SQL_DOUBLE:
		ipdrec->sql_desc_precision = (SQLSMALLINT) ColumnSize;
		break;
	default:
		/* Invalid SQL data type */
		addStmtError(stmt, "HY004", NULL, 0);
		return SQL_ERROR;
	}

	rc = SQLSetDescField_(apd, ParameterNumber,
			      SQL_DESC_CONCISE_TYPE,
			      (SQLPOINTER) (ssize_t) ValueType, 0);
	if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)
		return rc;
	rc = SQLSetDescField_(ipd, ParameterNumber,
			      SQL_DESC_CONCISE_TYPE,
			      (SQLPOINTER) (ssize_t) ParameterType, 0);
	if (rc != SQL_SUCCESS && rc != SQL_SUCCESS_WITH_INFO)
		return rc;
	ipdrec->sql_desc_parameter_type = InputOutputType;
	apdrec->sql_desc_data_ptr = ParameterValuePtr;
	apdrec->sql_desc_octet_length = BufferLength;
	apdrec->sql_desc_indicator_ptr = StrLen_or_IndPtr;
	apdrec->sql_desc_octet_length_ptr = StrLen_or_IndPtr;

	switch (ValueType) {
	case SQL_C_CHAR:
		/* note about the cast: on a system with
		   sizeof(long)==8, SQLINTEGER is typedef'ed as int,
		   otherwise as long, but on those other systems, long
		   and int are the same size, so the cast works */
		ret = mapi_param_string(stmt->hdl, ParameterNumber - 1,
					ParameterType, ParameterValuePtr,
					(int *) StrLen_or_IndPtr);
		break;
	case SQL_C_SSHORT:
		ret = mapi_param_type(stmt->hdl, ParameterNumber - 1,
				      MAPI_SHORT, ParameterType,
				      ParameterValuePtr);
		break;
	case SQL_C_USHORT:
		ret = mapi_param_type(stmt->hdl, ParameterNumber - 1,
				      MAPI_USHORT, ParameterType,
				      ParameterValuePtr);
		break;
	case SQL_C_SLONG:
		ret = mapi_param_type(stmt->hdl, ParameterNumber - 1,
				      MAPI_LONG, ParameterType,
				      ParameterValuePtr);
		break;
	case SQL_C_ULONG:
		ret = mapi_param_type(stmt->hdl, ParameterNumber - 1,
				      MAPI_ULONG, ParameterType,
				      ParameterValuePtr);
		break;
	case SQL_C_STINYINT:
		ret = mapi_param_type(stmt->hdl, ParameterNumber - 1,
				      MAPI_TINY, ParameterType,
				      ParameterValuePtr);
		break;
	case SQL_C_UTINYINT:
		ret = mapi_param_type(stmt->hdl, ParameterNumber - 1,
				      MAPI_UTINY, ParameterType,
				      ParameterValuePtr);
		break;
	case SQL_C_SBIGINT:
		ret = mapi_param_type(stmt->hdl, ParameterNumber - 1,
				      MAPI_LONGLONG, ParameterType,
				      ParameterValuePtr);
		break;
	case SQL_C_UBIGINT:
		ret = mapi_param_type(stmt->hdl, ParameterNumber - 1,
				      MAPI_ULONGLONG, ParameterType,
				      ParameterValuePtr);
		break;
	case SQL_C_FLOAT:
		ret = mapi_param_type(stmt->hdl, ParameterNumber - 1,
				      MAPI_FLOAT, ParameterType,
				      ParameterValuePtr);
		break;
	case SQL_C_DOUBLE:
		ret = mapi_param_type(stmt->hdl, ParameterNumber - 1,
				      MAPI_DOUBLE, ParameterType,
				      ParameterValuePtr);
		break;
	case SQL_C_TYPE_DATE:
		ret = mapi_param_type(stmt->hdl, ParameterNumber - 1,
				      MAPI_DATE, ParameterType,
				      ParameterValuePtr);
		break;
	case SQL_C_TYPE_TIME:
		ret = mapi_param_type(stmt->hdl, ParameterNumber - 1,
				      MAPI_TIME, ParameterType,
				      ParameterValuePtr);
		break;
	case SQL_C_TYPE_TIMESTAMP:
		ret = mapi_param_type(stmt->hdl, ParameterNumber - 1,
				      MAPI_DATETIME, ParameterType,
				      ParameterValuePtr);
		break;
	case SQL_C_DEFAULT:
		/* these are supported */
		break;

	case SQL_C_BINARY:
	case SQL_C_WCHAR:
	case SQL_C_NUMERIC:
	case SQL_C_GUID:
		/* these are NOT supported */
		addStmtError(stmt, "IM001", NULL, 0);
		return SQL_ERROR;
	default:
		/* HY003 = Invalid application buffer type */
		addStmtError(stmt, "HY003", NULL, 0);
		return SQL_ERROR;
	}

	if (ret == MOK)
		return stmt->Error ? SQL_SUCCESS_WITH_INFO : SQL_SUCCESS;

	addStmtError(stmt, "HY000", mapi_error_str(stmt->Dbc->mid), 0);
	return SQL_ERROR;
}

SQLRETURN
SQLBindParameter(SQLHSTMT hStmt, SQLUSMALLINT ParameterNumber,
		 SQLSMALLINT InputOutputType, SQLSMALLINT ValueType,
		 SQLSMALLINT ParameterType, SQLUINTEGER ColumnSize,
		 SQLSMALLINT DecimalDigits, SQLPOINTER ParameterValuePtr,
		 SQLINTEGER BufferLength, SQLINTEGER *StrLen_or_IndPtr)
{
#ifdef ODBCDEBUG
	ODBCLOG("SQLBindParameter %d %d %d %d %d %d\n", ParameterNumber,
		InputOutputType, ValueType, ParameterType, ColumnSize,
		DecimalDigits);
#endif

	return SQLBindParameter_((ODBCStmt *) hStmt, ParameterNumber,
				 InputOutputType, ValueType, ParameterType,
				 ColumnSize, DecimalDigits, ParameterValuePtr,
				 BufferLength, StrLen_or_IndPtr);
}
