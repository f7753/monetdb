package nl.cwi.monetdb.jdbc;

import java.sql.*;
import java.util.*;

/**
 * A DatabaseMetaData object suitable for the Monet database
 * <br /><br />
 *
 * @author Fabian Groffen <Fabian.Groffen@cwi.nl>
 * @version 0.1 (beta release)
 */
public class MonetDatabaseMetaData implements DatabaseMetaData {
	private Connection con;
	private Driver driver;
	private Statement stmt;

	public MonetDatabaseMetaData(Connection parent) {
		con = parent;
		driver = new MonetDriver();
	}

	private synchronized Statement getStmt() throws SQLException {
		if (stmt == null) stmt = con.createStatement();

		return(stmt);
	}

	/**
	 * Can all the procedures returned by getProcedures be called
	 * by the current user?
	 *
	 * @return true if so
	 */
	public boolean allProceduresAreCallable() {
		return(true);
	}

	/**
	 * Can all the tables returned by getTable be SELECTed by
	 * the current user?
	 *
	 * @return true because we only have one user a.t.m.
	 */
	public boolean allTablesAreSelectable() {
		return(true);
	}

	/**
	 * What is the URL for this database?
	 *
	 * @return null because it cannot be generated a.t.m.
	 */
	public String getURL() {
		return(null);
	}

	/**
	 * What is our user name as known to the database?
	 *
	 * @return null because it doesn't matter a.t.m.
	 */
	public String getUserName() {
		return(null);
	}

	/**
	 * Is the database in read-only mode?
	 *
	 * @return always false for now
	 */
	public boolean isReadOnly() {
		return(false);
	}

	/**
	 * Are NULL values sorted high?
	 *
	 * @return true because Monet puts NULL values on top upon ORDER BY
	 */
	public boolean nullsAreSortedHigh() {
		return(true);
	}

	/**
	 * Are NULL values sorted low?
	 *
	 * @return negative of nullsAreSortedHigh()
	 * @see nullsAreSortedHigh()
	 */
	public boolean nullsAreSortedLow() {
		return(!nullsAreSortedHigh());
	}

	/**
	 * Are NULL values sorted at the start regardless of sort order?
	 *
	 * @return false, since Monet doesn't do this
	 */
	public boolean nullsAreSortedAtStart() {
		return(false);
	}

	/**
	 * Are NULL values sorted at the end regardless of sort order?
	 *
	 * @return false, since Monet doesn't do this
	 */
	public boolean nullsAreSortedAtEnd() {
		return(false);
	}

	/**
	 * What is the name of this database product - this should be MonetDB
	 * of course, so we return that explicitly.
	 *
	 * @return the database product name
	 */
	public String getDatabaseProductName() {
		return "MonetDB";
	}

	/**
	 * What is the version of this database product.
	 *
	 * @return a fixed version number, yes it's quick and dirty
	 */
	public String getDatabaseProductVersion() {
		return("Generation 4");
	}

	/**
	 * What is the name of this JDBC driver?  If we don't know this
	 * we are doing something wrong!
	 *
	 * @return the JDBC driver name
	 */
	public String getDriverName() {
		return("MonetDB Native Driver");
	}

	/**
	 * What is the version string of this JDBC driver?	Again, this is
	 * static.
	 *
	 * @return the JDBC driver name.
	 */
	public String getDriverVersion() {
		return(((MonetDriver)driver).getDriverVersion());
	}

	/**
	 * What is this JDBC driver's major version number?
	 *
	 * @return the JDBC driver major version
	 */
	public int getDriverMajorVersion() {
		return(driver.getMajorVersion());
	}

	/**
	 * What is this JDBC driver's minor version number?
	 *
	 * @return the JDBC driver minor version
	 */
	public int getDriverMinorVersion() {
		return(driver.getMinorVersion());
	}

	/**
	 * Does the database store tables in a local file?	No - it
	 * stores them in a file on the server.
	 *
	 * @return false because that's what Monet is for
	 */
	public boolean usesLocalFiles() {
		return(false);
	}

	/**
	 * Does the database use a local file for each table?  Well, not really,
	 * since it doesn't use local files.
	 *
	 * @return false for it doesn't
	 */
	public boolean usesLocalFilePerTable() {
		return(false);
	}

	/**
	 * Does the database treat mixed case unquoted SQL identifiers
	 * as case sensitive and as a result store them in mixed case?
	 * A JDBC-Compliant driver will always return false.
	 *
	 * @return false
	 */
	public boolean supportsMixedCaseIdentifiers() {
		return(false);
	}

	/**
	 * Does the database treat mixed case unquoted SQL identifiers as
	 * case insensitive and store them in upper case?
	 *
	 * @return true if so
	 */
	public boolean storesUpperCaseIdentifiers() {
		return(false);
	}

	/**
	 * Does the database treat mixed case unquoted SQL identifiers as
	 * case insensitive and store them in lower case?
	 *
	 * @return true if so
	 */
	public boolean storesLowerCaseIdentifiers() {
		return(true);
	}

	/**
	 * Does the database treat mixed case unquoted SQL identifiers as
	 * case insensitive and store them in mixed case?
	 *
	 * @return true if so
	 */
	public boolean storesMixedCaseIdentifiers() {
		return(false);
	}

	/**
	 * Does the database treat mixed case quoted SQL identifiers as
	 * case sensitive and as a result store them in mixed case?  A
	 * JDBC compliant driver will always return true.
	 *
	 * @return true if so
	 */
	public boolean supportsMixedCaseQuotedIdentifiers() {
		return(true);
	}

	/**
	 * Does the database treat mixed case quoted SQL identifiers as
	 * case insensitive and store them in upper case?
	 *
	 * @return true if so
	 */
	public boolean storesUpperCaseQuotedIdentifiers() {
		return(false);
	}

	/**
	 * Does the database treat mixed case quoted SQL identifiers as case
	 * insensitive and store them in lower case?
	 *
	 * @return true if so
	 */
	public boolean storesLowerCaseQuotedIdentifiers() {
		return(false);
	}

	/**
	 * Does the database treat mixed case quoted SQL identifiers as case
	 * insensitive and store them in mixed case?
	 *
	 * @return true if so
	 */
	public boolean storesMixedCaseQuotedIdentifiers() {
		return(false);
	}

	/**
	 * What is the string used to quote SQL identifiers?  This returns
	 * a space if identifier quoting isn't supported.  A JDBC Compliant
	 * driver will always use a double quote character.
	 *
	 * @return the quoting string
	 */
	public String getIdentifierQuoteString() {
		return("\"");
	}

	/**
	 * Get a comma separated list of all a database's SQL keywords that
	 * are NOT also SQL92 keywords.
	 * <br /><br />
	 * Wasn't MonetDB fully standards compliant? So no extra keywords...
	 *
	 * @return a comma separated list of keywords we use (none)
	 */
	public String getSQLKeywords() {
		return("");
	}

	public String getNumericFunctions() {
		return("");
	}

	public String getStringFunctions() {
		return("");
	}

	public String getSystemFunctions() {
		return("");
	}

	public String getTimeDateFunctions() {
		return("");
	}

	/**
	 * This is the string that can be used to escape '_' and '%' in
	 * a search string pattern style catalog search parameters
	 *
	 * @return the string used to escape wildcard characters
	 */
	public String getSearchStringEscape() {
		return("\\");
	}

	/**
	 * Get all the "extra" characters that can be used in unquoted
	 * identifier names (those beyond a-zA-Z0-9 and _)
	 * Monet has no extras
	 *
	 * @return a string containing the extra characters
	 */
	public String getExtraNameCharacters() {
		return("");
	}

	/**
	 * Is "ALTER TABLE" with an add column supported?
	 *
	 * @return true if so
	 */
	public boolean supportsAlterTableWithAddColumn() {
		return(true);
	}

	/**
	 * Is "ALTER TABLE" with a drop column supported?
	 *
	 * @return true if so
	 * @exception SQLException if a database access error occurs
	 */
	public boolean supportsAlterTableWithDropColumn() {
		return(true);
	}

	/**
	 * Is column aliasing supported?
	 *
	 * <p>If so, the SQL AS clause can be used to provide names for
	 * computed columns or to provide alias names for columns as
	 * required.  A JDBC Compliant driver always returns true.
	 *
	 * <p>e.g.
	 *
	 * <br><pre>
	 * select count(C) as C_COUNT from T group by C;
	 *
	 * </pre><br>
	 * should return a column named as C_COUNT instead of count(C)
	 *
	 * @return true if so
	 * @exception SQLException if a database access error occurs
	 */
	public boolean supportsColumnAliasing() {
		return(true);
	}

	/**
	 * Are concatenations between NULL and non-NULL values NULL? A
	 * JDBC Compliant driver always returns true
	 *
	 * @return true if so
	 * @exception SQLException if a database access error occurs
	 */
	public boolean nullPlusNonNullIsNull() {
		return(true);
	}

	public boolean supportsConvert() {
		return(false);
	}

	public boolean supportsConvert(int fromType, int toType) {
		return(false);
	}

	/**
	 * Are table correlation names supported? A JDBC Compliant
	 * driver always returns true.
	 *
	 * @return true if so
	 */
	public boolean supportsTableCorrelationNames() {
		return(true);
	}

	/**
	 * If table correlation names are supported, are they restricted to
	 * be different from the names of the tables?
	 *
	 * @return true if so; false otherwise
	 */
	public boolean supportsDifferentTableCorrelationNames() {
		return(false);
	}

	/**
	 * Are expressions in "ORDER BY" lists supported?
	 *
	 * <br>e.g. select * from t order by a + b;
	 *
	 * MonetDB does not support this (yet?)
	 *
	 * @return true if so
	 */
	public boolean supportsExpressionsInOrderBy() {
		return(false);
	}

	/**
	 * Can an "ORDER BY" clause use columns not in the SELECT?
	 * Monet = SQL99 = false
	 *
	 * @return true if so
	 */
	public boolean supportsOrderByUnrelated() {
		return(false);
	}

	/**
	 * Is some form of "GROUP BY" clause supported?
	 *
	 * @return true since MonetDB supports it
	 * @exception SQLException if a database access error occurs
	 */
	public boolean supportsGroupBy() {
		return(true);
	}

	/**
	 * Can a "GROUP BY" clause use columns not in the SELECT?
	 *
	 * @return true since that also is supported
	 * @exception SQLException if a database access error occurs
	 */
	public boolean supportsGroupByUnrelated() {
		return(true);
	}

	/**
	 * Can a "GROUP BY" clause add columns not in the SELECT provided
	 * it specifies all the columns in the SELECT?
	 *
	 * (Monet already supports the more difficult supportsGroupByUnrelated(),
	 * so this is a piece of cake)
	 *
	 * @return true if so
	 */
	public boolean supportsGroupByBeyondSelect() {
		return(true);
	}

	/**
	 * Is the escape character in "LIKE" clauses supported?  A
	 * JDBC compliant driver always returns true.
	 *
	 * @return true if so
	 */
	public boolean supportsLikeEscapeClause() {
		return(true);
	}

	/**
	 * Are multiple ResultSets from a single execute supported?
	 * MonetDB probably supports it, but I don't
	 *
	 * @return true if so
	 */
	public boolean supportsMultipleResultSets() {
		return(false);
	}

	/**
	 * Can we have multiple transactions open at once (on different
	 * connections?)
	 * This is the main idea behind the Connection, is it?
	 *
	 * @return true if so
	 * @exception SQLException if a database access error occurs
	 */
	public boolean supportsMultipleTransactions() {
		return(true);
	}

	/**
	 * Can columns be defined as non-nullable.	A JDBC Compliant driver
	 * always returns true.
	 *
	 * @return true if so
	 */
	public boolean supportsNonNullableColumns() {
		return(true);
	}

	/**
	 * Does this driver support the minimum ODBC SQL grammar.  This
	 * grammar is defined at:
	 *
	 * http://msdn.microsoft.com/library/default.asp?url=/library/en-us/odbc/htm/odappcpr.asp
	 * From this description, we seem to support the ODBC minimal (Level 0) grammar.
	 *
	 * @return true if so
	 */
	public boolean supportsMinimumSQLGrammar() {
		return(true);
	}

	/**
	 * Does this driver support the Core ODBC SQL grammar.	We need
	 * SQL-92 conformance for this.
	 *
	 * @return true if so
	 */
	public boolean supportsCoreSQLGrammar() {
		return(true);
	}

	/**
	 * Does this driver support the Extended (Level 2) ODBC SQL
	 * grammar.  We don't conform to the Core (Level 1), so we can't
	 * conform to the Extended SQL Grammar.
	 *
	 * @return true if so
	 */
	public boolean supportsExtendedSQLGrammar() {
		return(false);
	}

	/**
	 * Does this driver support the ANSI-92 entry level SQL grammar?
	 * All JDBC Compliant drivers must return true. We should be this
	 * compliant, so let's 'act' like we are.
	 *
	 * @return true if so
	 */
	public boolean supportsANSI92EntryLevelSQL() {
		return(true);
	}

	/**
	 * Does this driver support the ANSI-92 intermediate level SQL
	 * grammar?
	 * probably not
	 *
	 * @return true if so
	 */
	public boolean supportsANSI92IntermediateSQL() {
		return(false);
	}

	/**
	 * Does this driver support the ANSI-92 full SQL grammar?
	 * Would be good if it was like that
	 *
	 * @return true if so
	 * @exception SQLException if a database access error occurs
	 */
	public boolean supportsANSI92FullSQL() {
		return(false);
	}

	/**
	 * Is the SQL Integrity Enhancement Facility supported?
	 * Our best guess is that this means support for constraints
	 *
	 * @return true if so
	 */
	public boolean supportsIntegrityEnhancementFacility() {
		return(true);
	}

	/**
	 * Is some form of outer join supported?
	 *
	 * @return true if so
	 * @exception SQLException if a database access error occurs
	 */
	public boolean supportsOuterJoins(){
		return(true);
	}

	/**
	 * Are full nexted outer joins supported?
	 *
	 * @return true if so
	 * @exception SQLException if a database access error occurs
	 */
	public boolean supportsFullOuterJoins() {
		return(true);
	}

