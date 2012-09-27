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
 * Copyright August 2008-2012 MonetDB B.V.
 * All Rights Reserved.
 */


#include "monetdb_config.h"
#include "bin_optimizer.h"
#include "sql_types.h"
#include "sql_rel2bin.h"
#include "sql_env.h"

/* push this select through the statement s */
static stmt *
push_select( sql_allocator *sa, stmt *select, stmt *s )
{
	if (select->type == st_uselect2) 
		return stmt_uselect2(sa,  s, select->op2, select->op3, (comp_type)select->flag, select->op4.stval);

	if (select->type == st_uselect) 
		return stmt_uselect(sa,  s, select->op2, (comp_type)select->flag, select->op3);
	assert(0);
	return NULL;
}

static stmt *
_bin_optimizer(mvc *c, stmt *s)
{
	if (s->optimized >= 3) {
		if (s->rewritten)
			return s->rewritten;
		else
			return s;
	}

	switch (s->type) {
		/* first just return those statements which we cannot optimize,
		 * such as schema manipulation, transaction managment, 
		 * and user authentication.
		 */
	case st_none:
	case st_rs_column:
	case st_dbat:
	case st_basetable:
	case st_idxbat:

	case st_atom:
	case st_export:
	case st_var:
	case st_table_clear:

	case st_bat:

		s->optimized = 3;
		return s;

	case st_limit: {
		stmt *ns, *j = NULL, *os = s;

		/* try to push the limit through the (fetch) join */
		j = s->op1;

		/* push through the projection joins */
		if (s->flag == 0 &&
		    isEqJoin(j) &&
		    j->op1->t && j->op1->t == j->op2->h) {
			stmt *l = j->op1;
			stmt *r = j->op2;

			l = stmt_limit(c->sa, l, s->op2, s->op3, s->flag);
			s = stmt_join(c->sa, l, r, cmp_equal); 
			ns = _bin_optimizer(c, s);
			assert(os->rewritten==NULL);
			os->rewritten = ns;
			os->optimized = ns->optimized = 3;
			return ns;
		} else
		/* try to push the limit through the reverse */
		if (!s->flag && j->type == st_reverse) {
			s = stmt_reverse(c->sa, stmt_limit(c->sa, j->op1, s->op2,
				s->op3, s->flag));
			ns = _bin_optimizer(c, s);
			assert(os->rewritten==NULL);
			os->rewritten = ns;
			os->optimized = ns->optimized = 3;
			return ns;
		}
		/* try to push the limit through the mark (only if there is no offset) */
		if (!s->op2->op4.aval->data.val.wval && j->type == st_mark) {
			s = stmt_mark_tail(c->sa, stmt_limit(c->sa, j->op1,
				s->op2, s->op3, s->flag),
				j->op2->op4.aval->data.val.ival);
			ns = _bin_optimizer(c, s);
			assert(os->rewritten==NULL);
			os->rewritten = ns;
			os->optimized = ns->optimized = 3;
			return ns;
		}
		if (s->op1) {
			stmt *os = s->op1;
			stmt *ns = _bin_optimizer(c, os);

			assert(ns != s);
			s->op1 = ns;
		}
		s->optimized = 3;
		return s;
	}

	case st_inter:{

		stmt *j = NULL;
		stmt *os, *ns;

		os = stmt_inter(c->sa, _bin_optimizer(c, s->op1), _bin_optimizer(c, s->op2));
		/* try to push the intersect through the (fetch) join */
		if (os->op1->type == st_join) {
			j = os->op1;
			/* equi join on same base table */
			if (isEqJoin(j) &&
		    		j->op1->t == j->op2->h ) {
				stmt *l = j->op1;
				stmt *r = j->op2;
				s = stmt_inter(c->sa, l, os->op2);
				l = _bin_optimizer(c, s);
				os = stmt_join( c->sa, l, r, cmp_equal);
				os->optimized = 3; 
				return os;
			}
		} 
		ns = os;
		s->optimized = ns->optimized = 3;
		if (ns != s) {
			assert(s->rewritten==NULL);
			s->rewritten = ns;
		}
		return ns;
	}

	case st_join:
	case st_join2:
	case st_joinN:{

		if (s->op1) {
			stmt *os = s->op1;
			stmt *ns = _bin_optimizer(c, os);

			assert(ns != s);
			s->op1 = ns;
		}
		if (s->op2) {
			stmt *os = s->op2;
			stmt *ns = _bin_optimizer(c, os);

			assert(ns != s);
			s->op2 = ns;
		}
		if (s->op3) {
			stmt *os = s->op3;
			stmt *ns = _bin_optimizer(c, os);

			assert(ns != s);
			s->op3 = ns;
		}

		/* remove expensive double kdiffs 
		 * if join on oids from the same table then 	
		 * right kdiff is not needed 
		 */
		if (isEqJoin(s) && 
		    s->op1->t == s->op2->h &&
		    s->op2->type == st_diff){
			stmt *old = s->op2;
			s->op2 = old->op1;
		}
		/* same as above but now with alias in between */ 
		if (isEqJoin(s) && 
		    s->op2->type == st_alias &&
		    s->op1->t == s->op2->op1->h &&
		    s->op2->op1->type == st_diff){
			stmt *old = s->op2;
			s->op2 = old->op1->op1;
		}
		s->optimized = 3;
		return s;
	}

	case st_reverse:{

		stmt *os, *ns;

		os = stmt_reverse(c->sa, _bin_optimizer(c, s->op1));
		ns = os;
		s->optimized = ns->optimized = 3;
		if (ns != s) {
			assert(s->rewritten==NULL);
			s->rewritten = ns;
		}
		return ns;
	}
	case st_uselect:
	case st_uselect2: {
		stmt *res = NULL;

		if (s->flag == cmp_filter) {
			s->optimized = 3;
			return s;
		}
			
		/* push down the select through st_alias */
		if (s->op1->type == st_alias) {
			stmt *a = s->op1;
	
			s = push_select( c->sa, s, a->op1); 
			s = stmt_alias(c->sa, s, table_name(c->sa, a), column_name(c->sa, a));
			res = _bin_optimizer(c, s);
			return res;
		}
		/* push down the select through st_diff */
		if (s->op1->type == st_diff && s->flag != cmp_notequal) {
			stmt *d = s->op1;
	
			s = push_select( c->sa, s, d->op1); 
			s = stmt_diff(c->sa, s, d->op2);
			res = _bin_optimizer(c, s);
			return res;
		}
		/* push down the select through st_union */
		if (s->op1->type == st_union && s->flag != cmp_notequal) {
			stmt *l, *r;
			stmt *u = s->op1;
	
			l = push_select( c->sa, s, u->op1); 
			r = push_select( c->sa, s, u->op2);
			s = stmt_union(c->sa, l, r);
			res = _bin_optimizer(c, s);
			return res;
		}

		if (s->op1) {
			stmt *os = s->op1;
			stmt *ns = _bin_optimizer(c, os);

			assert(ns != s);
			s->op1 = ns;
		}
		if (s->op2) {
			stmt *os = s->op2;
			stmt *ns = _bin_optimizer(c, os);

			assert(ns != s);
			s->op2 = ns;
		}
		s->optimized = 3;
		return s;
	}

	case st_tinter:
	case st_tdiff:
	case st_temp:
	case st_single:
	case st_diff:
	case st_union:
	case st_mirror:
	case st_const:
	case st_mark:
	case st_gen_group:
	case st_group:
	case st_unique:
	case st_order:
	case st_reorder:
	case st_ordered:
	case st_limit2:
	case st_sample:

	case st_alias:
	case st_append:
	case st_exception:
	case st_trans:
	case st_catalog:

	case st_aggr:
	case st_unop:
	case st_binop:
	case st_Nop:
	case st_func:
	case st_convert:

	case st_affected_rows:
	case st_table:

	case st_cond:
	case st_control_end:
	case st_return:
	case st_assign:

	case st_output: 

	case st_append_col:
	case st_append_idx:
	case st_update_col:
	case st_update_idx:
	case st_delete:

		if (s->op1) {
			stmt *os = s->op1;
			stmt *ns = _bin_optimizer(c, os);

			assert(ns != s);
			s->op1 = ns;
		}

		if (s->op2) {
			stmt *os = s->op2;
			stmt *ns = _bin_optimizer(c, os);
	
			assert(ns != s);
			s->op2 = ns;
		}
		if (s->op3) {
			stmt *os = s->op3;
			stmt *ns = _bin_optimizer(c, os);
	
			assert(ns != s);
			s->op3 = ns;
		}
		s->optimized = 3;
		return s;

	case st_list:{

		stmt *res = NULL;
		node *n;
		list *l = s->op4.lval;
		list *nl = NULL;

		nl = sa_list(c->sa);
		for (n = l->h; n; n = n->next) {
			stmt *ns = _bin_optimizer(c, n->data);

			list_append(nl, ns);
		}
		res = stmt_list(c->sa, nl);
		s->optimized = res->optimized = 3;
		if (res != s) {
			assert(s->rewritten==NULL);
			s->rewritten = res;
		}
		return res;
	}

	case st_releqjoin:

	default:
		assert(0);	/* these should have been rewriten by now */
	}
	return s;
}

stmt *
bin_optimizer(mvc *c, stmt *s)
{
	stmt **stmts = stmt_array(c->sa, s);

	int nr = 0;

	/*print_stmts(c->sa, stmts);*/
	clear_stmts(stmts);
	while (stmts[nr] ) {
		stmt *s = stmts[nr++];
		_bin_optimizer(c, s);
	}
	return _bin_optimizer(c, s);
}
