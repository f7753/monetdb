#include "ODBCGlobal.h"
#include "ODBCStmt.h"

#define ODBC_DESC_MAGIC_NR	21845 /* for internal sanity check only */

/*
 * Creates a new allocated ODBCDesc object and initializes it.
 *
 * Precondition: valid ODBCDbc object
 * Postcondition: returns a new ODBCDesc object
 */
ODBCDesc *
newODBCDesc(ODBCDbc *dbc)
{
	ODBCDesc *desc = malloc(sizeof(ODBCDesc));

	assert(desc);
	assert(dbc);

	if (desc == NULL) {
		/* HY001: Memory allocation error */
		addDbcError(dbc, "HY001", NULL, 0);
		return NULL;
	}

	desc->Dbc = dbc;
	desc->Error = NULL;
	desc->RetrievedErrors = 0;
	desc->Stmt = NULL;
	desc->descRec = NULL;
	desc->sql_desc_alloc_type = SQL_DESC_ALLOC_USER;
	desc->sql_desc_array_size = 1;
	desc->sql_desc_array_status_ptr = NULL;
	desc->sql_desc_bind_offset_ptr = NULL;
	desc->sql_desc_bind_type = SQL_BIND_TYPE_DEFAULT;
	desc->sql_desc_count = 0;
	desc->sql_desc_rows_processed_ptr = NULL;

	desc->Type = ODBC_DESC_MAGIC_NR; /* set it valid */
	return desc;
}


/*
 * Check if the descriptor handle is valid.
 * Note: this function is used internally by the driver to assert legal
 * and save usage of the handle and prevent crashes as much as possible.
 *
 * Precondition: none
 * Postcondition: returns 1 if it is a valid statement handle,
 * 	returns 0 if is invalid and thus an unusable handle.
 */
int
isValidDesc(ODBCDesc *desc)
{
#ifdef ODBCDEBUG
	if (!(desc && desc->Type == ODBC_DESC_MAGIC_NR))
		ODBCLOG("not a valid descriptor handle\n");
#endif
	return desc && desc->Type == ODBC_DESC_MAGIC_NR;
}

/*
 * Creates and adds an error msg object to the end of the error list of
 * this ODBCDesc struct.
 * When the errMsg is NULL and the SQLState is an ISO SQLState the
 * standard ISO message text for the SQLState is used as message.
 *
 * Precondition: desc must be valid. SQLState and errMsg may be NULL.
 */
void
addDescError(ODBCDesc *desc, const char *SQLState, const char *errMsg,
	     int nativeErrCode)
{
	ODBCError *error = NULL;

#ifdef ODBCDEBUG
	extern const char * getStandardSQLStateMsg(const char *);
	ODBCLOG("addDescError %s %s %d\n", SQLState,
		errMsg ? errMsg : getStandardSQLStateMsg(SQLState),
		nativeErrCode);
#endif
	assert(isValidDesc(desc));

	error = newODBCError(SQLState, errMsg, nativeErrCode);
	appendODBCError(&desc->Error, error);
}

/*
 * Extracts an error object from the error list of this ODBCDesc struct.
 * The error object itself is removed from the error list.
 * The caller is now responsible for freeing the error object memory.
 *
 * Precondition: desc and error must be valid
 * Postcondition: returns a ODBCError object or null when no error is available.
 */
ODBCError *
getDescError(ODBCDesc *desc)
{
	assert(isValidDesc(desc));

	return desc->Error;
}

static void
cleanODBCDescRec(ODBCDesc *desc, ODBCDescRec *rec)
{
	if (rec->sql_desc_base_column_name)
		free(rec->sql_desc_base_column_name);
	if (rec->sql_desc_base_table_name)
		free(rec->sql_desc_base_table_name);
	if (rec->sql_desc_catalog_name)
		free(rec->sql_desc_catalog_name);
	if (rec->sql_desc_label)
		free(rec->sql_desc_label);
	if (rec->sql_desc_literal_prefix)
		free(rec->sql_desc_literal_prefix);
	if (rec->sql_desc_literal_suffix)
		free(rec->sql_desc_literal_suffix);
	if (rec->sql_desc_local_type_name)
		free(rec->sql_desc_local_type_name);
	if (rec->sql_desc_name)
		free(rec->sql_desc_name);
	if (rec->sql_desc_schema_name)
		free(rec->sql_desc_schema_name);
	if (rec->sql_desc_table_name)
		free(rec->sql_desc_table_name);
	if (rec->sql_desc_type_name)
		free(rec->sql_desc_type_name);
	memset(rec, 0, sizeof(*rec));
	if (desc) {
		if (isAD(desc)) {
			rec->sql_desc_concise_type = SQL_C_DEFAULT;
			rec->sql_desc_type = SQL_C_DEFAULT;
		} else if (isIPD(desc))
			rec->sql_desc_parameter_type = SQL_PARAM_INPUT;
	}
}

void
setODBCDescRecCount(ODBCDesc *desc, int count)
{
	assert(count >= 0);
	assert(desc->sql_desc_count >= 0);

	if (count == desc->sql_desc_count)
		return;
	if (count < desc->sql_desc_count) {
		int i;

		for (i = count + 1; i <= desc->sql_desc_count; i++)
			cleanODBCDescRec(NULL, &desc->descRec[i]);
	}
	if (count == 0) {
		assert(desc->descRec != NULL);
		free(desc->descRec);
		desc->descRec = NULL;
	} else if (desc->descRec == NULL) {
		assert(desc->sql_desc_count == 0);
		desc->descRec = malloc((count + 1) * sizeof(*desc->descRec));
	} else {
		assert(desc->sql_desc_count > 0);
		desc->descRec = realloc(desc->descRec,
					(count + 1) * sizeof(*desc->descRec));
	}
	if (count > desc->sql_desc_count) {
		int i;
		memset(desc->descRec + desc->sql_desc_count + 1, 0,
		       count - desc->sql_desc_count);
		if (isAD(desc)) {
			for (i = desc->sql_desc_count + 1; i <= count; i++) {
				desc->descRec[i].sql_desc_concise_type = SQL_C_DEFAULT;
				desc->descRec[i].sql_desc_type = SQL_C_DEFAULT;
			}
		} else if (isIPD(desc)) {
			for (i = desc->sql_desc_count + 1; i <= count; i++)
				desc->descRec[i].sql_desc_parameter_type = SQL_PARAM_INPUT;
		}
	}
	desc->sql_desc_count = count;
}

/*
 * Destroys the ODBCDesc object including its own managed data.
 *
 * Precondition: desc must be valid and inactive (internal State == INITED or
 * State == PREPARED, so NO active result set).
 * Postcondition: desc is completely destroyed, desc handle is become invalid.
 */
void
destroyODBCDesc(ODBCDesc *desc)
{
	assert(isValidDesc(desc));

	desc->Type = 0;
	deleteODBCErrorList(&desc->Error);
	setODBCDescRecCount(desc, 0);
	free(desc);
}

ODBCDescRec *
addODBCDescRec(ODBCDesc *desc, SQLSMALLINT recno)
{
	assert(desc);
	assert(recno > 0);

	if (recno < desc->sql_desc_count)
		setODBCDescRecCount(desc, recno);
	else {
		assert(desc->descRec != NULL);
		cleanODBCDescRec(desc, &desc->descRec[recno]);
	}

	return &desc->descRec[recno];
}