	/**
	 * Is there limited support for outer joins?
	 *
	 * @return true if so
	 * @exception SQLException if a database access error occurs
	 */
	public boolean supportsLimitedOuterJoins() {
		return(false);
	}

	/**
	 * What is the database vendor's preferred term for "schema"?
	 * MonetDB uses the term "schema".
	 *
	 * @return the vendor term
	 */
	public String getSchemaTerm() {
		return("schema");
	}

	/**
	 * What is the database vendor's preferred term for "procedure"?
	 * Traditionally, "function" has been used.
	 *
	 * @return the vendor term
	 */
	public String getProcedureTerm() {
		return("function");
	}

	/**
	 * What is the database vendor's preferred term for "catalog"?
	 * MonetDB doesn't really have them (from driver accessible) but
	 * from the monetdb.conf file the term "database" sounds best
	 *
	 * @return the vendor term
	 */
	public String getCatalogTerm() {
		return("database");
	}

	/**
	 * Does a catalog appear at the start of a qualified table name?
	 * (Otherwise it appears at the end).
	 * Currently there is no catalog support at all in MonetDB
	 *
	 * @return true if so
	 */
	public boolean isCatalogAtStart() {
		// return true here; we return false for every other catalog function
		// so it won't matter what we return here
		return(true);
	}

	/**
	 * What is the Catalog separator.
	 *
	 * @return the catalog separator string
	 */
	public String getCatalogSeparator() {
		// Give them something to work with here
		// everything else returns false so it won't matter what we return here
		return(".");
	}

	/**
	 * Can a schema name be used in a data manipulation statement?
	 *
	 * @return true if so
	 */
	public boolean supportsSchemasInDataManipulation() {
		return(true);
	}

	/**
	 * Can a schema name be used in a procedure call statement?
	 * Ohw probably, but I don't know of procedures in Monet
	 *
	 * @return true if so
	 */
	public boolean supportsSchemasInProcedureCalls() {
		return(true);
	}

	/**
	 * Can a schema be used in a table definition statement?
	 *
	 * @return true if so
	 */
	public boolean supportsSchemasInTableDefinitions() {
		return(true);
	}

	/**
	 * Can a schema name be used in an index definition statement?
	 *
	 * @return true if so
	 */
	public boolean supportsSchemasInIndexDefinitions() {
		return(true);
	}

	/**
	 * Can a schema name be used in a privilege definition statement?
	 *
	 * @return true if so
	 */
	public boolean supportsSchemasInPrivilegeDefinitions() {
		return(true);
	}

	/**
	 * Can a catalog name be used in a data manipulation statement?
	 *
	 * @return true if so
	 */
	public boolean supportsCatalogsInDataManipulation() {
		return(false);
	}

	/**
	 * Can a catalog name be used in a procedure call statement?
	 *
	 * @return true if so
	 */
	public boolean supportsCatalogsInProcedureCalls() {
		return(false);
	}

	/**
	 * Can a catalog name be used in a table definition statement?
	 *
	 * @return true if so
	 */
	public boolean supportsCatalogsInTableDefinitions() {
		return(false);
	}

	/**
	 * Can a catalog name be used in an index definition?
	 *
	 * @return true if so
	 */
	public boolean supportsCatalogsInIndexDefinitions() {
		return(false);
	}

	/**
	 * Can a catalog name be used in a privilege definition statement?
	 *
	 * @return true if so
	 */
	public boolean supportsCatalogsInPrivilegeDefinitions() {
		return(false);
	}

	/**
	 * MonetDB doesn't support positioned DELETEs I guess
	 *
	 * @return true if so
	 */
	public boolean supportsPositionedDelete() {
		return(false);
	}

	/**
	 * Is positioned UPDATE supported? (same as above)
	 *
	 * @return true if so
	 */
	public boolean supportsPositionedUpdate() {
		return(false);
	}

	/**
	 * Is SELECT FOR UPDATE supported?
	 * My test resulted in a negative answer
	 *
	 * @return true if so; false otherwise
	 */
	public boolean supportsSelectForUpdate(){
		return(false);
	}

	/**
	 * Are stored procedure calls using the stored procedure escape
	 * syntax supported?
	 *
	 * @return true if so; false otherwise
	 */
	public boolean supportsStoredProcedures() {
		return(false);
	}

	/**
	 * Are subqueries in comparison expressions supported? A JDBC
	 * Compliant driver always returns true. MonetDB also supports this
	 *
	 * @return true if so; false otherwise
	 */
	public boolean supportsSubqueriesInComparisons() {
		return(true);
	}

	/**
	 * Are subqueries in 'exists' expressions supported? A JDBC
	 * Compliant driver always returns true.
	 *
	 * @return true if so; false otherwise
	 */
	public boolean supportsSubqueriesInExists() {
		return(true);
	}

	/**
	 * Are subqueries in 'in' statements supported? A JDBC
	 * Compliant driver always returns true.
	 *
	 * @return true if so; false otherwise
	 */
	public boolean supportsSubqueriesInIns() {
		return(true);
	}

	/**
	 * Are subqueries in quantified expressions supported? A JDBC
	 * Compliant driver always returns true.
	 *
	 * (No idea what this is, but we support a good deal of
	 * subquerying.)
	 *
	 * @return true if so; false otherwise
	 */
	public boolean supportsSubqueriesInQuantifieds() {
		return(true);
	}

	/**
	 * Are correlated subqueries supported? A JDBC Compliant driver
	 * always returns true.
	 *
	 * (a.k.a. subselect in from?)
	 *
	 * @return true if so; false otherwise
	 */
	public boolean supportsCorrelatedSubqueries() {
		return(true);
	}

	/**
	 * Is SQL UNION supported?
	 * since 2004-03-20
	 *
	 * @return true if so
	 */
	public boolean supportsUnion() {
		return(true);
	}

	/**
	 * Is SQL UNION ALL supported?
	 * since 2004-03-20
	 *
	 * @return true if so
	 */
	public boolean supportsUnionAll() {
		return(true);
	}

	/**
	 * ResultSet objects (cursors) are not closed upon explicit or
	 * implicit commit.
	 *
	 * @return true if so
	 */
	public boolean supportsOpenCursorsAcrossCommit() {
		return(true);
	}

	/**
	 * Same as above
	 *
	 * @return true if so
	 */
	public boolean supportsOpenCursorsAcrossRollback() {
		return(true);
	}

	/**
	 * Can statements remain open across commits?  They may, but
	 * this driver cannot guarentee that.  In further reflection.
	 * we are taking a Statement object here, so the answer is
	 * yes, since the Statement is only a vehicle to execute some SQL
	 *
	 * @return true if they always remain open; false otherwise
	 */
	public boolean supportsOpenStatementsAcrossCommit() {
		return(true);
	}

	/**
	 * Can statements remain open across rollbacks?  They may, but
	 * this driver cannot guarentee that.  In further contemplation,
	 * we are taking a Statement object here, so the answer is yes again.
	 *
	 * @return true if they always remain open; false otherwise
	 */
	public boolean supportsOpenStatementsAcrossRollback() {
		return(true);
	}

	/**
	 * How many hex characters can you have in an inline binary literal
	 * I honestly wouldn't know...
	 *
	 * @return the max literal length
	 */
	public int getMaxBinaryLiteralLength() {
		return(0); // no limit
	}

	/**
	 * What is the maximum length for a character literal
	 * Is there a max?
	 *
	 * @return the max literal length
	 */
	public int getMaxCharLiteralLength() {
		return(0); // no limit
	}

	/**
	 * Whats the limit on column name length.
	 * I take some safety here, but it's just a varchar in MonetDB
	 *
	 * @return the maximum column name length
	 */
	public int getMaxColumnNameLength() {
		return(256);
	}

	/**
	 * What is the maximum number of columns in a "GROUP BY" clause?
	 *
	 * @return the max number of columns
	 */
	public int getMaxColumnsInGroupBy() {
		return(0); // no limit
	}

	/**
	 * What's the maximum number of columns allowed in an index?
	 *
	 * @return max number of columns
	 */
	public int getMaxColumnsInIndex() {
		return(0);	// unlimited I guess
	}

	/**
	 * What's the maximum number of columns in an "ORDER BY clause?
	 *
	 * @return the max columns
	 */
	public int getMaxColumnsInOrderBy() {
		return(0); // unlimited I guess
	}

	/**
	 * What is the maximum number of columns in a "SELECT" list?
	 *
	 * @return the max columns
	 */
	public int getMaxColumnsInSelect() {
		return(0); // unlimited I guess
	}

	/**
	 * What is the maximum number of columns in a table?
	 * wasn't Monet designed for datamining? (= much columns)
	 *
	 * @return the max columns
	 */
	public int getMaxColumnsInTable() {
		return(0);
	}

	/**
	 * How many active connections can we have at a time to this
	 * database?  Well, since it depends on Mserver, which just listens
	 * for new connections and creates a new thread for each connection,
	 * this number can be very high, and theoretically till the system
	 * runs out of resources. However, knowing Monet is knowing that you
	 * should handle it a little bit with care, so I give a very minimalistic
	 * number here.
	 *
	 * @return the maximum number of connections
	 */
	public int getMaxConnections() {
		return(16);
	}

	/**
	 * What is the maximum cursor name length
	 * Actually we do not do named cursors, so I keep the value small as
	 * a precaution for maybe the future.
	 *
	 * @return max cursor name length in bytes
	 */
	public int getMaxCursorNameLength() {
		return(256);
	}

	/**
	 * Retrieves the maximum number of bytes for an index, including all
	 * of the parts of the index.
	 *
	 * @return max index length in bytes, which includes the composite
	 *         of all the constituent parts of the index; a result of zero
	 *         means that there is no limit or the limit is not known
	 */
	public int getMaxIndexLength() {
		return(0); // I assume it is large, but I don't know
	}

	/**
	 * Retrieves the maximum number of characters that this database
	 * allows in a schema name.
	 *
	 * @return the number of characters or 0 if there is no limit, or the
	 *         limit is unknown.
	 */
	public int getMaxSchemaNameLength() {
		return(0);	// I don't know...
	}

	/**
	 * What is the maximum length of a procedure name
	 *
	 * @return the max name length in bytes
	 */
	public int getMaxProcedureNameLength() {
		return(0);	// unknown
	}

	/**
	 * What is the maximum length of a catalog
	 *
	 * @return the max length
	 */
	public int getMaxCatalogNameLength() {
		return(0);
	}

	/**
	 * What is the maximum length of a single row?
	 *
	 * @return max row size in bytes
	 */
	public int getMaxRowSize() {
		return(0);	// very long I hope...
	}

	/**
	 * Did getMaxRowSize() include LONGVARCHAR and LONGVARBINARY
	 * blobs?
	 * Yes I thought so...
	 *
	 * @return true if so
	 */
	public boolean doesMaxRowSizeIncludeBlobs() {
		return(true);
	}

	/**
	 * What is the maximum length of a SQL statement?
	 * Till a programmer makes a mistake and causes a segmentation fault
	 * on a string overflow...
	 *
	 * @return max length in bytes
	 */
	public int getMaxStatementLength() {
		return(0);		// actually whatever fits in size_t
	}

	/**
	 * How many active statements can we have open at one time to
	 * this database?  Basically, since each Statement downloads
	 * the results as the query is executed, we can have many.
	 *
	 * @return the maximum
	 */
	public int getMaxStatements() {
		return(0);
	}

	/**
	 * What is the maximum length of a table name
	 *
	 * @return max name length in bytes
	 */
	public int getMaxTableNameLength() {
		return(0);
	}

	/**
	 * What is the maximum number of tables that can be specified
	 * in a SELECT?
	 *
	 * @return the maximum
	 */
	public int getMaxTablesInSelect() {
		return(0); // no limit
	}

	/**
	 * What is the maximum length of a user name
	 *
	 * @return the max name length in bytes
	 * @exception SQLException if a database access error occurs
	 */
	public int getMaxUserNameLength() {
		return(0);	// no limit
	}

	/**
	 * What is the database's default transaction isolation level?
	 * We only see commited data, nonrepeatable reads and phantom
	 * reads can occur.
	 *
	 * @return the default isolation level
	 * @see Connection
	 */
	public int getDefaultTransactionIsolation() {
		return(Connection.TRANSACTION_SERIALIZABLE);
	}

	/**
	 * Are transactions supported?	If not, commit and rollback are noops
	 * and the isolation level is TRANSACTION_NONE.  We do support
	 * transactions.
	 *
	 * @return true if transactions are supported
	 */
	public boolean supportsTransactions() {
		return(true);
	}

	/**
	 * Does the database support the given transaction isolation level?
	 * We only support TRANSACTION_READ_COMMITTED as far as I know
	 *
	 * @param level the values are defined in java.sql.Connection
	 * @return true if so
	 * @see Connection
	 */
	public boolean supportsTransactionIsolationLevel(int level) {
		return(level == Connection.TRANSACTION_SERIALIZABLE);
	}

	/**
	 * Are both data definition and data manipulation transactions
	 * supported?
	 * Supposedly that data definition is like CREATE or ALTER TABLE
	 * yes it is.
	 *
	 * @return true if so
	 */
	public boolean supportsDataDefinitionAndDataManipulationTransactions() {
		return(true);
	}

	/**
	 * Are only data manipulation statements withing a transaction
	 * supported?
	 *
	 * @return true if so
	 */
	public boolean supportsDataManipulationTransactionsOnly() {
		return(false);
	}

	/**
	 * Does a data definition statement within a transaction force
	 * the transaction to commit?  I think this means something like:
	 *
	 * <p><pre>
	 * CREATE TABLE T (A INT);
	 * INSERT INTO T (A) VALUES (2);
	 * BEGIN;
	 * UPDATE T SET A = A + 1;
	 * CREATE TABLE X (A INT);
	 * SELECT A FROM T INTO X;
	 * COMMIT;
	 * </pre><p>
	 *
	 * does the CREATE TABLE call cause a commit?  The answer is no.
	 *
	 * @return true if so
	 */
	public boolean dataDefinitionCausesTransactionCommit() {
		return(false);
	}

