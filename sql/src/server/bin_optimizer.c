/*
 * The contents of this file are subject to the MonetDB Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://monetdb.cwi.nl/Legal/MonetDBLicense-1.1.html
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
 * Copyright August 2008-2010 MonetDB B.V.
 * All Rights Reserved.
 */


#include "sql_config.h"
#include "bin_optimizer.h"
#include "sql_types.h"
#include "sql_rel2bin.h"
#include "sql_env.h"

static stmt *
stmt_uselect_select(stmt *s)
{
	assert(s->type == st_uselect2 || (s->type == st_uselect && !s->op2.stval->nrcols));
	if (s->type == st_uselect2)
		s->type = st_select2;
	else
		s->type = st_select;
	assert(!s->t);
	s->t = stmt_dup(s->op1.stval->t);
	return s;
}

static stmt *
eliminate_semijoin(stmt *s)
{
	stmt *s1, *s2;
	sql_column *bc1, *bc2;

	assert(s->type == st_semijoin);
	s1 = s->op1.stval;
	s2 = s->op2.stval;
	bc1 = basecolumn(s1);
	bc2 = basecolumn(s2);
	if (bc1 && bc1 == bc2) {
		int match1 = (PSEL(s1) || RSEL(s1));
		int match2 = (PSEL(s2) || RSEL(s2));

		if (match1 && match2) {
			/* semijoin( select(x,..), select(x,..) ) */
			int swap = 0;

			if (PSEL(s1) && s1->flag == cmp_equal) {
				/* do point select first */
				swap = 0;
			} else if (PSEL(s2) && s2->flag == cmp_equal) {
				/* do point select first */
				swap = 1;
			} else if (PSEL(s2) && s2->flag == cmp_notequal) {
				/* do notequal select last */
				swap = 0;
			} else if (PSEL(s1) && s1->flag == cmp_notequal) {
				/* do notequal select last */
				swap = 1;
			} else if (PSEL(s2) &&
					(s2->flag == cmp_notlike || s2->flag == cmp_notilike))
			{
				/* do notequal select last */
				swap = 0;
			} else if (PSEL(s1) &&
					(s1->flag == cmp_notlike || s1->flag == cmp_notilike))
			{
				/* do notequal select last */
				swap = 1;
			} else if (PSEL(s2) &&
					(s2->flag == cmp_like || s2->flag == cmp_ilike))
			{
				/* do like select last */
				swap = 0;
			} else if (PSEL(s1) &&
					(s1->flag == cmp_like || s1->flag == cmp_ilike))
			{
				/* do like select last */
				swap = 1;
			} else if (PSEL(s1)) {
				/* single-sided range before double-sided range */
				swap = 0;
			} else if (PSEL(s2)) {
				/* single-sided range before double-sided range */
				swap = 1;
			}
			if (swap) {
				stmt *os;

				os = s1;
				s1 = s2;
				s2 = os;
			}
			if (USEL(s1)) {
				/* uselect => select  to keep tail for s2 */
				s1 = stmt_uselect_select(s1);
			}
		} else if (match1) {
			/* semijoin( select(x,..), f(x) )  =>  semijoin( f(x), select(x,..) ) */
			stmt *os;
			int m;

			m = match1;
			match1 = match2;
			match2 = m;
			os = s1;
			s1 = s2;
			s2 = os;
		}
		if (match2 && 0) {
			/* semijoin( f(x), select(x,..) )  =>  select( f(x), .. ) */
			stmt *ns = NULL;

			switch (s2->type) {
			case st_select:
			case st_uselect:
				/* uselect => select  as semijoin also propagates the left input's tail */
				ns = stmt_select(stmt_dup(s1), stmt_dup(s2->op2.stval), (comp_type) s2->flag);
				break;
			case st_select2:
			case st_uselect2:
				/* uselect => select  as semijoin also propagates the left input's tail */
				ns = stmt_select2(stmt_dup(s1), stmt_dup(s2->op2.stval), stmt_dup(s2->op3.stval), s2->flag);
				break;
			default:
				/* pacify compiler; should never be reached. */
				assert(0);
			}
			return ns;
		}
	}
	return stmt_dup(s);
}

static stmt *
eliminate_reverse(stmt *s)
{
	stmt *os = s->op1.stval, *ns;

	assert(s->type == st_reverse);
	switch (os->type) {
	case st_reverse:
		/* reverse(reverse(x)) => x */
		ns = os->op1.stval;
		break;
	case st_mirror:
		/* reverse(mirror(x)) => mirror(x) */
		ns = os;
		break;
	default:
		ns = s;
	}
	return stmt_dup(ns);
}

