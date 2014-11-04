/*
 * The contents of this file are subject to the MonetDB Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.monetdb.org/Legal/MonetDBLicense
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is the MonetDB Database System.
 *
 * The Initial Developer of the Original Code is CWI.
 * Portions created by CWI are Copyright (C) 1997-July 2008 CWI.
 * Copyright August 2008-2014 MonetDB B.V.
 * All Rights Reserved.
 */

/*
 * SQL execution
 * N. Nes, M.L. Kersten
 */
/*
 * Execution of SQL instructions.
 * Before we are can process SQL statements the global catalog
 * should be initialized. Thereafter, each time a client enters
 * we update its context descriptor to denote an SQL scenario.
 */
#include "monetdb_config.h"
#include "mal_backend.h"
#include "sql_scenario.h"
#include "sql_result.h"
#include "sql_gencode.h"
#include "sql_optimizer.h"
#include "sql_env.h"
#include "sql_mvc.h"
#include "sql_readline.h"
#include "sql_user.h"
#include "sql_execute.h"
#include "sql_datetime.h"
#include "mal_io.h"
#include "mal_parser.h"
#include "mal_builder.h"
#include "mal_namespace.h"
#include "mal_debugger.h"
#include "mal_linker.h"
#include "bat5.h"
#include "msabaoth.h"
#include <mtime.h>
#include "optimizer.h"
#include "opt_statistics.h"
#include "opt_prelude.h"
#include "opt_pipes.h"
#include <unistd.h>
#include "sql_upgrades.h"

/*
 * The SQLcompile operation can be used by separate
 * front-ends to benefit from the SQL functionality.
 * It expects a string and returns the name of the
 * corresponding MAL block as it is known in the
 * SQL_cache, where it can be picked up.
 * The SQLstatement operation also executes the instruction upon request.
 *
 * In both cases the SQL string is handled like an ordinary
 * user query, following the same optimization paths and
 * caching.
 */

/* #define _SQL_COMPILE */