	/**
	 * Is a data definition statement within a transaction ignored?
	 *
	 * @return true if so
	 * @exception SQLException if a database access error occurs
	 */
	public boolean dataDefinitionIgnoredInTransactions() {
		return(false);
	}

	/**
	 * Get a description of stored procedures available in a catalog
	 * Currently not applicable and not implemented, returns null
	 *
	 * <p>Only procedure descriptions matching the schema and procedure
	 * name criteria are returned.	They are ordered by PROCEDURE_SCHEM
	 * and PROCEDURE_NAME
	 *
	 * <p>Each procedure description has the following columns:
	 * <ol>
	 * <li><b>PROCEDURE_CAT</b> String => procedure catalog (may be null)
	 * <li><b>PROCEDURE_SCHEM</b> String => procedure schema (may be null)
	 * <li><b>PROCEDURE_NAME</b> String => procedure name
	 * <li><b>Field 4</b> reserved (make it null)
	 * <li><b>Field 5</b> reserved (make it null)
	 * <li><b>Field 6</b> reserved (make it null)
	 * <li><b>REMARKS</b> String => explanatory comment on the procedure
	 * <li><b>PROCEDURE_TYPE</b> short => kind of procedure
	 *	<ul>
	 *	  <li> procedureResultUnknown - May return a result
	 *	<li> procedureNoResult - Does not return a result
	 *	<li> procedureReturnsResult - Returns a result
	 *	  </ul>
	 * </ol>
	 *
	 * @param catalog - a catalog name; "" retrieves those without a
	 *	catalog; null means drop catalog name from criteria
	 * @param schemaParrern - a schema name pattern; "" retrieves those
	 *	without a schema - we ignore this parameter
	 * @param procedureNamePattern - a procedure name pattern
	 * @return ResultSet - each row is a procedure description
	 * @throws SQLException if a database access error occurs
	 */
	public ResultSet getProcedures(
		String catalog,
		String schemaPattern,
		String procedureNamePattern
	) throws SQLException
	{
		return(null);
	}

	/**
	 * Get a description of a catalog's stored procedure parameters
	 * and result columns.
	 * Currently not applicable and not implemented, returns null
	 *
	 * <p>Only descriptions matching the schema, procedure and parameter
	 * name criteria are returned. They are ordered by PROCEDURE_SCHEM
	 * and PROCEDURE_NAME. Within this, the return value, if any, is
	 * first. Next are the parameter descriptions in call order. The
	 * column descriptions follow in column number order.
	 *
	 * <p>Each row in the ResultSet is a parameter description or column
	 * description with the following fields:
	 * <ol>
	 * <li><b>PROCEDURE_CAT</b> String => procedure catalog (may be null)
	 * <li><b>PROCEDURE_SCHE</b>M String => procedure schema (may be null)
	 * <li><b>PROCEDURE_NAME</b> String => procedure name
	 * <li><b>COLUMN_NAME</b> String => column/parameter name
	 * <li><b>COLUMN_TYPE</b> Short => kind of column/parameter:
	 * <ul><li>procedureColumnUnknown - nobody knows
	 * <li>procedureColumnIn - IN parameter
	 * <li>procedureColumnInOut - INOUT parameter
	 * <li>procedureColumnOut - OUT parameter
	 * <li>procedureColumnReturn - procedure return value
	 * <li>procedureColumnResult - result column in ResultSet
	 * </ul>
	 * <li><b>DATA_TYPE</b> short => SQL type from java.sql.Types
	 * <li><b>TYPE_NAME</b> String => Data source specific type name
	 * <li><b>PRECISION</b> int => precision
	 * <li><b>LENGTH</b> int => length in bytes of data
	 * <li><b>SCALE</b> short => scale
	 * <li><b>RADIX</b> short => radix
	 * <li><b>NULLABLE</b> short => can it contain NULL?
	 * <ul><li>procedureNoNulls - does not allow NULL values
	 * <li>procedureNullable - allows NULL values
	 * <li>procedureNullableUnknown - nullability unknown
	 * <li><b>REMARKS</b> String => comment describing parameter/column
	 * </ol>
	 * @param catalog This is ignored in org.postgresql, advise this is set to null
	 * @param schemaPattern
	 * @param procedureNamePattern a procedure name pattern
	 * @param columnNamePattern a column name pattern, this is currently ignored because postgresql does not name procedure parameters.
	 * @return each row is a stored procedure parameter or column description
	 * @throws SQLException if a database-access error occurs
	 * @see #getSearchStringEscape
	 */
	public ResultSet getProcedureColumns(
		String catalog,
		String schemaPattern,
		String procedureNamePattern,
		String columnNamePattern
	) throws SQLException
	{
		return(null);
	}

	//== this is a helper method which does not belong to the interface

	/**
	 * returns the given string where all slashes and single quotes are
	 * escaped with a slash.
	 *
	 * @param in the string to escape
	 * @return the escaped string
	 */
	private static String escapeQuotes(String in) {
		return(in.replaceAll("\\\\", "\\\\\\\\").replaceAll("'", "\\\\'"));
	}

	//== end helper method

	/**
	 * Get a description of tables available in a catalog.
	 *
	 * <p>Only table descriptions matching the catalog, schema, table
	 * name and type criteria are returned. They are ordered by
	 * TABLE_TYPE, TABLE_SCHEM and TABLE_NAME.
	 *
	 * <p>Each table description has the following columns:
	 *
	 * <ol>
	 * <li><b>TABLE_CAT</b> String => table catalog (may be null)
	 * <li><b>TABLE_SCHEM</b> String => table schema (may be null)
	 * <li><b>TABLE_NAME</b> String => table name
	 * <li><b>TABLE_TYPE</b> String => table type. Typical types are "TABLE",
	 * "VIEW", "SYSTEM TABLE", "GLOBAL TEMPORARY", "LOCAL
	 * TEMPORARY", "ALIAS", "SYNONYM".
	 * <li><b>REMARKS</b> String => explanatory comment on the table
	 * </ol>
	 *
	 * <p>The valid values for the types parameter are:
	 * "TABLE", "INDEX", "SEQUENCE", "VIEW",
	 * "SYSTEM TABLE", "SYSTEM INDEX", "SYSTEM VIEW",
	 * "SYSTEM TOAST TABLE", "SYSTEM TOAST INDEX",
	 * "TEMPORARY TABLE", and "TEMPORARY VIEW"
	 *
	 * @param catalog a catalog name; this parameter is currently ignored
	 * @param schemaPattern a schema name pattern
	 * @param tableNamePattern a table name pattern. For all tables this should be "%"
	 * @param types a list of table types to include; null returns all types;
	 *              this parameter is currently ignored
	 * @return each row is a table description
	 * @throws SQLException if a database-access error occurs.
	 */
	public ResultSet getTables(
		String catalog,
		String schemaPattern,
		String tableNamePattern,
		String types[]
	) throws SQLException
	{
		String select;
		String orderby;

		select =
			"SELECT * FROM ( " +
			"SELECT null AS TABLE_CAT, schemas.name AS TABLE_SCHEM, tables.name AS TABLE_NAME, " +
				"'SYSTEM TABLE' AS TABLE_TYPE, '' AS REMARKS, null AS TYPE_CAT, null AS TYPE_SCHEM, " +
				"null AS TYPE_NAME, 'id' AS SELF_REFERENCING_COL_NAME, 'SYSTEM' AS REF_GENERATION " +
				"FROM tables, schemas WHERE tables.schema_id = schemas.id AND tables.type = 1 " +
			"UNION ALL " +
			"SELECT null AS TABLE_CAT, schemas.name AS TABLE_SCHEM, tables.name AS TABLE_NAME, " +
				"'TABLE' AS TABLE_TYPE, '' AS REMARKS, null AS TYPE_CAT, null AS TYPE_SCHEM, " +
				"null AS TYPE_NAME, 'id' AS SELF_REFERENCING_COL_NAME, 'SYSTEM' AS REF_GENERATION " +
				"FROM tables, schemas WHERE tables.schema_id = schemas.id AND tables.type = 0 " +
			"UNION ALL " +
			"SELECT null AS TABLE_CAT, schemas.name AS TABLE_SCHEM, tables.name AS TABLE_NAME, " +
				"'VIEW' AS TABLE_TYPE, tables.query AS REMARKS, null AS TYPE_CAT, null AS TYPE_SCHEM, " +
				"null AS TYPE_NAME, 'id' AS SELF_REFERENCING_COL_NAME, 'SYSTEM' AS REF_GENERATION " +
				"FROM tables, schemas WHERE tables.schema_id = schemas.id AND tables.type = 2 " +
			") AS tables WHERE 1 = 1 ";

		if (tableNamePattern != null) {
			select += "AND TABLE_NAME LIKE '" + escapeQuotes(tableNamePattern) + "' ";
		}
		if (schemaPattern != null) {
			select += "AND TABLE_SCHEM LIKE '" + escapeQuotes(schemaPattern) + "' ";
		}
		if (types != null) {
			select += "AND (";
			for (int i = 0; i < types.length; i++) {
				select += (i == 0 ? "" : " OR ") + "TABLE_TYPE LIKE '" + escapeQuotes(types[i]) + "'";
			}
			select += ") ";
		}

		orderby = "ORDER BY TABLE_TYPE, TABLE_SCHEM, TABLE_NAME ";

		return(getStmt().executeQuery(select + orderby));
	}

	/**
	 * Get the schema names available in this database.  The results
	 * are ordered by schema name.
	 *
	 * <P>The schema column is:
	 *	<OL>
	 *	<LI><B>TABLE_SCHEM</B> String => schema name
	 *	<LI><B>TABLE_CATALOG</B> String => catalog name (may be null)
	 *	</OL>
	 *
	 * @return ResultSet each row has a single String column that is a
	 *         schema name
	 * @throws SQLException if a database error occurs
	 */
	public ResultSet getSchemas() throws SQLException {
		String query =
			"SELECT name AS TABLE_SCHEM, null AS TABLE_CATALOG " +
				"FROM schemas " +
				"ORDER BY TABLE_SCHEM";

		return(getStmt().executeQuery(query));
	}

	/**
	 * Get the catalog names available in this database.  The results
	 * are ordered by catalog name.
	 *
	 * <P>The catalog column is:
	 *	<OL>
	 *	<LI><B>TABLE_CAT</B> String => catalog name
	 *	</OL>
	 *
	 * Note: this will return a resultset with 'default', since there are no catalogs
	 *       in Monet
	 *
	 * @return ResultSet each row has a single String column that is a
	 *         catalog name
	 * @throws SQLException if a database error occurs
	 */
	public ResultSet getCatalogs() throws SQLException {
		String query =
			"SELECT 'default' AS TABLE_CAT";
			// some applications need a database or catalog...

		return(getStmt().executeQuery(query));
	}

	/**
	 * Get the table types available in this database.	The results
	 * are ordered by table type.
	 *
	 * <P>The table type is:
	 *	<OL>
	 *	<LI><B>TABLE_TYPE</B> String => table type.  Typical types are "TABLE",
	 *			"VIEW", "SYSTEM TABLE", "GLOBAL TEMPORARY",
	 *			"LOCAL TEMPORARY", "ALIAS", "SYNONYM".
	 *	</OL>
	 *
	 * @return ResultSet each row has a single String column that is a
	 *         table type
	 * @throws SQLException if a database error occurs
	 */
	public ResultSet getTableTypes() throws SQLException {
		String[] columns, types;
		String[][] results;

		columns = new String[1];
		types = new String[1];
		results = new String[3][1];

		columns[0] = "table_type";	// Monet would return lowercase columns
		types[0] = "varchar";
		results[0][0] = "SYSTEM TABLE";
		results[1][0] = "TABLE";
		results[2][0] = "VIEW";

		try {
			return(new MonetVirtualResultSet(columns, types, results));
		} catch (IllegalArgumentException e) {
			throw new SQLException("Internal driver error: " + e.getMessage());
		}
	}

