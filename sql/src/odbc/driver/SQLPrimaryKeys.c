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
 * SQLPrimaryKeys()
 * CLI Compliance: ODBC (Microsoft)
 *
 * Note: catalogs are not supported, we ignore any value set for
 * szCatalogName.
 *
 * Author: Martin van Dinther
 * Date  : 30 aug 2002
 *
 **********************************************************************/

#include "ODBCGlobal.h"
#include "ODBCStmt.h"
#include "ODBCUtil.h"

static SQLRETURN
SQLPrimaryKeys_(ODBCStmt *stmt,
		SQLCHAR *szCatalogName, SQLSMALLINT nCatalogNameLength,
		SQLCHAR *szSchemaName, SQLSMALLINT nSchemaNameLength,
		SQLCHAR *szTableName, SQLSMALLINT nTableNameLength)
{
	RETCODE rc;

	/* buffer for the constructed query to do meta data retrieval */
	char *query = NULL;
	char *query_end = NULL;	/* pointer to end of built-up query */

	/* check statement cursor state, no query should be prepared or executed */
	if (stmt->State == EXECUTED) {
		/* 24000 = Invalid cursor state */
		addStmtError(stmt, "24000", NULL, 0);
		return SQL_ERROR;
	}

	/* deal with SQL_NTS and SQL_NULL_DATA */
	fixODBCstring(szCatalogName, nCatalogNameLength, addStmtError, stmt);
	fixODBCstring(szSchemaName, nSchemaNameLength, addStmtError, stmt);
	fixODBCstring(szTableName, nTableNameLength, addStmtError, stmt);

	/* check if a valid (non null, not empty) table name is supplied */
	if (szTableName == NULL) {
		/* HY009 = Invalid use of null pointer */
		addStmtError(stmt, "HY009", NULL, 0);
		return SQL_ERROR;
	}

#ifdef ODBCDEBUG
	ODBCLOG("\".*s\" \".*s\" \".*s\"\n",
		nCatalogNameLength, szCatalogName,
		nSchemaNameLength, szSchemaName,
		nTableNameLength, szTableName);
#endif

	/* construct the query */
	query = (char *) malloc(1000 + nTableNameLength + nSchemaNameLength);
	assert(query);
	query_end = query;

	/* SQLPrimaryKeys returns a table with the following columns:
	   VARCHAR	table_cat
	   VARCHAR	table_schem
	   VARCHAR	table_name NOT NULL
	   VARCHAR	column_name NOT NULL
	   SMALLINT	key_seq NOT NULL
	   VARCHAR	pk_name
	 */
	strcpy(query_end,
	       "select "
	       "cast('' as varchar) as table_cat, "
	       "cast(s.name as varchar) as table_schem, "
	       "cast(t.name as varchar) as table_name, "
	       "cast(c.name as varchar) as column_name, "
	       "cast(kc.ordinal_position as smallint) as key_seq, "
	       "cast(k.key_name as varchar) as pk_name "
	       "from sys.schemas s, sys.tables t, columns c, keys k, keycolumns kc "
	       "where s.id = t.schema_id and t.id = c.table_id and "
	       "t.id = k.table_id and c.id = kc.column_id and "
	       "kc.key_id = k.key_id and k.is_primary = 1");
	query_end += strlen(query_end);

	/* Construct the selection condition query part */
	/* search pattern is not allowed for table name so use = and not LIKE */
	sprintf(query_end, " and t.name = '%.*s'",
		nTableNameLength, szTableName);
	query_end += strlen(query_end);

	if (szSchemaName != NULL) {
		/* filtering requested on schema name */
		/* search pattern is not allowed so use = and not LIKE */
		sprintf(query_end, " and s.name = '%.*s'",
			nSchemaNameLength, szSchemaName);
		query_end += strlen(query_end);
	}

	/* add the ordering */
	strcpy(query_end, " order by table_cat, table_schem, table_name, key_seq");
	query_end += strlen(query_end);

	/* query the MonetDb data dictionary tables */
	rc = SQLExecDirect_(stmt, (SQLCHAR *) query,
			    (SQLINTEGER) (query_end - query));

	free(query);

	return rc;
}

SQLRETURN SQL_API
SQLPrimaryKeys(SQLHSTMT hStmt,
	       SQLCHAR *szCatalogName, SQLSMALLINT nCatalogNameLength,
	       SQLCHAR *szSchemaName, SQLSMALLINT nSchemaNameLength,
	       SQLCHAR *szTableName, SQLSMALLINT nTableNameLength)
{
	ODBCStmt *stmt = (ODBCStmt *) hStmt;

#ifdef ODBCDEBUG
	ODBCLOG("SQLPrimaryKeys " PTRFMT " ", PTRFMTCAST hStmt);
#endif

	if (!isValidStmt(stmt))
		 return SQL_INVALID_HANDLE;

	clearStmtErrors(stmt);

	return SQLPrimaryKeys_(stmt, szCatalogName, nCatalogNameLength,
			       szSchemaName, nSchemaNameLength,
			       szTableName, nTableNameLength);
}

#ifdef WITH_WCHAR
SQLRETURN SQL_API
SQLPrimaryKeysW(SQLHSTMT hStmt,
		SQLWCHAR *szCatalogName, SQLSMALLINT nCatalogNameLength,
		SQLWCHAR *szSchemaName, SQLSMALLINT nSchemaNameLength,
		SQLWCHAR *szTableName, SQLSMALLINT nTableNameLength)
{
	ODBCStmt *stmt = (ODBCStmt *) hStmt;
	SQLRETURN rc = SQL_ERROR;
	SQLCHAR *catalog = NULL, *schema = NULL, *table = NULL;

#ifdef ODBCDEBUG
	ODBCLOG("SQLPrimaryKeysW " PTRFMT " ", PTRFMTCAST hStmt);
#endif

	if (!isValidStmt(stmt))
		 return SQL_INVALID_HANDLE;

	clearStmtErrors(stmt);

	fixWcharIn(szCatalogName, nCatalogNameLength, catalog, addStmtError, stmt, goto exit);
	fixWcharIn(szSchemaName, nSchemaNameLength, schema, addStmtError, stmt, goto exit);
	fixWcharIn(szTableName, nTableNameLength, table, addStmtError, stmt, goto exit);

	rc = SQLPrimaryKeys_(stmt, catalog, SQL_NTS, schema, SQL_NTS,
			     table, SQL_NTS);

  exit:
	if (catalog)
		free(catalog);
	if (schema)
		free(schema);
	if (table)
		free(table);

	return rc;
}
#endif	/* WITH_WCHAR */