/*
BEWARE: SQLstatementIntern only commits after all statements found
in expr are executed, when autocommit mode is enabled.
*/
str
SQLstatementIntern(Client c, str *expr, str nme, int execute, bit output, res_table **result)
{
	int status = 0;
	int err = 0;
	mvc *o, *m;
	int ac, sizevars, topvars;
	sql_var *vars;
	buffer *b;
	char *n;
	stream *buf;
	str msg = MAL_SUCCEED;
	backend *be, *sql = (backend *) c->sqlcontext;
	size_t len = strlen(*expr);

#ifdef _SQL_COMPILE
	mnstr_printf(c->fdout, "#SQLstatement:%s\n", *expr);
#endif
	if (!sql) {
		msg = SQLinitEnvironment(c, NULL, NULL, NULL);
		sql = (backend *) c->sqlcontext;
	}
	if (msg){
		GDKfree(msg);
		throw(SQL, "SQLstatement", "Catalogue not available");
	}

	initSQLreferences();
	m = sql->mvc;
	ac = m->session->auto_commit;
	o = MNEW(mvc);
	if (!o)
		throw(SQL, "SQLstatement", "Out of memory");
	*o = *m;

	/* create private allocator */
	m->sa = NULL;
	SQLtrans(m);
	status = m->session->status;

	m->type = Q_PARSE;
	be = sql;
	sql = backend_create(m, c);
	sql->output_format = be->output_format;
	m->qc = NULL;
	m->caching = 0;
	m->user_id = m->role_id = USER_MONETDB;
	if (result)
		m->reply_size = -2; /* do not cleanup, result tables */

	b = (buffer *) GDKmalloc(sizeof(buffer));
	n = GDKmalloc(len + 1 + 1);
	strncpy(n, *expr, len);
	n[len] = '\n';
	n[len + 1] = 0;
	len++;
	buffer_init(b, n, len);
	buf = buffer_rastream(b, "sqlstatement");
	scanner_init(&m->scanner, bstream_create(buf, b->len), NULL);
	m->scanner.mode = LINE_N;
	bstream_next(m->scanner.rs);

	m->params = NULL;
	m->argc = 0;
	m->session->auto_commit = 0;

	if (!m->sa)
		m->sa = sa_create();
	/*
	 * System has been prepared to parse it and generate code.
	 * Scan the complete string for SQL statements, stop at the first error.
	 */
	c->sqlcontext = sql;
	while (msg == MAL_SUCCEED && m->scanner.rs->pos < m->scanner.rs->len) {
		sql_rel *r;
		stmt *s;
		int oldvtop, oldstop;
		MalStkPtr oldglb = c->glb;

		if (!m->sa)
			m->sa = sa_create();
		m->sym = NULL;
		if ((err = sqlparse(m)) ||
		    /* Only forget old errors on transaction boundaries */
		    (mvc_status(m) && m->type != Q_TRANS) || !m->sym) {
			if (!err)
				err = mvc_status(m);
			if (*m->errstr)
				msg = createException(PARSE, "SQLparser", "%s", m->errstr);
			*m->errstr = 0;
			sqlcleanup(m, err);
			execute = 0;
			if (!err)
				continue;
			assert(c->glb == 0 || c->glb == oldglb);	/* detect leak */
			c->glb = oldglb;
			goto endofcompile;
		}

		/*
		 * We have dealt with the first parsing step and advanced the input reader
		 * to the next statement (if any).
		 * Now is the time to also perform the semantic analysis,
		 * optimize and produce code.
		 * We don't search the cache for a previous incarnation yet.
		 */
		MSinitClientPrg(c, "user", nme);
		oldvtop = c->curprg->def->vtop;
		oldstop = c->curprg->def->stop;
		r = sql_symbol2relation(m, m->sym);
		s = sql_relation2stmt(m, r);
#ifdef _SQL_COMPILE
		mnstr_printf(c->fdout, "#SQLstatement:\n");
#endif
		scanner_query_processed(&(m->scanner));
		if (s == 0 || (err = mvc_status(m))) {
			msg = createException(PARSE, "SQLparser", "%s", m->errstr);
			handle_error(m, c->fdout, status);
			sqlcleanup(m, err);
			/* restore the state */
			MSresetInstructions(c->curprg->def, oldstop);
			freeVariables(c, c->curprg->def, c->glb, oldvtop);
			c->curprg->def->errors = 0;
			assert(c->glb == 0 || c->glb == oldglb);	/* detect leak */
			c->glb = oldglb;
			goto endofcompile;
		}
		/* generate MAL code */
		if (backend_callinline(sql, c, s) == 0)
			addQueryToCache(c);
		else
			err = 1;

		if (err ||c->curprg->def->errors) {
			/* restore the state */
			MSresetInstructions(c->curprg->def, oldstop);
			freeVariables(c, c->curprg->def, c->glb, oldvtop);
			c->curprg->def->errors = 0;
			msg = createException(SQL, "SQLparser", "Errors encountered in query");
			assert(c->glb == 0 || c->glb == oldglb);	/* detect leak */
			c->glb = oldglb;
			goto endofcompile;
		}
#ifdef _SQL_COMPILE
		mnstr_printf(c->fdout, "#result of sql.eval()\n");
		printFunction(c->fdout, c->curprg->def, 0, c->listing);
#endif

		if (execute) {
			MalBlkPtr mb = c->curprg->def;

			if (!output)
				sql->out = NULL;	/* no output */
			msg = runMAL(c, mb, 0, 0);
			MSresetInstructions(mb, oldstop);
			freeVariables(c, mb, NULL, oldvtop);
		}
		sqlcleanup(m, 0);
		if (!execute) {
			assert(c->glb == 0 || c->glb == oldglb);	/* detect leak */
			c->glb = oldglb;
			goto endofcompile;
		}
#ifdef _SQL_COMPILE
		mnstr_printf(c->fdout, "#parse/execute result %d\n", err);
#endif
		assert(c->glb == 0 || c->glb == oldglb);	/* detect leak */
		c->glb = oldglb;
	}
	if (m->results && result) { /* return all results sets */
		*result = m->results;
		m->results = NULL;
	}
/*
 * We are done; a MAL procedure resides in the cache.
 */
endofcompile:
	if (execute)
		MSresetInstructions(c->curprg->def, 1);

	c->sqlcontext = be;
	backend_destroy(sql);
	GDKfree(n);
	GDKfree(b);
	bstream_destroy(m->scanner.rs);
	if (m->sa)
		sa_destroy(m->sa);
	m->sa = NULL;
	m->sym = NULL;
	/* variable stack maybe resized, ie we need to keep the new stack */
	status = m->session->status;
	sizevars = m->sizevars;
	topvars = m->topvars;
	vars = m->vars;
	*m = *o;
	_DELETE(o);
	m->sizevars = sizevars;
	m->topvars = topvars;
	m->vars = vars;
	m->session->status = status;
	m->session->auto_commit = ac;
	return msg;
}