	/**
	 * Get a description of table columns available in a catalog.
	 *
	 * <P>Only column descriptions matching the catalog, schema, table
	 * and column name criteria are returned.  They are ordered by
	 * TABLE_SCHEM, TABLE_NAME and ORDINAL_POSITION.
	 *
	 * <P>Each column description has the following columns:
	 *	<OL>
	 *	<LI><B>TABLE_CAT</B> String => table catalog (may be null)
	 *	<LI><B>TABLE_SCHEM</B> String => table schema (may be null)
	 *	<LI><B>TABLE_NAME</B> String => table name
	 *	<LI><B>COLUMN_NAME</B> String => column name
	 *	<LI><B>DATA_TYPE</B> short => SQL type from java.sql.Types
	 *	<LI><B>TYPE_NAME</B> String => Data source dependent type name
	 *	<LI><B>COLUMN_SIZE</B> int => column size.	For char or date
	 *		types this is the maximum number of characters, for numeric or
	 *		decimal types this is precision.
	 *	<LI><B>BUFFER_LENGTH</B> is not used.
	 *	<LI><B>DECIMAL_DIGITS</B> int => the number of fractional digits
	 *	<LI><B>NUM_PREC_RADIX</B> int => Radix (typically either 10 or 2)
	 *	<LI><B>NULLABLE</B> int => is NULL allowed?
	 *		<UL>
	 *		<LI> columnNoNulls - might not allow NULL values
	 *		<LI> columnNullable - definitely allows NULL values
	 *		<LI> columnNullableUnknown - nullability unknown
	 *		</UL>
	 *	<LI><B>REMARKS</B> String => comment describing column (may be null)
	 *	<LI><B>COLUMN_DEF</B> String => default value (may be null)
	 *	<LI><B>SQL_DATA_TYPE</B> int => unused
	 *	<LI><B>SQL_DATETIME_SUB</B> int => unused
	 *	<LI><B>CHAR_OCTET_LENGTH</B> int => for char types the
	 *		 maximum number of bytes in the column
	 *	<LI><B>ORDINAL_POSITION</B> int => index of column in table
	 *		(starting at 1)
	 *	<LI><B>IS_NULLABLE</B> String => "NO" means column definitely
	 *		does not allow NULL values; "YES" means the column might
	 *		allow NULL values.	An empty string means nobody knows.
	 *	</OL>
	 *
	 * @param catalog a catalog name; "" retrieves those without a catalog;
	 *                currently ignored
	 * @param schemaPattern a schema name pattern; "" retrieves those
	 *                      without a schema
	 * @param tableNamePattern a table name pattern
	 * @param columnNamePattern a column name pattern
	 * @return ResultSet each row is a column description
	 * @see #getSearchStringEscape
	 * @throws SQLException if a database error occurs
	 */
	public ResultSet getColumns(
		String catalog,
		String schemaPattern,
		String tableNamePattern,
		String columnNamePattern
	) throws SQLException
	{
		String query =
			"SELECT null AS TABLE_CAT, schemas.name AS TABLE_SCHEM, " +
			"tables.name AS TABLE_NAME, columns.name AS COLUMN_NAME, " +
			"columns.type AS TYPE_NAME, columns.type_digits AS COLUMN_SIZE, " +
			"columns.type_scale AS DECIMAL_DIGITS, 0 AS BUFFER_LENGTH, " +
			"10 AS NUM_PREC_RADIX, \"null\" AS nullable, null AS REMARKS, " +
			"columns.default AS COLUMN_DEF, 0 AS SQL_DATA_TYPE, " +
			"0 AS SQL_DATETIME_SUB, 0 AS CHAR_OCTET_LENGTH, " +
			"columns.number + 1 AS ORDINAL_POSITION, null AS SCOPE_CATALOG, " +
			"null AS SCOPE_SCHEMA, null AS SCOPE_TABLE " +
				"FROM columns, tables, schemas " +
				"WHERE columns.table_id = tables.id " +
					"AND tables.schema_id = schemas.id ";

		// Unfortunately we cannot get everything out of the query, so we have
		// to add DATA_TYPE, NULLABLE, IS_NULLABLE and SOURCE_DATA_TYPE using
		// Java logic and a virtual result set.

		if (schemaPattern != null) {
			query += "AND schemas.name LIKE '" + escapeQuotes(schemaPattern) + "' ";
		}
		if (tableNamePattern != null) {
			query += "AND tables.name LIKE '" + escapeQuotes(tableNamePattern) + "' ";
		}
		if (columnNamePattern != null) {
			query += "AND columns.name LIKE '" + escapeQuotes(columnNamePattern) + "' ";
		}

		query += "ORDER BY TABLE_SCHEM, TABLE_NAME, ORDINAL_POSITION";

		String[] columns = {	// Monet would return lowercase columns
			"table_cat", "table_schem", "table_name", "column_name",
			"data_type", "type_name", "column_size", "buffer_length",
			"decimal_digits", "num_prec_radix", "nullable", "remarks",
			"column_def", "sql_data_type", "sql_datetime_sub",
			"char_octet_length", "ordinal_position", "is_nullable",
			"scope_catlog", "scope_schema", "scope_table", "source_data_type"
		};

		String[] types = {
			"varchar", "varchar", "varchar", "varchar", "mediumint",
			"varchar", "mediumint", "mediumint", "mediumint", "mediumint",
			"mediumint", "varchar", "varchar", "mediumint", "mediumint",
			"mediumint", "mediumint", "varchar", "varchar", "varchar",
			"varchar", "mediumint"
		};

		String[][] results;
		ArrayList tmpRes = new ArrayList();

		ResultSet rs = getStmt().executeQuery(query);
		while (rs.next()) {
			String[] result = new String[22];
			result[0]  = rs.getString("table_cat");
			result[1]  = rs.getString("table_schem");
			result[2]  = rs.getString("table_name");
			result[3]  = rs.getString("column_name");
			result[4]  = "" + ((MonetDriver)driver).getJavaType(rs.getString("type_name"));
			result[5]  = rs.getString("type_name");
			result[6]  = rs.getString("column_size");
			result[7]  = rs.getString("buffer_length");
			result[8]  = rs.getString("decimal_digits");
			result[9]  = rs.getString("num_prec_radix");
			result[10] = "" + (rs.getBoolean("nullable") ? ResultSetMetaData.columnNullable : ResultSetMetaData.columnNoNulls);
			result[11] = rs.getString("remarks");
			result[12] = rs.getString("column_def");
			result[13] = rs.getString("sql_data_type");
			result[14] = rs.getString("sql_datetime_sub");
			result[15] = rs.getString("char_octet_length");
			result[16] = rs.getString("ordinal_position");
			result[17] = rs.getBoolean("nullable") ? "YES" : "NO";
			result[18] = rs.getString("scope_catalog");
			result[19] = rs.getString("scope_schema");
			result[20] = rs.getString("scope_table");
			result[21] = "" + ((MonetDriver)driver).getJavaType("other");
			tmpRes.add(result);
		}
		rs.close();

		results = (String[][])tmpRes.toArray(new String[tmpRes.size()][]);

		try {
			return(new MonetVirtualResultSet(columns, types, results));
		} catch (IllegalArgumentException e) {
			throw new SQLException("Internal driver error: " + e.getMessage());
		}
	}

	/**
	 * Get a description of the access rights for a table's columns.
	 * Not implemented since,Monet knows no access rights
	 *
	 * <P>Only privileges matching the column name criteria are
	 * returned.  They are ordered by COLUMN_NAME and PRIVILEGE.
	 *
	 * <P>Each privilige description has the following columns:
	 *	<OL>
	 *	<LI><B>TABLE_CAT</B> String => table catalog (may be null)
	 *	<LI><B>TABLE_SCHEM</B> String => table schema (may be null)
	 *	<LI><B>TABLE_NAME</B> String => table name
	 *	<LI><B>COLUMN_NAME</B> String => column name
	 *	<LI><B>GRANTOR</B> => grantor of access (may be null)
	 *	<LI><B>GRANTEE</B> String => grantee of access
	 *	<LI><B>PRIVILEGE</B> String => name of access (SELECT,
	 *		INSERT, UPDATE, REFRENCES, ...)
	 *	<LI><B>IS_GRANTABLE</B> String => "YES" if grantee is permitted
	 *		to grant to others; "NO" if not; null if unknown
	 *	</OL>
	 *
	 * @param catalog a catalog name; "" retrieves those without a catalog
	 * @param schema a schema name; "" retrieves those without a schema
	 * @param table a table name
	 * @param columnNamePattern a column name pattern
	 * @return ResultSet each row is a column privilege description
	 * @see #getSearchStringEscape
	 * @throws SQLException if a database error occurs
	 */
	public ResultSet getColumnPrivileges(
		String catalog,
		String schema,
		String table,
		String columnNamePattern
	) throws SQLException
	{
		String[] columns = {
			"TABLE_CAT", "TABLE_SCHEM", "TABLE_NAME", "COLUMN_NAME",
			"GRANTOR", "GRANTEE", "PRIVILEGE", "IS_GRANTABLE"
		};

		String[] types = {
			"varchar", "varchar", "varchar", "varchar", "varchar",
			"varchar", "varchar", "varchar"
		};

		String[][] results = new String[0][columns.length];

		try {
			return(new MonetVirtualResultSet(columns, types, results));
		} catch (IllegalArgumentException e) {
			throw new SQLException("Internal driver error: " + e.getMessage());
		}
	}

	/**
	 * Get a description of the access rights for each table available
	 * in a catalog.
	 * Not implemented since Monet knows no access rights
	 *
	 * <P>Only privileges matching the schema and table name
	 * criteria are returned.  They are ordered by TABLE_SCHEM,
	 * TABLE_NAME, and PRIVILEGE.
	 *
	 * <P>Each privilige description has the following columns:
	 *	<OL>
	 *	<LI><B>TABLE_CAT</B> String => table catalog (may be null)
	 *	<LI><B>TABLE_SCHEM</B> String => table schema (may be null)
	 *	<LI><B>TABLE_NAME</B> String => table name
	 *	<LI><B>GRANTOR</B> => grantor of access (may be null)
	 *	<LI><B>GRANTEE</B> String => grantee of access
	 *	<LI><B>PRIVILEGE</B> String => name of access (SELECT,
	 *		INSERT, UPDATE, REFRENCES, ...)
	 *	<LI><B>IS_GRANTABLE</B> String => "YES" if grantee is permitted
	 *		to grant to others; "NO" if not; null if unknown
	 *	</OL>
	 *
	 * @param catalog a catalog name; "" retrieves those without a catalog
	 * @param schemaPattern a schema name pattern; "" retrieves those
	 *                      without a schema
	 * @param tableNamePattern a table name pattern
	 * @return ResultSet each row is a table privilege description
	 * @see #getSearchStringEscape
	 * @throws SQLException if a database error occurs
	 */
	public ResultSet getTablePrivileges(
		String catalog,
		String schemaPattern,
		String tableNamePattern
	) throws SQLException
	{
		String[] columns = {
			"TABLE_CAT", "TABLE_SCHEM", "TABLE_NAME", "COLUMN_NAME",
			"GRANTOR", "GRANTEE", "PRIVILEGE", "IS_GRANTABLE"
		};

		String[] types = {
			"varchar", "varchar", "varchar", "varchar", "varchar",
			"varchar", "varchar", "varchar"
		};

		String[][] results = new String[0][columns.length];

		try {
			return(new MonetVirtualResultSet(columns, types, results));
		} catch (IllegalArgumentException e) {
			throw new SQLException("Internal driver error: " + e.getMessage());
		}
	}

	/**
	 * Get a description of a table's optimal set of columns that
	 * uniquely identifies a row. They are ordered by SCOPE.
	 *
	 * <P>Each column description has the following columns:
	 *	<OL>
	 *	<LI><B>SCOPE</B> short => actual scope of result
	 *		<UL>
	 *		<LI> bestRowTemporary - very temporary, while using row
	 *		<LI> bestRowTransaction - valid for remainder of current transaction
	 *		<LI> bestRowSession - valid for remainder of current session
	 *		</UL>
	 *	<LI><B>COLUMN_NAME</B> String => column name
	 *	<LI><B>DATA_TYPE</B> short => SQL data type from java.sql.Types
	 *	<LI><B>TYPE_NAME</B> String => Data source dependent type name
	 *	<LI><B>COLUMN_SIZE</B> int => precision
	 *	<LI><B>BUFFER_LENGTH</B> int => not used
	 *	<LI><B>DECIMAL_DIGITS</B> short  => scale
	 *	<LI><B>PSEUDO_COLUMN</B> short => is this a pseudo column
	 *		like an Oracle ROWID
	 *		<UL>
	 *		<LI> bestRowUnknown - may or may not be pseudo column
	 *		<LI> bestRowNotPseudo - is NOT a pseudo column
	 *		<LI> bestRowPseudo - is a pseudo column
	 *		</UL>
	 *	</OL>
	 *
	 * @param catalog a catalog name; "" retrieves those without a catalog
	 * @param schema a schema name; "" retrieves those without a schema
	 * @param table a table name
	 * @param scope the scope of interest; use same values as SCOPE
	 * @param nullable include columns that are nullable?
	 * @return ResultSet each row is a column description
	 * @throws SQLException if a database error occurs
	 */
	public ResultSet getBestRowIdentifier(
		String catalog,
		String schema,
		String table,
		int scope,
		boolean nullable
	) throws SQLException
	{
		String query =
			"SELECT columns.name AS COLUMN_NAME, columns.type AS TYPE_NAME, " +
			"columns.type_digits AS COLUMN_SIZE, 0 AS BUFFER_LENGTH, " +
			"columns.type_scale AS DECIMAL_DIGITS, keys.type AS keytype " +
				"FROM keys, keycolumns, tables, columns, schemas " +
				"WHERE keys.id = keycolumns.id AND keys.table_id = tables.id " +
					"AND keys.table_id = columns.table_id " +
					"AND keycolumns.\"column\" = columns.name " +
					"AND tables.schema_id = schemas.id " +
					"AND keys.type IN (0, 1) ";

		// SCOPE, DATA_TYPE, PSEUDO_COLUMN have to be generated with Java logic

		if (schema != null) {
			query += "AND schemas.name LIKE '" + escapeQuotes(schema) + "' ";
		}
		if (table != null) {
			query += "AND tables.name LIKE '" + escapeQuotes(table) + "' ";
		}
		if (!nullable) {
			query += "AND columns.\"null\" = false ";
		}

		query += "ORDER BY keytype";

		String columns[] = {
			"SCOPE", "COLUMN_NAME", "DATA_TYPE", "TYPE_NAME", "COLUMN_SIZE",
			"BUFFER_LENGTH", "DECIMAL_DIGITS", "PSEUDO_COLUMN"
		};

		String types[] = {
			"mediumint", "varchar", "mediumint", "varchar", "mediumint",
			"mediumint", "mediumint", "mediumint"
		};

		String[][] results;
		ArrayList tmpRes = new ArrayList();

		ResultSet rs = getStmt().executeQuery(query);
		while (rs.next()) {
			String[] result = new String[8];
			result[0]  = "" + DatabaseMetaData.bestRowSession;
			result[1]  = rs.getString("column_name");
			result[2]  = "" + ((MonetDriver)driver).getJavaType(rs.getString("type_name"));
			result[3]  = rs.getString("type_name");
			result[4]  = rs.getString("column_size");
			result[5]  = rs.getString("buffer_length");
			result[6]  = rs.getString("decimal_digits");
			result[7]  = "" + DatabaseMetaData.bestRowNotPseudo;
			tmpRes.add(result);
		}
		rs.close();

		results = (String[][])tmpRes.toArray(new String[tmpRes.size()][]);

		try {
			return(new MonetVirtualResultSet(columns, types, results));
		} catch (IllegalArgumentException e) {
			throw new SQLException("Internal driver error: " + e.getMessage());
		}
	}