/* push this select through the statement s */
static stmt *
push_select( stmt *select, stmt *s )
{
	if (select->type == st_select2) 
		return stmt_select2( s, stmt_dup(select->op2.stval), stmt_dup(select->op3.stval), (comp_type)select->flag);

	if (select->type == st_uselect2) 
		return stmt_uselect2( s, stmt_dup(select->op2.stval), stmt_dup(select->op3.stval), (comp_type)select->flag);

	if (select->type == st_select) {
		if (select->flag == cmp_like || select->flag == cmp_notlike ||
		    select->flag == cmp_ilike || select->flag == cmp_notilike)
			return stmt_likeselect(s, stmt_dup(select->op2.stval),
					stmt_dup(select->op3.stval), (comp_type)select->flag);
		else
			return stmt_select(s, stmt_dup(select->op2.stval),
					(comp_type)select->flag);
	}

	if (select->type == st_uselect) 
		return stmt_uselect( s, stmt_dup(select->op2.stval), (comp_type)select->flag);
	assert(0);
	return NULL;
}

static int 
is_reduced( stmt *s )
{
	switch(s->type) 
	{
	case st_relselect:
	case st_select:
	case st_select2:
	case st_uselect:
	case st_uselect2:
	case st_semijoin:
	case st_limit:
	case st_releqjoin:
	case st_join2:
	case st_joinN:
		return 1;

	case st_join:
		/* fetch join, look at the join operands */
		if (isEqJoin(s) &&
		    s->op1.stval->t && s->op1.stval->t == s->op2.stval->h)
			return  is_reduced(s->op1.stval) + 
				is_reduced(s->op2.stval);
		return 1;
	case st_union:	
		return is_reduced(s->op1.stval) + is_reduced(s->op2.stval);
	case st_reverse:
	case st_mark:
	case st_diff:	
		return is_reduced(s->op1.stval);
	default:
		return 0;
	}
}