/*
 * Execution of the SQL program is delegated to the MALengine.
 * Different cases should be distinguished. The default is to
 * hand over the MAL block derived by the parser for execution.
 * However, when we received an Execute call, we make a shortcut
 * and prepare the stack for immediate execution
 */
str
SQLexecutePrepared(Client c, backend *be, cq *q)
{
	mvc *m = be->mvc;
	int argc, parc;
	ValPtr *argv, argvbuffer[MAXARG], v;
	ValRecord *argrec, argrecbuffer[MAXARG];
	MalBlkPtr mb;
	MalStkPtr glb;
	InstrPtr pci;
	int i;
	str ret;
	Symbol qcode = q->code;

	if (!qcode || qcode->def->errors) {
		if (!qcode && *m->errstr)
			return createException(PARSE, "SQLparser", "%s", m->errstr);
		throw(SQL, "SQLengine", "39000!program contains errors");
	}
	mb = qcode->def;
	pci = getInstrPtr(mb, 0);
	if (pci->argc >= MAXARG)
		argv = (ValPtr *) GDKmalloc(sizeof(ValPtr) * pci->argc);
	else
		argv = argvbuffer;

	if (pci->retc >= MAXARG)
		argrec = (ValRecord *) GDKmalloc(sizeof(ValRecord) * pci->retc);
	else
		argrec = argrecbuffer;

	/* prepare the target variables */
	for (i = 0; i < pci->retc; i++) {
		argv[i] = argrec + i;
		argv[i]->vtype = getVarGDKType(mb, i);
	}

	argc = m->argc;
	parc = q->paramlen;

	if (argc != parc) {
		if (pci->argc >= MAXARG)
			GDKfree(argv);
		if (pci->retc >= MAXARG)
			GDKfree(argrec);
		throw(SQL, "sql.prepare", "07001!EXEC: wrong number of arguments for prepared statement: %d, expected %d", argc, parc);
	} else {
		for (i = 0; i < m->argc; i++) {
			atom *arg = m->args[i];
			sql_subtype *pt = q->params + i;

			if (!atom_cast(arg, pt)) {
				/*sql_error(c, 003, buf); */
				if (pci->argc >= MAXARG)
					GDKfree(argv);
				if (pci->retc >= MAXARG)
					GDKfree(argrec);
				throw(SQL, "sql.prepare", "07001!EXEC: wrong type for argument %d of " "prepared statement: %s, expected %s", i + 1, atom_type(arg)->type->sqlname, pt->type->sqlname);
			}
			argv[pci->retc + i] = &arg->data;
		}
	}
	glb = (MalStkPtr) (q->stk);
	ret = callMAL(c, mb, &glb, argv, (m->emod & mod_debug ? 'n' : 0));
	/* cleanup the arguments */
	for (i = pci->retc; i < pci->argc; i++) {
		garbageElement(c, v = &glb->stk[pci->argv[i]]);
		v->vtype = TYPE_int;
		v->val.ival = int_nil;
	}
	q->stk = (backend_stack) glb;
	if (glb && SQLdebug & 1)
		printStack(GDKstdout, mb, glb);
	if (pci->argc >= MAXARG)
		GDKfree(argv);
	if (pci->retc >= MAXARG)
		GDKfree(argrec);
	return ret;
}