	/**
	 * Get a description of a table's columns that are automatically
	 * updated when any value in a row is updated.	They are
	 * unordered.
	 *
	 * <P>Each column description has the following columns:
	 *	<OL>
	 *	<LI><B>SCOPE</B> short => is not used
	 *	<LI><B>COLUMN_NAME</B> String => column name
	 *	<LI><B>DATA_TYPE</B> short => SQL data type from java.sql.Types
	 *	<LI><B>TYPE_NAME</B> String => Data source dependent type name
	 *	<LI><B>COLUMN_SIZE</B> int => precision
	 *	<LI><B>BUFFER_LENGTH</B> int => length of column value in bytes
	 *	<LI><B>DECIMAL_DIGITS</B> short  => scale
	 *	<LI><B>PSEUDO_COLUMN</B> short => is this a pseudo column
	 *		like an Oracle ROWID
	 *		<UL>
	 *		<LI> versionColumnUnknown - may or may not be pseudo column
	 *		<LI> versionColumnNotPseudo - is NOT a pseudo column
	 *		<LI> versionColumnPseudo - is a pseudo column
	 *		</UL>
	 *	</OL>
	 *
	 * @param catalog a catalog name; "" retrieves those without a catalog
	 * @param schema a schema name; "" retrieves those without a schema
	 * @param table a table name
	 * @return ResultSet each row is a column description
	 * @throws SQLException if a database error occurs
	 */
	public ResultSet getVersionColumns(
		String catalog,
		String schema,
		String table
	) throws SQLException
	{
		// I don't know of columns which update themselves, except maybe on the
		// system tables

		String columns[] = {
			"SCOPE", "COLUMN_NAME", "DATA_TYPE", "TYPE_NAME", "COLUMN_SIZE",
			"BUFFER_LENGTH", "DECIMAL_DIGITS", "PSEUDO_COLUMN"
		};

		String types[] = {
			"mediumint", "varchar", "mediumint", "varchar", "mediumint",
			"mediumint", "mediumint", "mediumint"
		};

		String[][] results = new String[0][columns.length];

		try {
			return(new MonetVirtualResultSet(columns, types, results));
		} catch (IllegalArgumentException e) {
			throw new SQLException("Internal driver error: " + e.getMessage());
		}
	}

	/**
	 * Get a description of a table's primary key columns.  They
	 * are ordered by COLUMN_NAME.
	 *
	 * <P>Each column description has the following columns:
	 *	<OL>
	 *	<LI><B>TABLE_CAT</B> String => table catalog (may be null)
	 *	<LI><B>TABLE_SCHEM</B> String => table schema (may be null)
	 *	<LI><B>TABLE_NAME</B> String => table name
	 *	<LI><B>COLUMN_NAME</B> String => column name
	 *	<LI><B>KEY_SEQ</B> short => sequence number within primary key
	 *	<LI><B>PK_NAME</B> String => primary key name (may be null)
	 *	</OL>
	 *
	 * @param catalog a catalog name; "" retrieves those without a catalog
	 * @param schema a schema name pattern; "" retrieves those
	 * without a schema
	 * @param table a table name
	 * @return ResultSet each row is a primary key column description
	 * @throws SQLException if a database error occurs
	 */
	public ResultSet getPrimaryKeys(
		String catalog,
		String schema,
		String table
	) throws SQLException
	{
		String query =
			"SELECT null AS TABLE_CAT, schemas.name AS TABLE_SCHEM, " +
			"tables.name AS TABLE_NAME, keycolumns.\"column\" AS COLUMN_NAME, " +
			"keys.type AS KEY_SEQ, keys.name AS PK_NAME " +
				"FROM keys, keycolumns, tables, schemas " +
				"WHERE keys.id = keycolumns.id AND keys.table_id = tables.id " +
					"AND tables.schema_id = schemas.id " +
					"AND keys.type = 0 ";

		if (schema != null) {
			query += "AND schemas.name LIKE '" + escapeQuotes(schema) + "' ";
		}
		if (table != null) {
			query += "AND tables.name LIKE '" + escapeQuotes(table) + "' ";
		}

		query += "ORDER BY COLUMN_NAME";

		return(getStmt().executeQuery(query));
	}

	/**
	 * Get a description of the primary key columns that are
	 * referenced by a table's foreign key columns (the primary keys
	 * imported by a table).  They are ordered by PKTABLE_CAT,
	 * PKTABLE_SCHEM, PKTABLE_NAME, and KEY_SEQ.
	 *
	 * <P>Each primary key column description has the following columns:
	 *	<OL>
	 *	<LI><B>PKTABLE_CAT</B> String => primary key table catalog
	 *		being imported (may be null)
	 *	<LI><B>PKTABLE_SCHEM</B> String => primary key table schema
	 *		being imported (may be null)
	 *	<LI><B>PKTABLE_NAME</B> String => primary key table name
	 *		being imported
	 *	<LI><B>PKCOLUMN_NAME</B> String => primary key column name
	 *		being imported
	 *	<LI><B>FKTABLE_CAT</B> String => foreign key table catalog (may be null)
	 *	<LI><B>FKTABLE_SCHEM</B> String => foreign key table schema (may be null)
	 *	<LI><B>FKTABLE_NAME</B> String => foreign key table name
	 *	<LI><B>FKCOLUMN_NAME</B> String => foreign key column name
	 *	<LI><B>KEY_SEQ</B> short => sequence number within foreign key
	 *	<LI><B>UPDATE_RULE</B> short => What happens to
	 *		 foreign key when primary is updated:
	 *		<UL>
	 *		<LI> importedKeyCascade - change imported key to agree
	 *				 with primary key update
	 *		<LI> importedKeyRestrict - do not allow update of primary
	 *				 key if it has been imported
	 *		<LI> importedKeySetNull - change imported key to NULL if
	 *				 its primary key has been updated
	 *		</UL>
	 *	<LI><B>DELETE_RULE</B> short => What happens to
	 *		the foreign key when primary is deleted.
	 *		<UL>
	 *		<LI> importedKeyCascade - delete rows that import a deleted key
	 *		<LI> importedKeyRestrict - do not allow delete of primary
	 *				 key if it has been imported
	 *		<LI> importedKeySetNull - change imported key to NULL if
	 *				 its primary key has been deleted
	 *		</UL>
	 *	<LI><B>FK_NAME</B> String => foreign key name (may be null)
	 *	<LI><B>PK_NAME</B> String => primary key name (may be null)
	 *	</OL>
	 *
	 * @param catalog a catalog name; "" retrieves those without a catalog
	 * @param schema a schema name pattern; "" retrieves those
	 * without a schema
	 * @param table a table name
	 * @return ResultSet each row is a primary key column description
	 * @see #getExportedKeys
	 * @throws SQLException if a database error occurs
	 */
	public ResultSet getImportedKeys(String catalog, String schema, String table)
		throws SQLException
	{
		String query =
			"SELECT null AS PKTABLE_CAT, pkschema.name AS PKTABLE_SCHEM, " +
			"pktable.name AS PKTABLE_NAME, pkkeycol.\"column\" AS PKCOLUMN_NAME, " +
			"null AS FKTABLE_CAT, fkschema.name AS FKTABLE_SCHEM, " +
			"fktable.name AS FKTABLE_NAME, fkkeycol.\"column\" AS FKCOLUMN_NAME, " +
			"fkkey.type AS KEY_SEQ, " + DatabaseMetaData.importedKeyNoAction + " AS UPDATE_RULE, " +
			"" + DatabaseMetaData.importedKeyNoAction + " AS DELETE_RULE, " +
			"fkkey.name AS FK_NAME, pkkey.name AS PK_NAME, " +
			"" + DatabaseMetaData.importedKeyNotDeferrable + " AS DEFERRABILITY " +
				"FROM keys AS fkkey, keys AS pkkey, keycolumns AS fkkeycol, " +
				"keycolumns AS pkkeycol, tables AS fktable, tables AS pktable, " +
				"schemas AS fkschema, schemas AS pkschema " +
				"WHERE fktable.id = fkkey.table_id AND pktable.id = pkkey.table_id AND " +
				"fkkey.id = fkkeycol.id AND pkkey.id = pkkeycol.id AND " +
				"fkschema.id = fktable.schema_id AND pkschema.id = pktable.schema_id AND " +
				"fkkey.rkey > -1 AND fkkey.rkey = pkkey.id ";

		if (schema != null) {
			query += "AND fkschema.name LIKE '" + escapeQuotes(schema) + "' ";
		}
		if (table != null) {
			query += "AND fktable.name LIKE '" + escapeQuotes(table) + "' ";
		}

		query += "ORDER BY PKTABLE_CAT, PKTABLE_SCHEM, PKTABLE_NAME, KEY_SEQ";

		return(getStmt().executeQuery(query));
	}

	/**
	 * Get a description of a foreign key columns that reference a
	 * table's primary key columns (the foreign keys exported by a
	 * table).	They are ordered by FKTABLE_CAT, FKTABLE_SCHEM,
	 * FKTABLE_NAME, and KEY_SEQ.
	 *
	 * <P>Each foreign key column description has the following columns:
	 *	<OL>
	 *	<LI><B>PKTABLE_CAT</B> String => primary key table catalog (may be null)
	 *	<LI><B>PKTABLE_SCHEM</B> String => primary key table schema (may be null)
	 *	<LI><B>PKTABLE_NAME</B> String => primary key table name
	 *	<LI><B>PKCOLUMN_NAME</B> String => primary key column name
	 *	<LI><B>FKTABLE_CAT</B> String => foreign key table catalog (may be null)
	 *		being exported (may be null)
	 *	<LI><B>FKTABLE_SCHEM</B> String => foreign key table schema (may be null)
	 *		being exported (may be null)
	 *	<LI><B>FKTABLE_NAME</B> String => foreign key table name
	 *		being exported
	 *	<LI><B>FKCOLUMN_NAME</B> String => foreign key column name
	 *		being exported
	 *	<LI><B>KEY_SEQ</B> short => sequence number within foreign key
	 *	<LI><B>UPDATE_RULE</B> short => What happens to
	 *		 foreign key when primary is updated:
	 *		<UL>
	 *		<LI> importedKeyCascade - change imported key to agree
	 *				 with primary key update
	 *		<LI> importedKeyRestrict - do not allow update of primary
	 *				 key if it has been imported
	 *		<LI> importedKeySetNull - change imported key to NULL if
	 *				 its primary key has been updated
	 *		</UL>
	 *	<LI><B>DELETE_RULE</B> short => What happens to
	 *		the foreign key when primary is deleted.
	 *		<UL>
	 *		<LI> importedKeyCascade - delete rows that import a deleted key
	 *		<LI> importedKeyRestrict - do not allow delete of primary
	 *				 key if it has been imported
	 *		<LI> importedKeySetNull - change imported key to NULL if
	 *				 its primary key has been deleted
	 *		</UL>
	 *	<LI><B>FK_NAME</B> String => foreign key identifier (may be null)
	 *	<LI><B>PK_NAME</B> String => primary key identifier (may be null)
	 *	</OL>
	 *
	 * @param catalog a catalog name; "" retrieves those without a catalog
	 * @param schema a schema name pattern; "" retrieves those
	 * without a schema
	 * @param table a table name
	 * @return ResultSet each row is a foreign key column description
	 * @see #getImportedKeys
	 * @throws SQLException if a database error occurs
	 */
	public ResultSet getExportedKeys(String catalog, String schema, String table)
		throws SQLException
	{
		String query =
			"SELECT null AS PKTABLE_CAT, pkschema.name AS PKTABLE_SCHEM, " +
			"pktable.name AS PKTABLE_NAME, pkkeycol.\"column\" AS PKCOLUMN_NAME, " +
			"null AS FKTABLE_CAT, fkschema.name AS FKTABLE_SCHEM, " +
			"fktable.name AS FKTABLE_NAME, fkkeycol.\"column\" AS FKCOLUMN_NAME, " +
			"fkkey.type AS KEY_SEQ, " + DatabaseMetaData.importedKeyNoAction + " AS UPDATE_RULE, " +
			"" + DatabaseMetaData.importedKeyNoAction + " AS DELETE_RULE, " +
			"fkkey.name AS FK_NAME, pkkey.name AS PK_NAME, " +
			"" + DatabaseMetaData.importedKeyNotDeferrable + " AS DEFERRABILITY " +
				"FROM keys AS fkkey, keys AS pkkey, keycolumns AS fkkeycol, " +
				"keycolumns AS pkkeycol, tables AS fktable, tables AS pktable, " +
				"schemas AS fkschema, schemas AS pkschema " +
				"WHERE fktable.id = fkkey.table_id AND pktable.id = pkkey.table_id AND " +
				"fkkey.id = fkkeycol.id AND pkkey.id = pkkeycol.id AND " +
				"fkschema.id = fktable.schema_id AND pkschema.id = pktable.schema_id AND " +
				"fkkey.rkey > -1 AND fkkey.rkey = pkkey.id ";

		if (schema != null) {
			query += "AND pkschema.name LIKE '" + escapeQuotes(schema) + "' ";
		}
		if (table != null) {
			query += "AND pktable.name LIKE '" + escapeQuotes(table) + "' ";
		}

		query += "ORDER BY FKTABLE_CAT, FKTABLE_SCHEM, FKTABLE_NAME, KEY_SEQ";

		return(getStmt().executeQuery(query));
	}