stmt *
bin_optimizer(mvc *c, stmt *s)
{
	if (s->optimized >= 3) {
		if (s->rewritten)
			return stmt_dup(s->rewritten);
		else
			return stmt_dup(s);
	}

	switch (s->type) {
		/* first just return those statements which we cannot optimize,
		 * such as schema manipulation, transaction managment, 
		 * and user authentication.
		 */
	case st_none:
	case st_connection:
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
		return stmt_dup(s);

	case st_limit: {
		stmt *ns, *j = NULL, *os = s;

		/* try to push the limit through the (fetch) join */
		j = s->op1.stval;

		/* push through the projection joins */
		if (!s->op4.stval &&
		    s->flag == 0 &&
		    isEqJoin(j) &&
		    j->op1.stval->t && j->op1.stval->t == j->op2.stval->h) {
			stmt *l = stmt_dup(j->op1.stval);
			stmt *r = stmt_dup(j->op2.stval);

			l = stmt_limit(l, 
			 	stmt_dup(s->op2.stval),
				stmt_dup(s->op3.stval),
			      	s->flag);
			s = stmt_join(l, r, cmp_equal); 
			ns = bin_optimizer(c, s);
			stmt_destroy(s);
			assert(os->rewritten==NULL);
			os->rewritten = stmt_dup(ns);
			os->optimized = ns->optimized = 3;
			return ns;
		} else
		/* incase there is no reduction on the left, push
		   the topn into the right part */
		if (!s->op4.stval && 
		    s->flag != 0 /* topn, ie limit and orderby */ &&
		    j->type == st_join && 
		  ((isEqJoin(j) &&
		    !is_reduced(j) &&
		    j->op1.stval->t && j->op1.stval->t == j->op2.stval->h) || 
		    j->flag == cmp_all)) {
			stmt *l = stmt_dup(j->op1.stval);
			stmt *r = stmt_dup(j->op2.stval);

			r = stmt_limit(r, 
			 	stmt_dup(s->op2.stval),
				stmt_dup(s->op3.stval),
			      	s->flag);
			s = stmt_join(l, r, cmp_equal); 
			ns = bin_optimizer(c, s);
			stmt_destroy(s);
			assert(os->rewritten==NULL);
			os->rewritten = stmt_dup(ns);
			os->optimized = ns->optimized = 3;
			return ns;
		}
		/* try to push the limit through the reverse */
		if (!s->op4.stval && !s->flag && j->type == st_reverse) {
			s = stmt_reverse(stmt_limit(stmt_dup(j->op1.stval),
				stmt_dup(s->op2.stval),
				stmt_dup(s->op3.stval),
				s->flag));
			ns = bin_optimizer(c, s);
			stmt_destroy(s);
			assert(os->rewritten==NULL);
			os->rewritten = stmt_dup(ns);
			os->optimized = ns->optimized = 3;
			return ns;
		}
		/* try to push the limit through the mark (only if there is no offset) */
		if (!s->op4.stval && !s->op2.stval->op1.aval->data.val.wval && j->type == st_mark) {
			s = stmt_mark_tail(stmt_limit(stmt_dup(j->op1.stval),
				stmt_dup(s->op2.stval),
				stmt_dup(s->op3.stval),
				s->flag),
				j->op2.stval->op1.aval->data.val.ival);
			ns = bin_optimizer(c, s);
			stmt_destroy(s);
			assert(os->rewritten==NULL);
			os->rewritten = stmt_dup(ns);
			os->optimized = ns->optimized = 3;
			return ns;
		}
		if (s->op1.stval) {
			stmt *os = s->op1.stval;
			stmt *ns = bin_optimizer(c, os);

			assert(ns != s);
			stmt_destroy(os);
			s->op1.stval = ns;
		}
		s->optimized = 3;
		return stmt_dup(s);
	}

	case st_semijoin:{

		stmt *j = NULL;
		stmt *os, *ns;

		os = stmt_semijoin(bin_optimizer(c, s->op1.stval), bin_optimizer(c, s->op2.stval));
		/* try to push the semijoin through the (fetch) join */
		if (os->op1.stval->type == st_join) {
			j = os->op1.stval;
			/* equi join on same base table */
			if (isEqJoin(j) &&
		    		j->op1.stval->t == j->op2.stval->h ) {
				stmt *l = stmt_dup(j->op1.stval);
				stmt *r = stmt_dup(j->op2.stval);
				s = stmt_semijoin(l, stmt_dup(os->op2.stval));
				l = bin_optimizer(c, s);
				stmt_destroy(s);
				stmt_destroy(os);
				os = stmt_join( l, r, cmp_equal);
				os->optimized = 3; 
				return os;
			}
		} 
		if (!mvc_debug_on(c, 4096) && os->nrcols) {
			ns = eliminate_semijoin(os);
		} else {
			ns = stmt_dup(os);
		}
		stmt_destroy(os);
		s->optimized = ns->optimized = 3;
		if (ns != s) {
			assert(s->rewritten==NULL);
			s->rewritten = stmt_dup(ns);
		}
		return ns;
	}

	case st_join:
	case st_join2:
	case st_joinN:{

		if (s->op1.stval) {
			stmt *os = s->op1.stval;
			stmt *ns = bin_optimizer(c, os);

			assert(ns != s);
			stmt_destroy(os);
			s->op1.stval = ns;
		}
		if (s->op2.stval) {
			stmt *os = s->op2.stval;
			stmt *ns = bin_optimizer(c, os);

			assert(ns != s);
			stmt_destroy(os);
			s->op2.stval = ns;
		}
		if (s->op3.stval) {
			stmt *os = s->op3.stval;
			stmt *ns = bin_optimizer(c, os);

			assert(ns != s);
			stmt_destroy(os);
			s->op3.stval = ns;
		}

		/* remove expensive double kdiffs 
		 * if join on oids from the same table then 	
		 * right kdiff is not needed 
		 */
		if (isEqJoin(s) && 
		    s->op1.stval->t == s->op2.stval->h &&
		    s->op2.stval->type == st_diff){
			stmt *old = s->op2.stval;
			s->op2.stval = stmt_dup(old->op1.stval);
			stmt_destroy(old);
		}
		/* same as above but now with alias in between */ 
		if (isEqJoin(s) && 
		    s->op2.stval->type == st_alias &&
		    s->op1.stval->t == s->op2.stval->op1.stval->h &&
		    s->op2.stval->op1.stval->type == st_diff){
			stmt *old = s->op2.stval;
			s->op2.stval = stmt_dup(old->op1.stval->op1.stval);
			stmt_destroy(old);
		}
		s->optimized = 3;
		return stmt_dup(s);
	}

	case st_reverse:{

		stmt *os, *ns;

		os = stmt_reverse(bin_optimizer(c, s->op1.stval));
		if (!mvc_debug_on(c, 4096)) {
			ns = eliminate_reverse(os);
		} else {
			ns = stmt_dup(os);
		}
		stmt_destroy(os);
		s->optimized = ns->optimized = 3;
		if (ns != s) {
			assert(s->rewritten==NULL);
			s->rewritten = stmt_dup(ns);
		}
		return ns;
	}
	case st_select:
	case st_select2:
	case st_uselect:
	case st_uselect2: {
		stmt *res = NULL;

		/* push down the select through st_alias */
		if (s->op1.stval->type == st_alias) {
			stmt *a = s->op1.stval;
	
			s = push_select( s, stmt_dup(a->op1.stval)); 
			s = stmt_alias(s, table_name(a), column_name(a));
			res = bin_optimizer(c, s);
			stmt_destroy(s);
			return res;
		}
		/* push down the select through st_diff */
		if (s->op1.stval->type == st_diff && s->flag != cmp_notequal) {
			stmt *d = s->op1.stval;
	
			s = push_select( s, stmt_dup(d->op1.stval)); 
			s = stmt_diff(s, stmt_dup(d->op2.stval));
			res = bin_optimizer(c, s);
			stmt_destroy(s);
			return res;
		}
		/* push down the select through st_union */
		if (s->op1.stval->type == st_union && s->flag != cmp_notequal) {
			stmt *l, *r;
			stmt *u = s->op1.stval;
	
			l = push_select( s, stmt_dup(u->op1.stval)); 
			r = push_select( s, stmt_dup(u->op2.stval)); 
			s = stmt_union(l, r);
			res = bin_optimizer(c, s);
			stmt_destroy(s);
			return res;
		}

		if (s->op1.stval) {
			stmt *os = s->op1.stval;
			stmt *ns = bin_optimizer(c, os);

			assert(ns != s);
			stmt_destroy(os);
			s->op1.stval = ns;
		}
		if (s->op2.stval) {
			stmt *os = s->op2.stval;
			stmt *ns = bin_optimizer(c, os);

			assert(ns != s);
			stmt_destroy(os);
			s->op2.stval = ns;
		}
		s->optimized = 3;
		return stmt_dup(s);
	}

	case st_temp:
	case st_single:
	case st_diff:
	case st_union:
	case st_outerjoin:
	case st_mirror:
	case st_const:
	case st_mark:
	case st_gen_group:
	case st_group:
	case st_group_ext:
	case st_derive:
	case st_unique:
	case st_order:
	case st_reorder:
	case st_ordered:

	case st_alias:
	case st_column:
	case st_append:
	case st_exception:
	case st_trans:
	case st_catalog:

	case st_aggr:
	case st_unop:
	case st_binop:
	case st_Nop:
	case st_convert:

	case st_affected_rows:

	case st_table:
	case st_while:
	case st_if:
	case st_return:
	case st_assign:

	case st_output: 

		if (s->op1.stval) {
			stmt *os = s->op1.stval;
			stmt *ns = bin_optimizer(c, os);

			assert(ns != s);
			stmt_destroy(os);
			s->op1.stval = ns;
		}
	case st_append_col:
	case st_append_idx:
	case st_update_col:
	case st_update_idx:
	case st_delete:

		if (s->type != st_convert) {
			if (s->op2.stval) {
				stmt *os = s->op2.stval;
				stmt *ns = bin_optimizer(c, os);
	
				assert(ns != s);
				stmt_destroy(os);
				s->op2.stval = ns;
			}
			if (s->op3.stval) {
				stmt *os = s->op3.stval;
				stmt *ns = bin_optimizer(c, os);
	
				assert(ns != s);
				stmt_destroy(os);
				s->op3.stval = ns;
			}
		}
		s->optimized = 3;
		return stmt_dup(s);

	case st_list:{

		stmt *res = NULL;
		node *n;
		list *l = s->op1.lval;
		list *nl = NULL;

		nl = create_stmt_list();
		for (n = l->h; n; n = n->next) {
			stmt *ns = bin_optimizer(c, n->data);

			list_append(nl, ns);
		}
		res = stmt_list(nl);
		s->optimized = res->optimized = 3;
		if (res != s) {
			assert(s->rewritten==NULL);
			s->rewritten = stmt_dup(res);
		}
		return res;
	}

	case st_releqjoin:
	case st_relselect:

	default:
		assert(0);	/* these should have been rewriten by now */
	}
	return stmt_dup(s);
}