str
SQLengineIntern(Client c, backend *be)
{
	str msg = MAL_SUCCEED;
	MalStkPtr oldglb = c->glb;
	char oldlang = be->language;
	mvc *m = be->mvc;
	InstrPtr p;
	MalBlkPtr mb;

	if (oldlang == 'X') {	/* return directly from X-commands */
		sqlcleanup(be->mvc, 0);
		return MAL_SUCCEED;
	}

	if (m->emod & mod_explain) {
		if (be->q && be->q->code)
			printFunction(c->fdout, ((Symbol) (be->q->code))->def, 0, LIST_MAL_STMT | LIST_MAL_UDF | LIST_MAPI);
		else if (be->q)
			msg = createException(PARSE, "SQLparser", "%s", (*m->errstr) ? m->errstr : "39000!program contains errors");
		else if (c->curprg->def)
			printFunction(c->fdout, c->curprg->def, 0, LIST_MAL_STMT | LIST_MAL_UDF | LIST_MAPI);
		goto cleanup_engine;
	}
	if (m->emod & mod_dot) {
		if (be->q && be->q->code)
			showFlowGraph(((Symbol) (be->q->code))->def, 0, "stdout-mapi");
		else if (be->q)
			msg = createException(PARSE, "SQLparser", "%s", (*m->errstr) ? m->errstr : "39000!program contains errors");
		else if (c->curprg->def)
			showFlowGraph(c->curprg->def, 0, "stdout-mapi");
		goto cleanup_engine;
	}
#ifdef SQL_SCENARIO_DEBUG
	mnstr_printf(GDKout, "#Ready to execute SQL statement\n");
#endif

	if (c->curprg->def->stop == 1) {
		sqlcleanup(be->mvc, 0);
		return MAL_SUCCEED;
	}

	if (m->emode == m_inplace) {
		msg = SQLexecutePrepared(c, be, be->q);
		goto cleanup_engine;
	}

	if (m->emode == m_prepare)
		goto cleanup_engine;

	assert(c->glb == 0 || c->glb == oldglb);	/* detect leak */
	c->glb = 0;
	be->language = 'D';
	/*
	 * The code below is copied from MALengine, which handles execution
	 * in the context of a user global environment. We have a private
	 * environment.
	 */
	if (MALcommentsOnly(c->curprg->def)) {
		msg = MAL_SUCCEED;
	} else {
		msg = (str) runMAL(c, c->curprg->def, 0, 0);
	}

cleanup_engine:
	if (m->type == Q_SCHEMA)
		qc_clean(m->qc);
	if (msg) {
		enum malexception type = getExceptionType(msg);
		if (type == OPTIMIZER) {
			MSresetInstructions(c->curprg->def, 1);
			freeVariables(c, c->curprg->def, NULL, be->vtop);
			be->language = oldlang;
			assert(c->glb == 0 || c->glb == oldglb);	/* detect leak */
			c->glb = oldglb;
			if ( msg)
				GDKfree(msg);
			return SQLrecompile(c, be); // retry compilation
		} else {
			/* don't print exception decoration, just the message */
			char *n = NULL;
			char *o = msg;
			while ((n = strchr(o, '\n')) != NULL) {
				*n = '\0';
				mnstr_printf(c->fdout, "!%s\n", getExceptionMessage(o));
				*n++ = '\n';
				o = n;
			}
			if (*o != 0)
				mnstr_printf(c->fdout, "!%s\n", getExceptionMessage(o));
		}
		showErrors(c);
		m->session->status = -10;
	}

	mb = c->curprg->def;
	if (m->type != Q_SCHEMA && be->q && msg) {
		qc_delete(m->qc, be->q);
	} else if (m->type != Q_SCHEMA && be->q && mb && varGetProp(mb, getArg(p = getInstrPtr(mb, 0), 0), runonceProp)) {
		msg = SQLCacheRemove(c, getFunctionId(p));
		qc_delete(be->mvc->qc, be->q);
		///* this should invalidate any match */
		//be->q->key= -1;
		//be->q->paramlen = -1;
		///* qc_delete(be->q) */
	}
	be->q = NULL;
	sqlcleanup(be->mvc, (!msg) ? 0 : -1);
	MSresetInstructions(c->curprg->def, 1);
	freeVariables(c, c->curprg->def, NULL, be->vtop);
	be->language = oldlang;
	/*
	 * Any error encountered during execution should block further processing
	 * unless auto_commit has been set.
	 */
	assert(c->glb == 0 || c->glb == oldglb);	/* detect leak */
	c->glb = oldglb;
	return msg;
}