	/**
	 * Get a description of the foreign key columns in the foreign key
	 * table that reference the primary key columns of the primary key
	 * table (describe how one table imports another's key.) This
	 * should normally return a single foreign key/primary key pair
	 * (most tables only import a foreign key from a table once.)  They
	 * are ordered by FKTABLE_CAT, FKTABLE_SCHEM, FKTABLE_NAME, and
	 * KEY_SEQ.
	 *
	 * This method is currently unimplemented.
	 *
	 * <P>Each foreign key column description has the following columns:
	 *	<OL>
	 *	<LI><B>PKTABLE_CAT</B> String => primary key table catalog (may be null)
	 *	<LI><B>PKTABLE_SCHEM</B> String => primary key table schema (may be null)
	 *	<LI><B>PKTABLE_NAME</B> String => primary key table name
	 *	<LI><B>PKCOLUMN_NAME</B> String => primary key column name
	 *	<LI><B>FKTABLE_CAT</B> String => foreign key table catalog (may be null)
	 *		being exported (may be null)
	 *	<LI><B>FKTABLE_SCHEM</B> String => foreign key table schema (may be null)
	 *		being exported (may be null)
	 *	<LI><B>FKTABLE_NAME</B> String => foreign key table name
	 *		being exported
	 *	<LI><B>FKCOLUMN_NAME</B> String => foreign key column name
	 *		being exported
	 *	<LI><B>KEY_SEQ</B> short => sequence number within foreign key
	 *	<LI><B>UPDATE_RULE</B> short => What happens to
	 *		 foreign key when primary is updated:
	 *		<UL>
	 *		<LI> importedKeyCascade - change imported key to agree
	 *				 with primary key update
	 *		<LI> importedKeyRestrict - do not allow update of primary
	 *				 key if it has been imported
	 *		<LI> importedKeySetNull - change imported key to NULL if
	 *				 its primary key has been updated
	 *		</UL>
	 *	<LI><B>DELETE_RULE</B> short => What happens to
	 *		the foreign key when primary is deleted.
	 *		<UL>
	 *		<LI> importedKeyCascade - delete rows that import a deleted key
	 *		<LI> importedKeyRestrict - do not allow delete of primary
	 *				 key if it has been imported
	 *		<LI> importedKeySetNull - change imported key to NULL if
	 *				 its primary key has been deleted
	 *		</UL>
	 *	<LI><B>FK_NAME</B> String => foreign key identifier (may be null)
	 *	<LI><B>PK_NAME</B> String => primary key identifier (may be null)
	 *	</OL>
	 *
	 * @param catalog a catalog name; "" retrieves those without a catalog
	 * @param schema a schema name pattern; "" retrieves those
	 * without a schema
	 * @param table a table name
	 * @return ResultSet each row is a foreign key column description
	 * @see #getImportedKeys
	 * @throws SQLException if a database error occurs
	 */
	public ResultSet getCrossReference(
		String pcatalog,
		String pschema,
		String ptable,
		String fcatalog,
		String fschema,
		String ftable
	) throws SQLException
	{
		String query =
			"SELECT null AS PKTABLE_CAT, pkschema.name AS PKTABLE_SCHEM, " +
			"pktable.name AS PKTABLE_NAME, pkkeycol.\"column\" AS PKCOLUMN_NAME, " +
			"null AS FKTABLE_CAT, fkschema.name AS FKTABLE_SCHEM, " +
			"fktable.name AS FKTABLE_NAME, fkkeycol.\"column\" AS FKCOLUMN_NAME, " +
			"fkkey.type AS KEY_SEQ, " + DatabaseMetaData.importedKeyNoAction + " AS UPDATE_RULE, " +
			"" + DatabaseMetaData.importedKeyNoAction + " AS DELETE_RULE, " +
			"fkkey.name AS FK_NAME, pkkey.name AS PK_NAME, " +
			"" + DatabaseMetaData.importedKeyNotDeferrable + " AS DEFERRABILITY " +
				"FROM keys AS fkkey, keys AS pkkey, keycolumns AS fkkeycol, " +
				"keycolumns AS pkkeycol, tables AS fktable, tables AS pktable, " +
				"schemas AS fkschema, schemas AS pkschema " +
				"WHERE fktable.id = fkkey.table_id AND pktable.id = pkkey.table_id AND " +
				"fkkey.id = fkkeycol.id AND pkkey.id = pkkeycol.id AND " +
				"fkschema.id = fktable.schema_id AND pkschema.id = pktable.schema_id AND " +
				"fkkey.rkey > -1 AND fkkey.rkey = pkkey.id ";

		if (pschema != null) {
			query += "AND pkschema.name LIKE '" + escapeQuotes(pschema) + "' ";
		}
		if (ptable != null) {
			query += "AND pktable.name LIKE '" + escapeQuotes(ptable) + "' ";
		}
		if (fschema != null) {
			query += "AND fkschema.name LIKE '" + escapeQuotes(fschema) + "' ";
		}
		if (ftable != null) {
			query += "AND fktable.name LIKE '" + escapeQuotes(ftable) + "' ";
		}

		query += "ORDER BY FKTABLE_CAT, FKTABLE_SCHEM, FKTABLE_NAME, KEY_SEQ";

		return(getStmt().executeQuery(query));
	}

	/**
	 * Get a description of all the standard SQL types supported by
	 * this database. They are ordered by DATA_TYPE and then by how
	 * closely the data type maps to the corresponding JDBC SQL type.
	 *
	 * <P>Each type description has the following columns:
	 *	<OL>
	 *	<LI><B>TYPE_NAME</B> String => Type name
	 *	<LI><B>DATA_TYPE</B> short => SQL data type from java.sql.Types
	 *	<LI><B>PRECISION</B> int => maximum precision
	 *	<LI><B>LITERAL_PREFIX</B> String => prefix used to quote a literal
	 *		(may be null)
	 *	<LI><B>LITERAL_SUFFIX</B> String => suffix used to quote a literal
	 *  (may be null)
	 *	<LI><B>CREATE_PARAMS</B> String => parameters used in creating
	 *		the type (may be null)
	 *	<LI><B>NULLABLE</B> short => can you use NULL for this type?
	 *		<UL>
	 *		<LI> typeNoNulls - does not allow NULL values
	 *		<LI> typeNullable - allows NULL values
	 *		<LI> typeNullableUnknown - nullability unknown
	 *		</UL>
	 *	<LI><B>CASE_SENSITIVE</B> boolean=> is it case sensitive?
	 *	<LI><B>SEARCHABLE</B> short => can you use "WHERE" based on this type:
	 *		<UL>
	 *		<LI> typePredNone - No support
	 *		<LI> typePredChar - Only supported with WHERE .. LIKE
	 *		<LI> typePredBasic - Supported except for WHERE .. LIKE
	 *		<LI> typeSearchable - Supported for all WHERE ..
	 *		</UL>
	 *	<LI><B>UNSIGNED_ATTRIBUTE</B> boolean => is it unsigned?
	 *	<LI><B>FIXED_PREC_SCALE</B> boolean => can it be a money value?
	 *	<LI><B>AUTO_INCREMENT</B> boolean => can it be used for an
	 *		auto-increment value?
	 *	<LI><B>LOCAL_TYPE_NAME</B> String => localized version of type name
	 *		(may be null)
	 *	<LI><B>MINIMUM_SCALE</B> short => minimum scale supported
	 *	<LI><B>MAXIMUM_SCALE</B> short => maximum scale supported
	 *	<LI><B>SQL_DATA_TYPE</B> int => unused
	 *	<LI><B>SQL_DATETIME_SUB</B> int => unused
	 *	<LI><B>NUM_PREC_RADIX</B> int => usually 2 or 10
	 *	</OL>
	 *
	 * @return ResultSet each row is a SQL type description
	 * @throws Exception if the developer made a Boo-Boo
	 */
	public ResultSet getTypeInfo() throws SQLException {
		String columns[] = {
			"TYPE_NAME", "DATA_TYPE", "PRECISION", "LITERAL_PREFIX",
			"LITERAL_SUFFIX", "CREATE_PARAMS", "NULLABLE", "CASE_SENSITIVE",
			"SEARCHABLE", "UNSIGNED_ATTRIBUTE", "FIXED_PREC_SCALE", "AUTO_INCREMENT",
			"LOCAL_TYPE_NAME", "MINIMUM_SCALE", "MAXIMUM_SCALE", "SQL_DATA_TYPE",
			"SQL_DATETIME_SUB", "NUM_PREC_RADIX"
		};

		String types[] = {
			"varchar", "mediumint", "mediumint", "varchar", "varchar",
			"varchar", "tinyint", "boolean", "tinyint", "boolean",
			"boolean", "boolean", "varchar", "tinyint", "tinyint",
			"mediumint", "mediumint", "mediumint"
		};

		String results[][] = {
		//	type_name     data_type         prec.   pre  suff  crte  nullable                            casesns  searchable                            unsign fixprec  autoinc   name    min  max  sql  sql radix
			{"table",     "" + Types.ARRAY,   "0", null, null, null, "" + DatabaseMetaData.typeNullable, "false", "" + DatabaseMetaData.typePredNone,   "true", "false", "false", "bat",  "0", "0", "0", "0",  "0"},
			{"boolean",   "" + Types.BOOLEAN, "1", null, null, null, "" + DatabaseMetaData.typeNullable, "false", "" + DatabaseMetaData.typePredBasic,  "true", "false", "false", "bit",  "0", "0", "0", "0",  "2"},
			{"bool",      "" + Types.BOOLEAN, "1", null, null, null, "" + DatabaseMetaData.typeNullable, "false", "" + DatabaseMetaData.typePredBasic,  "true", "false", "false", "bit",  "0", "0", "0", "0",  "2"},
			{"blob",      "" + Types.BLOB,    "0", null, null, null, "" + DatabaseMetaData.typeNullable, "false", "" + DatabaseMetaData.typePredNone,   "true", "false", "false", "blob", "0", "0", "0", "0",  "0"},
			{"char",      "" + Types.CHAR,    "0", null, null, null, "" + DatabaseMetaData.typeNullable, "false", "" + DatabaseMetaData.typePredChar,   "true", "false", "false", "str",  "0", "0", "0", "0",  "0"},
			{"character", "" + Types.CHAR,    "0", null, null, null, "" + DatabaseMetaData.typeNullable, "false", "" + DatabaseMetaData.typePredChar,   "true", "false", "false", "str",  "0", "0", "0", "0",  "0"},
			{"ubyte",     "" + Types.CHAR,    "2", null, null, null, "" + DatabaseMetaData.typeNullable, "false", "" + DatabaseMetaData.typePredNone,   "true", "false", "false", "uchr", "0", "0", "0", "0",  "2"},
			{"date",      "" + Types.DATE,    "0", null, null, null, "" + DatabaseMetaData.typeNullable, "false", "" + DatabaseMetaData.typePredBasic,  "true", "false", "false", "date", "0", "0", "0", "0",  "0"},
			{"decimal",   "" + Types.DECIMAL, "4", null, null, null, "" + DatabaseMetaData.typeNullable, "false", "" + DatabaseMetaData.typePredBasic, "false",  "true", "false", "sht",  "1", "0", "0", "0", "10"},
			{"decimal",   "" + Types.DECIMAL, "9", null, null, null, "" + DatabaseMetaData.typeNullable, "false", "" + DatabaseMetaData.typePredBasic, "false",  "true", "false", "int",  "1", "0", "0", "0", "10"},
			{"decimal",   "" + Types.DECIMAL,"19", null, null, null, "" + DatabaseMetaData.typeNullable, "false", "" + DatabaseMetaData.typePredBasic, "false",  "true", "false", "lng",  "1", "0", "0", "0", "10"},
			{"double",    "" + Types.DOUBLE, "51", null, null, null, "" + DatabaseMetaData.typeNullable, "false", "" + DatabaseMetaData.typePredBasic, "false", "false", "false", "dbl",  "2", "0", "0", "0",  "2"},
			{"float",     "" + Types.FLOAT,  "23", null, null, null, "" + DatabaseMetaData.typeNullable, "false", "" + DatabaseMetaData.typePredBasic, "false", "false", "false", "flt",  "2", "0", "0", "0",  "2"},
			{"float",     "" + Types.FLOAT,  "51", null, null, null, "" + DatabaseMetaData.typeNullable, "false", "" + DatabaseMetaData.typePredBasic, "false", "false", "false", "dbl",  "2", "0", "0", "0",  "2"},
			{"integer",   "" + Types.INTEGER, "9", null, null, null, "" + DatabaseMetaData.typeNullable, "false", "" + DatabaseMetaData.typePredBasic, "false", "false", "false", "int",  "0", "0", "0", "0",  "2"},
			{"number",    "" + Types.INTEGER, "9", null, null, null, "" + DatabaseMetaData.typeNullable, "false", "" + DatabaseMetaData.typePredBasic, "false", "false", "false", "int",  "0", "0", "0", "0",  "2"},
			{"mediumint", "" + Types.INTEGER, "9", null, null, null, "" + DatabaseMetaData.typeNullable, "false", "" + DatabaseMetaData.typePredBasic, "false", "false", "false", "int",  "0", "0", "0", "0",  "2"},
			{"int",       "" + Types.INTEGER, "9", null, null, null, "" + DatabaseMetaData.typeNullable, "false", "" + DatabaseMetaData.typePredBasic, "false", "false", "false", "int",  "0", "0", "0", "0",  "2"},
			{"tinyint",   "" + Types.INTEGER, "2", null, null, null, "" + DatabaseMetaData.typeNullable, "false", "" + DatabaseMetaData.typePredBasic, "false", "false", "false", "sht",  "0", "0", "0", "0",  "2"},
			{"smallint",  "" + Types.INTEGER, "4", null, null, null, "" + DatabaseMetaData.typeNullable, "false", "" + DatabaseMetaData.typePredBasic, "false", "false", "false", "sht",  "0", "0", "0", "0",  "2"},
			{"int",       "" + Types.INTEGER, "4", null, null, null, "" + DatabaseMetaData.typeNullable, "false", "" + DatabaseMetaData.typePredBasic, "false", "false", "false", "sht",  "0", "0", "0", "0",  "2"},
			{"int",       "" + Types.INTEGER,"19", null, null, null, "" + DatabaseMetaData.typeNullable, "false", "" + DatabaseMetaData.typePredBasic, "false", "false", "false", "lng",  "0", "0", "0", "0",  "2"},
			{"bigint",    "" + Types.INTEGER,"19", null, null, null, "" + DatabaseMetaData.typeNullable, "false", "" + DatabaseMetaData.typePredBasic, "false", "false", "false", "lng",  "0", "0", "0", "0",  "2"},
			{"text",      "" + Types.LONGVARCHAR,"0",null,null,null, "" + DatabaseMetaData.typeNullable, "false", "" + DatabaseMetaData.typePredChar,   "true", "false", "false", "str",  "0", "0", "0", "0",  "0"},
			{"tinytext",  "" + Types.LONGVARCHAR,"0",null,null,null, "" + DatabaseMetaData.typeNullable, "false", "" + DatabaseMetaData.typePredChar,   "true", "false", "false", "str",  "0", "0", "0", "0",  "0"},
			{"string",    "" + Types.LONGVARCHAR,"0",null,null,null, "" + DatabaseMetaData.typeNullable, "false", "" + DatabaseMetaData.typePredChar,   "true", "false", "false", "str",  "0", "0", "0", "0",  "0"},
			{"numeric",   "" + Types.NUMERIC, "4", null, null, null, "" + DatabaseMetaData.typeNullable, "false", "" + DatabaseMetaData.typePredBasic, "false", "false", "false", "sht",  "1", "0", "0", "0", "10"},
			{"numeric",   "" + Types.NUMERIC, "9", null, null, null, "" + DatabaseMetaData.typeNullable, "false", "" + DatabaseMetaData.typePredBasic, "false", "false", "false", "int",  "1", "0", "0", "0", "10"},
			{"numeric",   "" + Types.NUMERIC,"19", null, null, null, "" + DatabaseMetaData.typeNullable, "false", "" + DatabaseMetaData.typePredBasic, "false", "false", "false", "lng",  "1", "0", "0", "0", "10"},
			{"oid",       "" + Types.OTHER,   "9", null, null, null, "" + DatabaseMetaData.typeNullable, "false", "" + DatabaseMetaData.typePredNone,   "true", "false",  "true", "lng",  "0", "0", "0", "0",  "2"},
			{"month_interval",""+Types.OTHER, "0", null, null, null, "" + DatabaseMetaData.typeNullable, "false", "" + DatabaseMetaData.typePredNone,  "false", "false", "false", "int",  "0", "0", "0", "0", "10"},
			{"sec_interval",""+Types.OTHER,   "0", null, null, null, "" + DatabaseMetaData.typeNullable, "false", "" + DatabaseMetaData.typePredNone,  "false", "false", "false", "lng",  "0", "0", "0", "0", "10"},
			{"real",      "" + Types.REAL,   "51", null, null, null, "" + DatabaseMetaData.typeNullable, "false", "" + DatabaseMetaData.typePredBasic, "false", "false", "false", "dbl",  "2", "0", "0", "0",  "2"},
			{"time",      "" + Types.TIME,    "0", null, null, null, "" + DatabaseMetaData.typeNullable, "false", "" + DatabaseMetaData.typePredBasic,  "true", "false", "false", "daytime","0","0","0", "0",  "0"},
			{"datetime",  "" + Types.TIMESTAMP,"0",null, null, null, "" + DatabaseMetaData.typeNullable, "false", "" + DatabaseMetaData.typePredBasic,  "true", "false", "false", "timestamp","0","0","0","0", "0"},
			{"timestamp", "" + Types.TIMESTAMP,"0",null, null, null, "" + DatabaseMetaData.typeNullable, "false", "" + DatabaseMetaData.typePredBasic,  "true", "false", "false", "timestamp","0","0","0","0", "0"},
			{"varchar",   "" + Types.VARCHAR, "0", null, null, null, "" + DatabaseMetaData.typeNullable, "false", "" + DatabaseMetaData.typePredChar,   "true", "false", "false", "str",  "0", "0", "0", "0",  "0"}
		};

		try {
			return(new MonetVirtualResultSet(columns, types, results));
		} catch (IllegalArgumentException e) {
			throw new SQLException("Internal driver error: " + e.getMessage());
		}
	}

