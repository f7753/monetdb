/**
 * @file
 *
 * Copyright Notice:
 * -----------------
 *
 *  The contents of this file are subject to the MonetDB Public
 *  License Version 1.0 (the "License"); you may not use this file
 *  except in compliance with the License. You may obtain a copy of
 *  the License at http://monetdb.cwi.nl/Legal/MonetDBLicense-1.0.html
 *
 *  Software distributed under the License is distributed on an "AS
 *  IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 *  implied. See the License for the specific language governing
 *  rights and limitations under the License.
 *
 *  The Original Code is the ``Pathfinder'' system. The Initial
 *  Developer of the Original Code is the Database & Information
 *  Systems Group at the University of Konstanz, Germany. Portions
 *  created by U Konstanz are Copyright (C) 2000-2005 University
 *  of Konstanz. All Rights Reserved.
 *
 *  Contributors:
 *          Peter Boncz <boncz@cwi.nl>
 *
 * $Id$
 */

#include <stdio.h>

/* ----------------------------------------------------------------------------
 * dead MIL code eliminator (one-day hack by Peter Boncz) 
 *
 * accepts chunks of MIL at a time
 * echoes optimized MIL to a file pointer
 *
 * limitations: 
 * - don't declare more than one variable in a line "var x := 1, y;", if any but the last is assigned to. 
 * - don't generate MIL statements with multiple (i.e. inline) assignments "x := (y := 1) + 1;"
 * - for better pruning: use assignment notation for update statements "x := x.insert(y);"
 * ----------------------------------------------------------------------------
 */

#define OPT_STMTS 32767 /* <65535 if first-use further out than this amount of stamements, dead codes survises anyhow */
#define OPT_VARS 1024 /* don't try dead code elimintation above this amount of live variables */
#define OPT_REFS 64 /* keep track of usage dependencies; may omit some, which results in surviving dead code */
#define OPT_CONDS 32  /* maximum if-nesting */

#define OPT_SEC_PROLOGUE 0
#define OPT_SEC_QUERY 1
#define OPT_SEC_EPILOGUE 2

typedef struct {
        char *mil; /* buffered line of MIL */
        unsigned int stmt_nr:30,sec:2; /* absolute statement number in MIL input */
        unsigned int used:16,inactive:1,delchar:7,nilassign:1,refs:7; 
        /* used:      becomes true if this variable was used */ 
        /* inactive:  set if we have tried to eliminate this statement already */
	/* delchar:   we separate statements by substituting a ';' char (or \n for comments) */
        /* nilassign: special treatment: nil assignments for early memory reduction are never pruned */
        /* refs:      number of variable references found */
        unsigned short refstmt[OPT_REFS]; /* variable references found on this stmt */
} opt_stmt_t;

typedef struct {
        long long prefix[2]; /* first 12 chars, last integer actually stores suffix length */
        char* suffix;
} opt_name_t; /* string representation designed for fast equality check */

#define OPT_NAME_SUFFIXLEN(x) ((unsigned int*) (x))[3] 
#define OPT_NAME_EQ(x,y) (((((x)->prefix[0] == (y)->prefix[0]) & ((x)->prefix[1] == (y)->prefix[1])) & \
			    (OPT_NAME_SUFFIXLEN(x) == 0)) | \
			 (((((x)->prefix[0] == (y)->prefix[0]) & ((x)->prefix[1] == (y)->prefix[1])) & \
			    (OPT_NAME_SUFFIXLEN(x) != 0)) && \
			   (strncmp((x)->suffix, (y)->suffix, OPT_NAME_SUFFIXLEN(x)) == 0)))

typedef struct {
        opt_name_t name; /* variable name  */
        unsigned short scope; /* scope in which var was defined */

	/* a set of statements that last assigned to it (may be multiple due to if-then-else) */
        unsigned short lastset[OPT_REFS]; 
        unsigned int stmt_nr[OPT_REFS]; 
	unsigned char setmax; /* last occupied position  in lastset/stmt_nr */


	/* range alive references in lastset/stmt_nr for a certain 'cond' */
	unsigned char always; /* bitmask: var always assigned in cond (bitpos)? */
	unsigned char setlo[OPT_CONDS+OPT_CONDS];
	unsigned char sethi[OPT_CONDS+OPT_CONDS];
} opt_var_t;

/* for each condlevel, there is a cond (even) for the if and (odd) for the else block 
 * 0 = root, 1 = unused, 2 = first iflevel 3= first elselevel, 4 = second iflevel, etc 
 */
#define OPT_COND(o) ((o)->condlevel + (o)->condlevel + (o)->condifelse[(o)->condlevel])

typedef struct {
        unsigned int curstmt; /* number of detected MIL statements so far */
        unsigned int scope; /* current scope depth */
        unsigned int curvar; /* length of variable stack */
        unsigned int iflevel; /* how many conditionals have we passed? */
        unsigned int optimize; 

	unsigned char if_statement; 
	unsigned char else_statement;
        unsigned short condlevel; /* number of nested conditional blocks recorded on stack (<OPT_CONDS) */
        unsigned short condscopes[OPT_CONDS]; /* scopes where each conditional block starts */
        unsigned char condifelse[OPT_CONDS]; /* scopes where each conditional block starts */

        opt_stmt_t stmts[OPT_STMTS]; /* line buffer */
        opt_var_t vars[OPT_VARS]; /* variable stack */
	char *buf[3];
	size_t off[3], len[3];
	int sec;
        FILE *fp; 
} opt_t; /* should take ~1.5MB of RAM resources */

#define opt_output(o,x) ((o)->sec = x)

opt_t *opt_open(int optimize);
void opt_close(opt_t *o, char** prologue, char** query, char** epilogue);
void opt_mil(opt_t *o, char* milbuf);