	/**
	 * Get a description of a table's indices and statistics. They are
	 * ordered by NON_UNIQUE, TYPE, INDEX_NAME, and ORDINAL_POSITION.
	 *
	 * <P>Each index column description has the following columns:
	 *	<OL>
	 *	<LI><B>TABLE_CAT</B> String => table catalog (may be null)
	 *	<LI><B>TABLE_SCHEM</B> String => table schema (may be null)
	 *	<LI><B>TABLE_NAME</B> String => table name
	 *	<LI><B>NON_UNIQUE</B> boolean => Can index values be non-unique?
	 *		false when TYPE is tableIndexStatistic
	 *	<LI><B>INDEX_QUALIFIER</B> String => index catalog (may be null);
	 *		null when TYPE is tableIndexStatistic
	 *	<LI><B>INDEX_NAME</B> String => index name; null when TYPE is
	 *		tableIndexStatistic
	 *	<LI><B>TYPE</B> short => index type:
	 *		<UL>
	 *		<LI> tableIndexStatistic - this identifies table statistics that are
	 *			 returned in conjuction with a table's index descriptions
	 *		<LI> tableIndexClustered - this is a clustered index
	 *		<LI> tableIndexHashed - this is a hashed index
	 *		<LI> tableIndexOther - this is some other style of index
	 *		</UL>
	 *	<LI><B>ORDINAL_POSITION</B> short => column sequence number
	 *		within index; zero when TYPE is tableIndexStatistic
	 *	<LI><B>COLUMN_NAME</B> String => column name; null when TYPE is
	 *		tableIndexStatistic
	 *	<LI><B>ASC_OR_DESC</B> String => column sort sequence, "A" => ascending
	 *		"D" => descending, may be null if sort sequence is not supported;
	 *		null when TYPE is tableIndexStatistic
	 *	<LI><B>CARDINALITY</B> int => When TYPE is tableIndexStatisic then
	 *		this is the number of rows in the table; otherwise it is the
	 *		number of unique values in the index.
	 *	<LI><B>PAGES</B> int => When TYPE is  tableIndexStatisic then
	 *		this is the number of pages used for the table, otherwise it
	 *		is the number of pages used for the current index.
	 *	<LI><B>FILTER_CONDITION</B> String => Filter condition, if any.
	 *		(may be null)
	 *	</OL>
	 *
	 * @param catalog a catalog name; "" retrieves those without a catalog
	 * @param schema a schema name pattern; "" retrieves those without a schema
	 * @param table a table name
	 * @param unique when true, return only indices for unique values;
	 *	   when false, return indices regardless of whether unique or not
	 * @param approximate when true, result is allowed to reflect approximate
	 *	   or out of data values; when false, results are requested to be
	 *	   accurate
	 * @return ResultSet each row is an index column description
	 * @throws SQLException if a database occurs
	 */
	public ResultSet getIndexInfo(
		String catalog,
		String schema,
		String table,
		boolean unique,
		boolean approximate
	) throws SQLException
	{
		String query =
			"SELECT null AS TABLE_CAT, schemas.name AS TABLE_SCHEM, " +
			"tables.name AS TABLE_NAME, idxs.type as nonunique, " +
			"null AS INDEX_QUALIFIER, idxs.name AS INDEX_NAME, " +
			DatabaseMetaData. tableIndexOther + " AS TYPE, " +
			"columns.number AS ORDINAL_POSITION, columns.name AS COLUMN_NAME, " +
			"null AS ASC_OR_DESC, 0 AS PAGES, " +
			"null AS FILTER_CONDITION " +
				"FROM idxs, columns, keycolumns, tables, schemas " +
				"WHERE idxs.table_id = tables.id " +
					"AND tables.schema_id = schemas.id " +
					"AND keycolumns.id = idxs.id " +
					"AND columns.name = keycolumns.\"column\" " +
					"AND columns.table_id = tables.id " +
					"AND idxs.name NOT IN (SELECT name FROM keys WHERE type <> 1) ";

		if (schema != null) {
			query += "AND schemas.name LIKE '" + escapeQuotes(schema) + "' ";
		}
		if (table != null) {
			query += "AND tables.name LIKE '" + escapeQuotes(table) + "' ";
		}
		if (unique) {
			query += "AND idxs.type = 0 ";
		}

		query += "ORDER BY nonunique, TYPE, INDEX_NAME, ORDINAL_POSITION";

		String columns[] = {
			"TABLE_CAT", "TABLE_SCHEM", "TABLE_NAME", "NON_UNIQUE",
			"INDEX_QUALIFIER", "INDEX_NAME", "TYPE", "ORDINAL_POSITION",
			"COLUMN_NAME", "ASC_OR_DESC", "CARDINALITY", "PAGES",
			"FILTER_CONDITION"
		};

		String types[] = {
			"varchar", "varchar", "varchar", "boolean",
			"varchar", "varchar", "mediumint", "mediumint",
			"varchar", "varchar", "mediumint", "mediumint", "varchar"
		};

		String[][] results;
		ArrayList tmpRes = new ArrayList();

		Statement sub = null;
		if (!approximate) sub = con.createStatement();

		ResultSet rs = getStmt().executeQuery(query);
		while (rs.next()) {
			String[] result = new String[13];
			result[0]  = null;
			result[1]  = rs.getString("table_schem");
			result[2]  = rs.getString("table_name");
			result[3]  = (rs.getInt("nonunique") == 0 ? "false" : "true");
			result[4]  = rs.getString("index_qualifier");
			result[5]  = rs.getString("index_name");
			result[6]  = rs.getString("type");
			result[7]  = rs.getString("ordinal_position");
			result[8]  = rs.getString("column_name");
			result[9]  = rs.getString("asc_or_desc");
			if (approximate) {
				result[10] = "0";
			} else {
				ResultSet count = sub.executeQuery("SELECT COUNT(*) AS CARDINALITY FROM " + rs.getString("table_schem") + "." + rs.getString("table_name"));
				if (count.next()) {
					result[10] = count.getString("cardinality");
				} else {
					result[10] = "0";
				}
			}
			result[11] = rs.getString("pages");
			result[12] = rs.getString("filter_condition");
			tmpRes.add(result);
		}

		if (!approximate) sub.close();
		rs.close();

		results = (String[][])tmpRes.toArray(new String[tmpRes.size()][]);

		try {
			return(new MonetVirtualResultSet(columns, types, results));
		} catch (IllegalArgumentException e) {
			throw new SQLException("Internal driver error: " + e.getMessage());
		}
	}

	// ** JDBC 2 Extensions **

	/**
	 * Does the database support the given result set type?
	 *
	 * @param type - defined in java.sql.ResultSet
	 * @return true if so; false otherwise
	 * @throws SQLException - if a database access error occurs
	 */
	public boolean supportsResultSetType(int type) throws SQLException {
		// The only type we don't support
		return(type != ResultSet.TYPE_SCROLL_SENSITIVE);
	}


	/**
	 * Does the database support the concurrency type in combination
	 * with the given result set type?
	 *
	 * @param type - defined in java.sql.ResultSet
	 * @param concurrency - type defined in java.sql.ResultSet
	 * @return true if so; false otherwise
	 * @throws SQLException - if a database access error occurs
	*/
	public boolean supportsResultSetConcurrency(int type, int concurrency)
		throws SQLException
	{
		// These combinations are not supported!
		if (type == ResultSet.TYPE_SCROLL_SENSITIVE)
			return(false);

		// We do only support Read Only ResultSets
		if (concurrency != ResultSet.CONCUR_READ_ONLY)
			return(false);

		// Everything else we do (well, what's left of it :) )
		return(true);
	}


	/* lots of unsupported stuff... (no updatable ResultSet!) */
	public boolean ownUpdatesAreVisible(int type) {
		return(false);
	}

	public boolean ownDeletesAreVisible(int type) {
		return(false);
	}

	public boolean ownInsertsAreVisible(int type) {
		return(false);
	}

	public boolean othersUpdatesAreVisible(int type) {
		return(false);
	}

	public boolean othersDeletesAreVisible(int i) {
		return(false);
	}

	public boolean othersInsertsAreVisible(int type) {
		return(false);
	}

	public boolean updatesAreDetected(int type) {
		return(false);
	}

	public boolean deletesAreDetected(int i) {
		return(false);
	}

	public boolean insertsAreDetected(int type) {
		return(false);
	}

	/**
	 * Indicates whether the driver supports batch updates.
	 * Not yet, but I want to do them in the near future
	 */
	public boolean supportsBatchUpdates() {
		return(false);
	}

	/**
	 * Return user defined types in a schema
	 * Probably not possible within Monet
	 *
	 * @throws SQLException if I made a Boo-Boo
	 */
	public ResultSet getUDTs(
		String catalog,
		String schemaPattern,
		String typeNamePattern,
		int[] types
	) throws SQLException
	{
		String query =
			"SELECT null AS TYPE_CAT, '' AS TYPE_SCHEM, '' AS TYPE_NAME, " +
			"'java.lang.Object' AS CLASS_NAME, 0 AS DATA_TYPE, " +
			"'' AS REMARKS, 0 AS BASE_TYPE WHERE 1 = 0";

		return(getStmt().executeQuery(query));
	}


	/**
	 * Retrieves the connection that produced this metadata object.
	 *
	 * @return the connection that produced this metadata object
	 */
	public Connection getConnection() {
		return(con);
	}

	/* I don't find these in the spec!?! */
	public boolean rowChangesAreDetected(int type) {
		return(false);
	}

	public boolean rowChangesAreVisible(int type) {
		return(false);
	}

	// ** JDBC 3 extensions **

	/**
	 * Retrieves whether this database supports savepoints.
	 *
	 * @return <code>true</code> if savepoints are supported;
	 *		   <code>false</code> otherwise
	 */
	public boolean supportsSavepoints() {
		return(true);
	}

	/**
	 * Retrieves whether this database supports named parameters to callable
	 * statements.
	 *
	 * @return <code>true</code> if named parameters are supported;
	 *		   <code>false</code> otherwise
	 */
	public boolean supportsNamedParameters() {
		return(false);
	}

	/**
	 * Retrieves whether it is possible to have multiple <code>ResultSet</code> objects
	 * returned from a <code>CallableStatement</code> object
	 * simultaneously.
	 *
	 * @return <code>true</code> if a <code>CallableStatement</code> object
	 *		   can return multiple <code>ResultSet</code> objects
	 *		   simultaneously; <code>false</code> otherwise
	 */
	public boolean supportsMultipleOpenResults() {
		return(false);
	}

	/**
	 * Retrieves whether auto-generated keys can be retrieved after
	 * a statement has been executed.
	 *
	 * @return <code>true</code> if auto-generated keys can be retrieved
	 *		   after a statement has executed; <code>false</code> otherwise
	 */
	public boolean supportsGetGeneratedKeys() {
		return(false);
	}

	/**
	 * Retrieves a description of the user-defined type (UDT) hierarchies defined in a
	 * particular schema in this database. Only the immediate super type/
	 * sub type relationship is modeled.
	 * <P>
	 * Only supertype information for UDTs matching the catalog,
	 * schema, and type name is returned. The type name parameter
	 * may be a fully-qualified name. When the UDT name supplied is a
	 * fully-qualified name, the catalog and schemaPattern parameters are
	 * ignored.
	 * <P>
	 * If a UDT does not have a direct super type, it is not listed here.
	 * A row of the <code>ResultSet</code> object returned by this method
	 * describes the designated UDT and a direct supertype. A row has the following
	 * columns:
	 *	<OL>
	 *	<LI><B>TYPE_CAT</B> String => the UDT's catalog (may be <code>null</code>)
	 *	<LI><B>TYPE_SCHEM</B> String => UDT's schema (may be <code>null</code>)
	 *	<LI><B>TYPE_NAME</B> String => type name of the UDT
	 *	<LI><B>SUPERTYPE_CAT</B> String => the direct super type's catalog
	 *							 (may be <code>null</code>)
	 *	<LI><B>SUPERTYPE_SCHEM</B> String => the direct super type's schema
	 *							   (may be <code>null</code>)
	 *	<LI><B>SUPERTYPE_NAME</B> String => the direct super type's name
	 *	</OL>
	 *
	 * <P><B>Note:</B> If the driver does not support type hierarchies, an
	 * empty result set is returned.
	 *
	 * @param catalog a catalog name; "" retrieves those without a catalog;
	 *		  <code>null</code> means drop catalog name from the selection criteria
	 * @param schemaPattern a schema name pattern; "" retrieves those
	 *		  without a schema
	 * @param typeNamePattern a UDT name pattern; may be a fully-qualified
	 *		  name
	 * @return a <code>ResultSet</code> object in which a row gives information
	 *		   about the designated UDT
	 * @throws SQLException if a database access error occurs
	 */
	public ResultSet getSuperTypes(
		String catalog,
		String schemaPattern,
		String typeNamePattern
	) throws SQLException
	{
		String query =
			"SELECT '' AS TYPE_CAT, '' AS TYPE_SCHEM, '' AS TYPE_NAME, " +
			"'' AS SUPERTYPE_CAT, '' AS SUPERTYPE_SCHEM, " +
			"'' AS SUPERTYPE_NAME WHERE 1 = 0";

		return(getStmt().executeQuery(query));
	}

	/**
	 * Retrieves a description of the table hierarchies defined in a particular
	 * schema in this database.
	 *
	 * <P>Only supertable information for tables matching the catalog, schema
	 * and table name are returned. The table name parameter may be a fully-
	 * qualified name, in which case, the catalog and schemaPattern parameters
	 * are ignored. If a table does not have a super table, it is not listed here.
	 * Supertables have to be defined in the same catalog and schema as the
	 * sub tables. Therefore, the type description does not need to include
	 * this information for the supertable.
	 *
	 * <P>Each type description has the following columns:
	 *	<OL>
	 *	<LI><B>TABLE_CAT</B> String => the type's catalog (may be <code>null</code>)
	 *	<LI><B>TABLE_SCHEM</B> String => type's schema (may be <code>null</code>)
	 *	<LI><B>TABLE_NAME</B> String => type name
	 *	<LI><B>SUPERTABLE_NAME</B> String => the direct super type's name
	 *	</OL>
	 *
	 * <P><B>Note:</B> If the driver does not support type hierarchies, an
	 * empty result set is returned.
	 *
	 * @param catalog a catalog name; "" retrieves those without a catalog;
	 *		  <code>null</code> means drop catalog name from the selection criteria
	 * @param schemaPattern a schema name pattern; "" retrieves those
	 *		  without a schema
	 * @param tableNamePattern a table name pattern; may be a fully-qualified
	 *		  name
	 * @return a <code>ResultSet</code> object in which each row is a type description
	 * @throws SQLException if a database access error occurs
	 */
	public ResultSet getSuperTables(
		String catalog,
		String schemaPattern,
		String tableNamePattern
	) throws SQLException
	{
		String query =
			"SELECT '' AS TABLE_CAT, '' AS TABLE_SCHEM, '' AS TABLE_NAME, " +
			"'' AS SUPERTABLE_NAME WHERE 1 = 0";

		return(getStmt().executeQuery(query));
	}

	/**
	 * Retrieves a description of the given attribute of the given type
	 * for a user-defined type (UDT) that is available in the given schema
	 * and catalog.
	 * <P>
	 * Descriptions are returned only for attributes of UDTs matching the
	 * catalog, schema, type, and attribute name criteria. They are ordered by
	 * TYPE_SCHEM, TYPE_NAME and ORDINAL_POSITION. This description
	 * does not contain inherited attributes.
	 * <P>
	 * The <code>ResultSet</code> object that is returned has the following
	 * columns:
	 * <OL>
	 *	<LI><B>TYPE_CAT</B> String => type catalog (may be <code>null</code>)
	 *	<LI><B>TYPE_SCHEM</B> String => type schema (may be <code>null</code>)
	 *	<LI><B>TYPE_NAME</B> String => type name
	 *	<LI><B>ATTR_NAME</B> String => attribute name
	 *	<LI><B>DATA_TYPE</B> short => attribute type SQL type from java.sql.Types
	 *	<LI><B>ATTR_TYPE_NAME</B> String => Data source dependent type name.
	 *	For a UDT, the type name is fully qualified. For a REF, the type name is
	 *	fully qualified and represents the target type of the reference type.
	 *	<LI><B>ATTR_SIZE</B> int => column size.  For char or date
	 *		types this is the maximum number of characters; for numeric or
	 *		decimal types this is precision.
	 *	<LI><B>DECIMAL_DIGITS</B> int => the number of fractional digits
	 *	<LI><B>NUM_PREC_RADIX</B> int => Radix (typically either 10 or 2)
	 *	<LI><B>NULLABLE</B> int => whether NULL is allowed
	 *		<UL>
	 *		<LI> attributeNoNulls - might not allow NULL values
	 *		<LI> attributeNullable - definitely allows NULL values
	 *		<LI> attributeNullableUnknown - nullability unknown
	 *		</UL>
	 *	<LI><B>REMARKS</B> String => comment describing column (may be <code>null</code>)
	 *	<LI><B>ATTR_DEF</B> String => default value (may be <code>null</code>)
	 *	<LI><B>SQL_DATA_TYPE</B> int => unused
	 *	<LI><B>SQL_DATETIME_SUB</B> int => unused
	 *	<LI><B>CHAR_OCTET_LENGTH</B> int => for char types the
	 *		 maximum number of bytes in the column
	 *	<LI><B>ORDINAL_POSITION</B> int => index of column in table
	 *		(starting at 1)
	 *	<LI><B>IS_NULLABLE</B> String => "NO" means column definitely
	 *		does not allow NULL values; "YES" means the column might
	 *		allow NULL values.	An empty string means unknown.
	 *	<LI><B>SCOPE_CATALOG</B> String => catalog of table that is the
	 *		scope of a reference attribute (<code>null</code> if DATA_TYPE isn't REF)
	 *	<LI><B>SCOPE_SCHEMA</B> String => schema of table that is the
	 *		scope of a reference attribute (<code>null</code> if DATA_TYPE isn't REF)
	 *	<LI><B>SCOPE_TABLE</B> String => table name that is the scope of a
	 *		reference attribute (<code>null</code> if the DATA_TYPE isn't REF)
	 * <LI><B>SOURCE_DATA_TYPE</B> short => source type of a distinct type or user-generated
	 *		Ref type,SQL type from java.sql.Types (<code>null</code> if DATA_TYPE
	 *		isn't DISTINCT or user-generated REF)
	 *	</OL>
	 * @param catalog a catalog name; must match the catalog name as it
	 *		  is stored in the database; "" retrieves those without a catalog;
	 *		  <code>null</code> means that the catalog name should not be used to narrow
	 *		  the search
	 * @param schemaPattern a schema name pattern; must match the schema name
	 *		  as it is stored in the database; "" retrieves those without a schema;
	 *		  <code>null</code> means that the schema name should not be used to narrow
	 *		  the search
	 * @param typeNamePattern a type name pattern; must match the
	 *		  type name as it is stored in the database
	 * @param attributeNamePattern an attribute name pattern; must match the attribute
	 *		  name as it is declared in the database
	 * @return a <code>ResultSet</code> object in which each row is an
	 *		   attribute description
	 * @throws SQLException if a database access error occurs
	 */
	public ResultSet getAttributes(
		String catalog,
		String schemaPattern,
		String typeNamePattern,
		String attributeNamePattern
	) throws SQLException
	{
		String query =
			"SELECT '' AS TYPE_CAT, '' AS TYPE_SCHEM, '' AS TYPE_NAME, " +
			"'' AS ATTR_NAME, '' AS ATTR_TYPE_NAME, 0 AS ATTR_SIZE, " +
			"0 AS DECIMAL_DIGITS, 0 AS NUM_PREC_RADIX, 0 AS NULLABLE, " +
			"'' AS REMARKS, '' AS ATTR_DEF, 0 AS SQL_DATA_TYPE, " +
			"0 AS SQL_DATETIME_SUB, 0 AS CHAR_OCTET_LENGTH, " +
			"0 AS ORDINAL_POSITION, 'YES' AS IS_NULLABLE, " +
			"'' AS SCOPE_CATALOG, '' AS SCOPE_SCHEMA, '' AS SCOPE_TABLE, " +
			"0 AS SOURCE_DATA_TYPE WHERE 1 = 0";

		return(getStmt().executeQuery(query));
	}

	/**
	 * Retrieves whether this database supports the given result set holdability.
	 *
	 * @param holdability one of the following constants:
	 *			<code>ResultSet.HOLD_CURSORS_OVER_COMMIT</code> or
	 *			<code>ResultSet.CLOSE_CURSORS_AT_COMMIT<code>
	 * @return <code>true</code> if so; <code>false</code> otherwise
	 * @see Connection
	 */
	public boolean supportsResultSetHoldability(int holdability) {
		return(holdability == ResultSet.HOLD_CURSORS_OVER_COMMIT);
	}

	/**
	 * Retrieves the default holdability of this <code>ResultSet</code>
	 * object.
	 *
	 * @return the default holdability; either
	 *		   <code>ResultSet.HOLD_CURSORS_OVER_COMMIT</code> or
	 *		   <code>ResultSet.CLOSE_CURSORS_AT_COMMIT</code>
	 */
	public int getResultSetHoldability() {
		return(ResultSet.HOLD_CURSORS_OVER_COMMIT);
	}

	/**
	 * Retrieves the major version number of the underlying database.
	 * We should actually retrieve this from the DB!!!
	 *
	 * @return the underlying database's major version
	 */
	public int getDatabaseMajorVersion() {
		return(4);
	}

	/**
	 * Retrieves the minor version number of the underlying database.
	 *
	 * @return underlying database's minor version
	 */
	public int getDatabaseMinorVersion() {
		return(3);
	}

	/**
	 * Retrieves the major JDBC version number for this
	 * driver.
	 *
	 * @return JDBC version major number
	 */
	public int getJDBCMajorVersion() {
		return(3); // This class implements JDBC 3.0 (at least we try to)
	}

	/**
	 * Retrieves the minor JDBC version number for this
	 * driver.
	 *
	 * @return JDBC version minor number
	 */
	public int getJDBCMinorVersion() {
		return(0); // This class implements JDBC 3.0 (at least we try to)
	}

	/**
	 * Indicates whether the SQLSTATEs returned by <code>SQLException.getSQLState</code>
	 * is X/Open (now known as Open Group) SQL CLI or SQL99.
	 * @return the type of SQLSTATEs, one of:
	 *		  sqlStateXOpen or
	 *		  sqlStateSQL99
	 */
	public int getSQLStateType() {
		// we conform to the SQL99 standard, so...
		return(DatabaseMetaData.sqlStateSQL99);
	}

	/**
	 * Indicates whether updates made to a LOB are made on a copy or directly
	 * to the LOB.
	 * @return <code>true</code> if updates are made to a copy of the LOB;
	 *		   <code>false</code> if updates are made directly to the LOB
	 */
	public boolean locatorsUpdateCopy() {
		// not that we have it, but in a transaction it will be copy-on-write
		return(true);
	}

	/**
	 * Retrieves whether this database supports statement pooling.
	 *
	 * @return <code>true</code> is so;
		<code>false</code> otherwise
	 */
	public boolean supportsStatementPooling() {
		// For the moment, I don't think so
		return(false);
	}
}

/**
 * This class is not intended for normal use. Therefore it is restricted to
 * classes from the very same package only. Because it is used only in the
 * MonetDatabaseMetaData class it is placed here.
 *
 * Any use of this class is disencouraged.
 */
class MonetVirtualResultSet extends MonetResultSet {
	private String results[][];

	MonetVirtualResultSet(
		String[] columns,
		String[] types,
		String[][] results
	) throws IllegalArgumentException {
		super(columns, types, results.length);

		this.results = results;
	}

	/**
	 * This method is overridden in order to let it use the results array
	 * instead of the cache in the Statement object that created it.
	 *
	 * @param row the number of the row to which the cursor should move. A
	 *        positive number indicates the row number counting from the
	 *        beginning of the result set; a negative number indicates the row
	 *        number counting from the end of the result set
	 * @return true if the cursor is on the result set; false otherwise
	 * @throws SQLException if a database error occurs
	 */
 	public boolean absolute(int row) throws SQLException {
		if (closed) throw new SQLException("ResultSet is closed!");

		// first calculate what the JDBC row is
		if (row < 0) {
			// calculate the negatives...
			row = tupleCount + row + 1;
		}
		// now place the row not farther than just before or after the result
		if (row < 0) row = 0;	// before first
		else if (row > tupleCount + 1) row = tupleCount + 1;	// after last

		// see if we have the row
		if (row < 1 || row > tupleCount) return(false);

		// store it
		curRow = row;

		result = results[row - 1];

		return(true);
	}

	/**
	 * Mainly here to prevent errors when the close method is called. There
	 * is no real need for this object to close it. We simply remove our
	 * resultset data.
	 */
	public void close() {
		if (!closed) {
			closed = true;
			results = null;
		}
	}
}
