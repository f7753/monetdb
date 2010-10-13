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

#include "rel_bin.h"
#include "rel_exp.h"
#include "rel_prop.h"
/* needed for recursion (ie triggers) */
#include "rel_select.h"
#include "rel_subquery.h"

static stmt * subrel_bin(mvc *sql, sql_rel *rel, list *refs);

static void 
print_stmtlist( stmt *l )
{
	node *n;
	if (l)
	for (n = l->op1.lval->h; n; n = n->next) {
		char *rnme = table_name(n->data);
		char *nme = column_name(n->data);

		printf("%s.%s\n", rnme, nme);
		_DELETE(rnme);
		_DELETE(nme);
	}
}

static stmt *
list_find_column( list *l, char *rname, char *name ) 
{
	stmt *res = NULL;
	node *n;

	if (rname) {
		for (n = l->h; n; n = n->next) {
			char *rnme = table_name(n->data);
			char *nme = column_name(n->data);

			if (rnme && strcmp(rnme, rname) == 0 && 
				    strcmp(nme, name) == 0) {
				res = n->data;
				_DELETE(rnme);
				_DELETE(nme);
				break;
			}
			if (rnme)
				_DELETE(rnme);
			if (nme)
				_DELETE(nme);
		}
	} else {
		for (n = l->h; n; n = n->next) {
			char *nme = column_name(n->data);

			if (nme && strcmp(nme, name) == 0) {
				res = n->data;
				_DELETE(nme);
				break;
			}
			if (nme)
				_DELETE(nme);
		}
	}
	if (!res)
		return NULL;
	return stmt_dup(res);
}

static stmt *
bin_find_column( stmt *sub, char *rname, char *name ) 
{
	return list_find_column( sub->op1.lval, rname, name);
}

static list *
bin_find_columns( stmt *sub, char *name ) 
{
	node *n;
	list *l = create_stmt_list();

	for (n = sub->op1.lval->h; n; n = n->next) {
		char *nme = column_name(n->data);

		if (strcmp(nme, name) == 0) 
			append(l, stmt_dup(n->data));
		_DELETE(nme);
	}
	if (list_length(l)) 
		return l;
	list_destroy(l);
	return NULL;
}

static stmt *column( stmt *val )
{
	val = stmt_dup(val);
	if (val->nrcols == 0)
		return const_column(val);
	return val;
}

static stmt *Column( stmt *val )
{
	val = stmt_dup(val);
	if (val->nrcols == 0)
		val = const_column(val);
	return stmt_append(stmt_temp(tail_type(val)), val);
}

static stmt *
bin_first_column( stmt *sub ) 
{
	node *n = sub->op1.lval->h;
	stmt *c = stmt_dup(n->data);

	if (c->nrcols == 0)
		return const_column(c);
	return c;
}

static stmt *
row2cols( stmt *sub)
{
	if (sub->nrcols == 0 && sub->key) {
		node *n; 
		list *l = create_stmt_list();

		for (n = sub->op1.lval->h; n; n = n->next) {
			stmt *sc = stmt_dup(n->data);
			char *cname = column_name(sc);
			char *tname = table_name(sc);

			sc = column(sc);
			list_append(l, stmt_alias(sc, tname, cname));
		}
		stmt_destroy(sub);
		sub = stmt_list(l);
	}
	return sub;
}

/* Here we also recognize 'IN'.
 * We change that into a 
 * mark(reverse(semijoin( reverse(column), bat_of_vals)), 0). 
 */
static int
are_equality_exps( list *exps ) 
{
	if (list_length(exps) == 1) {
		sql_exp *e = exps->h->data, *le = e->l, *re = e->r;

		if (e->type == e_cmp && e->flag == cmp_equal && le->card != CARD_ATOM && re->card == CARD_ATOM)
			return 1;
		if (e->type == e_cmp && e->flag == cmp_or)
			return (are_equality_exps(e->l) && 
				are_equality_exps(e->r));
	}
	return 0;
}

static void 
get_exps( list *n, list *l )
{
	sql_exp *e = l->h->data, *re = e->r;

	if (e->type == e_cmp && e->flag == cmp_equal && re->card == CARD_ATOM)
		list_append(n, exp_dup(e));
	if (e->type == e_cmp && e->flag == cmp_or) {
		get_exps(n, e->l);
		get_exps(n, e->r);
	}
}

/* For now this only works if all or's are part of the 'IN' */
static stmt *
handle_equality_exps( mvc *sql, list *l, list *r, stmt *left, stmt *right, group *grp )
{
	node *n;
	sql_exp *ce = NULL;
	list *nl = new_exp_list();
	stmt *s = NULL, *c;

	get_exps(nl, l);
	get_exps(nl, r);

	for( n = nl->h; n; n = n->next) {
		sql_exp *e = n->data;
		if (!ce) {
			ce = e->l;
			if (!is_column(ce->type))
				return NULL;
		}
		if (!exp_match(ce, e->l)) 
			return NULL;
	} 

	/* create bat append values */
	s = stmt_temp(exp_subtype(ce));
	for( n = nl->h; n; n = n->next) {
		sql_exp *e = n->data;
		stmt *i = exp_bin(sql, e->r, left, right, grp, NULL);
		
		s = stmt_append(s, i);
	}
	c = exp_bin(sql, ce, left, right, grp, NULL);
	/*s = stmt_mark_tail(stmt_reverse(stmt_semijoin(stmt_reverse(c), stmt_reverse(s))), 0);*/
	/* not really a projection join, therefore make sure left values are unique !! */
	s = stmt_project(c, stmt_reverse(stmt_unique(s, NULL)));
	s = stmt_const(s, NULL);

	list_destroy(nl);
	return s;
}

stmt *
exp_bin(mvc *sql, sql_exp *e, stmt *left, stmt *right, group *grp, stmt *sel) 
{
	stmt *s = NULL;

	if (!e) {
		assert(0);
		return NULL;
	}

	switch(e->type) {
	case e_atom: {
		if (e->l) { 			/* literals */
			atom *a = e->l;
			s = stmt_atom(atom_dup(a));
		} else if (e->r) { 		/* parameters */
			s = stmt_var(_strdup(e->r), e->tpe.type?&e->tpe:NULL, 0, e->flag);
		} else { 			/* arguments */
			s = stmt_varnr(e->flag, e->tpe.type?&e->tpe:NULL);
		}
	}	break;
	case e_convert: {
		stmt *l = exp_bin(sql, e->l, left, right, grp, sel);
		list *tps = e->r;
		sql_subtype *from = tps->h->data;
		sql_subtype *to = tps->h->next->data;
		if (!l) 
			return NULL;
		s = stmt_convert(l, from, to);
	} 	break;
	case e_func: {
		node *en;
		list *l = create_stmt_list(), *exps = e->l, *obe = e->r;
		sql_subfunc *f = e->f;

		if (!obe && exps) {
			for (en = exps->h; en; en = en->next) {
				stmt *es;

				es = exp_bin(sql, en->data, left, right, grp, sel);
				if (!es) {
					list_destroy(l);
					return NULL;
				}
				list_append(l,es);
			}
		}
		/* Window expressions are handled differently.
		   ->l == group by expression list
		   ->r == order by expression list
		   If both lists are empty, we pass a single 
		 	column for the inner relation
		 */
		if (obe) {
			group *g = NULL;
			stmt *orderby = NULL;
		
			if (exps) {
				for (en = exps->h; en; en = en->next) {
					stmt *es;

					es = exp_bin(sql, en->data, left, right, NULL, sel);
					if (!es) {
						list_destroy(l);
						return NULL;
					}
					g = grp_create(es, g);
				}
			}
			/* order on the group first */
			grp_done(g);
			if (g) 
				orderby = stmt_order(stmt_dup(g->grp), 1);
			for (en = obe->h; en; en = en->next) {
				sql_exp *orderbycole = en->data; 
				stmt *orderbycols = exp_bin(sql, orderbycole, left, right, NULL, sel); 

				if (!orderbycols) {
					list_destroy(l);
					return NULL;
				}
				if (orderby)
					orderby = stmt_reorder(orderby, orderbycols, is_ascending(orderbycole));
				else
					orderby = stmt_order(orderbycols, is_ascending(orderbycole));
			}
			if (!orderby && left)
				orderby = stmt_mirror(bin_first_column(left));
			if (!orderby) {
				list_destroy(l);
				grp_destroy(g);
				return NULL;
			}
			list_append(l, orderby);
			if (g) {
				list_append(l, stmt_dup(g->grp));
				list_append(l, stmt_dup(g->ext));
				grp_destroy(g);
			}
		}
		if (strcmp(f->func->base.name, "identity") == 0) {
			s = stmt_mirror(stmt_dup(l->h->data));
			list_destroy(l);
		} else
			s = stmt_Nop(stmt_list(l), sql_dup_func(e->f)); 
	} 	break;
	case e_aggr: {
		sql_exp *at = NULL;
		list *attr = e->l; 
		stmt *as = NULL;
		stmt *as2 = NULL;
		sql_subaggr *a = e->f;
		group *g = grp;

		assert(sel == NULL);
		if (attr) { 
			at = attr->h->data;
			as = exp_bin(sql, at, left, right, NULL, sel);
			if (list_length(attr) == 2)
				as2 = exp_bin(sql, attr->h->next->data, left, right, NULL, sel);
			/* insert single value into a column */
			if (as && as->nrcols <= 0 && !left)
				as = const_column(as);
		} else {
			/* count(*) may need the default group (relation) and
			   and/or an attribute to count */
			if (g) {
				as = stmt_dup(grp->grp);
			} else if (left) {
				as = bin_first_column(left);
			} else {
				/* create dummy single value in a column */
				as = stmt_atom_wrd(0);
				as = const_column(as);
			}
		}
		if (!as) 
			return NULL;	

		if (as->nrcols <= 0 && left) 
			as = stmt_const(bin_first_column(left), as);
		/* inconsistent sql requires NULL != NULL, ie unknown
		 * but also NULL means no values, which means 'ignore'
		 *
		 * so here we need to ignore NULLs
		 */
		if (need_no_nil(e) && at && has_nil(at) && attr) {
			sql_subtype *t = exp_subtype(at);
			stmt *n = stmt_atom(atom_general(t, NULL, 0));
			as = stmt_select2(as, n, stmt_dup(n), 0);
		}
		if (need_distinct(e)){ 
			if (g)
				as = stmt_unique(as, grp_dup(grp));
			else
				as = stmt_unique(as, NULL);
		}
		if (g) 
			g = grp_dup(g);
		if (as2) 
			s = stmt_aggr2(stmt_reverse(as), as2, sql_dup_aggr(a) );
		else
			s = stmt_aggr(as, g, sql_dup_aggr(a), 1 );
		/* HACK: correct cardinality for window functions */
		if (e->card > CARD_AGGR)
			s->nrcols = 2;
	} 	break;
	case e_column: {
		if (right) /* check relation names */
			s = bin_find_column(right, e->l, e->r);
		if (!s && left) {
			if (left->type == st_Nop && tail_type(left)->comp_type ) { /* table function */
				s = stmt_rs_column(stmt_dup(left), stmt_atom_string(_strdup(e->r)), &e->tpe);
			} else {
				s = bin_find_column(left, e->l, e->r);
			}
		}
		if (s && grp)
			s = stmt_join(stmt_dup(grp->ext), s, cmp_equal);
		if (!s && right) {
			printf("could not find %s.%s\n", (char*)e->l, (char*)e->r);
			print_stmtlist(left);
			print_stmtlist(right);
		}
		if (s && sel)
			s = stmt_semijoin(s, stmt_dup(sel));
	 }	break;
	case e_cmp: {
		stmt *l = NULL, *r = NULL, *r2 = NULL;
		int swapped = 0, is_select = 0;
		sql_exp *re = e->r, *re2 = e->f;
		prop *p;

		if (e->flag == cmp_or) {
			list *l = e->l;
			node *n;
			stmt *sel1, *sel2;

			/* Here we also recognize 'IN'.
			 * We change that into a 
			 * reverse(semijoin( reverse(column), bat_of_vals)). 
			 */
			if (are_equality_exps(e->l) && are_equality_exps(e->r))
				if ((s = handle_equality_exps(sql, e->l, e->r, left, right, grp)) != NULL)
					return s;
			sel1 = stmt_relselect_init();
			sel2 = stmt_relselect_init();
			for( n = l->h; n; n = n->next ) {
				s = exp_bin(sql, n->data, left, right, grp, sel); 
				if (!s) {
					stmt_destroy(sel1);
					stmt_destroy(sel2);
					return s;
				}
				stmt_relselect_fill(sel1, s);
			}
			l = e->r;
			for( n = l->h; n; n = n->next ) {
				s = exp_bin(sql, n->data, left, right, grp, sel); 
				if (!s) {
					stmt_destroy(sel1);
					stmt_destroy(sel2);
					return s;
				}
				stmt_relselect_fill(sel2, s);
			}
			if (sel1->nrcols == 0 && sel2->nrcols == 0) {
				sql_subtype *bt = sql_bind_localtype("bit");
				sql_subfunc *f = sql_bind_func(sql->session->schema, "or", bt, bt);
				assert(f);
				return stmt_binop(sel1, sel2, f);
			}
			return stmt_union(sel1,sel2);
		}
		/* here we handle join indices */
		if ((p=find_prop(e->p, PROP_JOINIDX)) != NULL) {
			sql_idx *i = p->value;
			sql_exp *el = e->l;
			sql_exp *er = e->r;

			/* find out left and right */
			l = bin_find_column(left, el->l, i->base.name);
			if (!l) {
				swapped = 1;
				l = bin_find_column(right, el->l, i->base.name);
				r = bin_find_column(left, er->l, "%TID%");
			} else {
				r = bin_find_column(right, er->l, "%TID%");
			}
			/* small performance improvement, ie use idx directly */
			if (l->type == st_alias && 
			    l->op1.stval->type == st_idxbat &&
			    r->type == st_alias && 
			    r->op1.stval->type == st_mirror) {
				stmt_destroy(r);
				s = l;
			} else if (swapped)
				s = stmt_join(r, stmt_reverse(l), cmp_equal);
			else
				s = stmt_join(l, stmt_reverse(r), cmp_equal);
			sql->opt_stats[0]++; 
			assert(sel==NULL);
			break;
		}
		if (!l) {
			l = exp_bin(sql, e->l, left, NULL, grp, sel);
			swapped = 0;
		}
		if (!l && right) {
 			l = exp_bin(sql, e->l, right, NULL, grp, sel);
			swapped = 1;
		}
		if (swapped || !right)
 			r = exp_bin(sql, e->r, left, NULL, grp, sel);
		else
 			r = exp_bin(sql, e->r, right, NULL, grp, sel);
		if (!r && !swapped) {
 			r = exp_bin(sql, e->r, left, NULL, grp, sel);
			is_select = 1;
		}
		if (!r && swapped) {
 			r = exp_bin(sql, e->r, right, NULL, grp, sel);
			is_select = 1;
		}
		if (re2)
 			r2 = exp_bin(sql, e->f, left, right, grp, sel);
		if (!l || !r || (re2 && !r2)) {
			assert(0);
			if (l) stmt_destroy(l);
			if (r) stmt_destroy(r);
			if (r2) stmt_destroy(r2);
			return NULL;
		}

		/* the escape character of like is in the right expression */
		if (e->flag == cmp_notlike || e->flag == cmp_like ||
		    e->flag == cmp_notilike || e->flag == cmp_ilike)
		{
			if (!e->f)
				r2 = stmt_atom_string(_strdup(""));
			if (!l || !r || !r2) {
				assert(0);
				if (l) stmt_destroy(l);
				if (r) stmt_destroy(r);
				if (r2) stmt_destroy(r2);
				return NULL;
			}
			if (l->nrcols == 0) {
				stmt *lstmt;
				char *likef = (e->flag == cmp_notilike || e->flag == cmp_ilike ?
					"ilike" : "like");
				sql_subtype *s = sql_bind_localtype("str");
				sql_subfunc *like = sql_bind_func3(sql->session->schema, likef, s, s, s);
				list *ops = create_stmt_list();

				assert(s && like);
				
				list_append(ops, l);
				list_append(ops, r);
				list_append(ops, r2);
				lstmt = stmt_Nop(stmt_list(ops), like);
				if (e->flag == cmp_notlike || e->flag == cmp_notilike) {
					sql_subtype *bt = sql_bind_localtype("bit");
					sql_subfunc *not = sql_bind_func(sql->session->schema,
							"not", bt, NULL);
					lstmt = stmt_unop(lstmt, not);
				}
				return lstmt;
			}
                        if (left && right && re->card > CARD_ATOM && !is_select) {
                                /* create l and r, gen operator func */
                                char *like = (e->flag == cmp_like || e->flag == cmp_notlike)?"like":"ilike";
                                int anti = (e->flag == cmp_notlike || e->flag == cmp_notilike);
                                sql_subtype *s = sql_bind_localtype("str");
                                sql_subfunc *f = sql_bind_func3(sql->session->schema, like, s, s, s);

                                stmt *j = stmt_joinN(stmt_list(append(create_stmt_list(),l)), stmt_list(append(append(create_stmt_list(),r),r2)), f);
                                if (is_anti(e) || anti)
                                        j->flag |= ANTI;
                                return j;
                        }
			return stmt_likeselect(l, r, r2, (comp_type)e->flag);
		}
		if (left && right && !is_select &&
		    (re->card > CARD_ATOM || (l->nrcols && r->nrcols))) {
			if (l->nrcols == 0)
				l = stmt_const(bin_first_column(swapped?right:left), l); 
			if (r->nrcols == 0)
				r = stmt_const(bin_first_column(swapped?left:right), r); 
			if (r2) {
				s = stmt_join2(l, r, r2, (comp_type)e->flag);
				if (swapped) 
					s = stmt_reverse(s);
			} else if (swapped) {
				s = stmt_join(r, stmt_reverse(l), swap_compare((comp_type)e->flag));
			} else {
				s = stmt_join(l, stmt_reverse(r), (comp_type)e->flag);
			}
		} else {
			if (r2) {
				if (l->nrcols == 0 && r->nrcols == 0 && r2->nrcols == 0) {
					sql_subtype *bt = sql_bind_localtype("bit");
					sql_subfunc *lf = sql_bind_func(sql->session->schema,
							compare_func(range2lcompare(e->flag)),
							tail_type(l), tail_type(r));
					sql_subfunc *rf = sql_bind_func(sql->session->schema,
							compare_func(range2rcompare(e->flag)),
							tail_type(l), tail_type(r));
					sql_subfunc *a = sql_bind_func(sql->session->schema,
							"and", bt, bt);
					assert(lf && rf && a);
					l = stmt_dup(l);
					s = stmt_binop(
						stmt_binop(l, r, lf), 
						stmt_binop(l, r2, rf), a);
				} else if (l->nrcols > 0 && r->nrcols > 0 && r2->nrcols > 0) {
					l = stmt_dup(l);
					s = stmt_semijoin(
						stmt_uselect(l, r, range2lcompare(e->flag)),
						stmt_uselect(l, r2, range2rcompare(e->flag)));
				} else {
					s = stmt_uselect2(l, r, r2, (comp_type)e->flag);
				}
			} else {
				/* value compare or select */
				if (l->nrcols == 0 && r->nrcols == 0) {
					sql_subfunc *f = sql_bind_func(sql->session->schema,
							compare_func((comp_type)e->flag),
							tail_type(l), tail_type(r));
					assert(f);
					s = stmt_binop(l, r, f);
				} else {
					/* this can still be a join (as relationa algebra and single value subquery results still means joins */
					s = stmt_uselect(l, r, (comp_type)e->flag);
					/* so we still need the proper side */
					if (swapped)
						s = stmt_reverse(s);
				}
			}
		}
		if (is_anti(e))
			s->flag |= ANTI;
	 }	break;
	default:
		;
	}
	return s;
}


static stmt *
stmt_rename( sql_rel *rel, sql_exp *exp, stmt *s )
{
	char *name = exp->name;
	char *rname = exp->rname;

	(void)rel;
	if (!name)
		name = column_name(s);
	else
		name = _strdup(name);
	if (!rname)
		rname = table_name(s);
	else
		rname = _strdup(rname);
	s = stmt_alias(s, rname, name);
	return s;
}

static stmt *
rel2bin_sql_table(sql_table *t) 
{
	list *l; 
	node *n;
	stmt *ts;
	char *tname = t->base.name;
			
	l = create_stmt_list();
	ts = stmt_basetable(t, t->base.name);
	for (n = t->columns.set->h; n; n = n->next) {
		sql_column *c = n->data;

		stmt *sc = stmt_bat(c, stmt_dup(ts), RDONLY);
		list_append(l, sc);
	}
	/* %TID% column */
	if (t->columns.set->h) { 
		sql_column *c = t->columns.set->h->data;
		stmt *sc = stmt_bat(c, stmt_dup(ts), RDONLY);
		char *rnme = _strdup(t->base.name);

		sc = stmt_mirror(sc);
		sc = stmt_alias(sc, rnme, _strdup("%TID%"));
		list_append(l, sc);
	}
	stmt_destroy(ts);

	if (t->idxs.set) {
		for (n = t->idxs.set->h; n; n = n->next) {
			sql_idx *i = n->data;
			stmt *sc = stmt_idxbat(i, RDONLY);
			char *rnme = _strdup(tname);

			sc = stmt_alias(sc, rnme, _strdup(i->base.name));
			list_append(l, sc);
		}
	}
	return stmt_list(l);
}

static stmt *
rel2bin_basetable( mvc *sql, sql_rel *rel, list *refs)
{
	list *l; 
	stmt *ts, *sub = NULL;
	sql_table *t = rel->l;
	node *n;
	char *tname = t->base.name;
			
	(void)sql;
	(void)refs;
	if (rel->exps) {
		sql_exp *e = rel->exps->h->data;

		if (e->rname)
			tname = e->rname;
	}
	l = create_stmt_list();
	ts = stmt_basetable(t, t->base.name);
	assert(rel->exps);
	for (n = t->columns.set->h; n; n = n->next) {
		sql_column *c = n->data;

		stmt *sc = stmt_bat(c, stmt_dup(ts), RDONLY);
		list_append(l, sc);
	}
	/* %TID% column */
	if (t->columns.set->h) { 
		sql_column *c = t->columns.set->h->data;
		stmt *sc = stmt_bat(c, stmt_dup(ts), RDONLY);
		char *rnme = _strdup(t->base.name);

		sc = stmt_mirror(sc);
		sc = stmt_alias(sc, rnme, _strdup("%TID%"));
		list_append(l, sc);
	}
	stmt_destroy(ts);

	sub = stmt_list(l);
	/* add aliases */
	if (rel->exps) {
		node *en;

		l = create_stmt_list();
		for( en = rel->exps->h; en; en = en->next ) {
			sql_exp *exp = en->data;
			stmt *s = bin_find_column(sub, exp->l, exp->r);
			char *rname = exp->rname?exp->rname:exp->l;
	
			if (!s) {
				assert(0);
				stmt_destroy(sub);
				return NULL;
			}
			rname = rname?_strdup(rname):NULL;
			s = stmt_alias(s, rname, _strdup(exp->name));
			list_append(l, s);
		}
		stmt_destroy(sub);
		sub = stmt_list(l);
	}
	if (t->idxs.set) {
		for (n = t->idxs.set->h; n; n = n->next) {
			sql_idx *i = n->data;
			stmt *sc = stmt_idxbat(i, RDONLY);
			char *rnme = _strdup(tname);

			sc = stmt_alias(sc, rnme, _strdup(i->base.name));
			list_append(l, sc);
		}
	}
	return sub;
}

static stmt *
rel2bin_table( mvc *sql, sql_rel *rel, list *refs)
{
	list *l; 
	stmt *sub = NULL;
	node *en;
			
	(void)refs;
	sub = exp_bin(sql, rel->l, NULL, NULL, NULL, NULL); /* table function */
	if (!sub) { 
		assert(0);
		return NULL;	
	}
	l = create_stmt_list();
	for( en = rel->exps->h; en; en = en->next ) {
		sql_exp *exp = en->data;
		stmt *s = exp_bin(sql, exp, sub, NULL, NULL, NULL);
		char *rnme = exp->rname?exp->rname:exp->l;

		if (!s) {
			assert(0);
			list_destroy(l);
			if (sub) 
				stmt_destroy(sub);
			return NULL;
		}
		if (sub && sub->nrcols >= 1 && s->nrcols == 0)
			s = stmt_const(bin_first_column(sub), s);
		rnme = (rnme)?_strdup(rnme):NULL;
		s = stmt_alias(s, rnme, _strdup(exp->name));
		list_append(l, s);
	}
	if (sub) 
		stmt_destroy(sub);
	sub = stmt_list(l);
	return sub;
}

static int
equi_join(stmt *j)
{
	if (j->flag == cmp_equal)
		return 0;
	return -1;
}

static int
not_equi_join(stmt *j)
{
	if (j->flag != cmp_equal)
		return 0;
	return -1;
}

static stmt *
rel2bin_join( mvc *sql, sql_rel *rel, list *refs)
{
	list *l; 
	node *en, *n;
	stmt *left = NULL, *right = NULL, *join = NULL, *jl, *jr;
	stmt *ld = NULL, *rd = NULL;

	if (rel->l) /* first construct the left sub relation */
		left = subrel_bin(sql, rel->l, refs);
	if (rel->r) /* first construct the right sub relation */
		right = subrel_bin(sql, rel->r, refs);
	if (!left || !right) { 
		if (left) stmt_destroy(left);
		if (right) stmt_destroy(right);
		return NULL;	
	}
	left = row2cols(left);
	right = row2cols(right);
	if (rel->exps) {
		int idx = 0;
		list *jns = create_stmt_list();

		/* generate a stmt_reljoin */
		for( en = rel->exps->h; en; en = en->next ) {
			int join_idx = sql->opt_stats[0];
			stmt *s = exp_bin(sql, en->data, left, right, NULL, NULL);

			if (!s) {
				assert(0);
				stmt_destroy(left);
				stmt_destroy(right);
				list_destroy(jns);
				return NULL;
			}
			if (join_idx != sql->opt_stats[0])
				idx = 1;
			if (!join) {
				join = s;
			} else if (s->type != st_join && s->type != st_join2) {
				/* handle select expressions */
				/*assert(0);*/
				if (s->h == join->h) {
					join = stmt_semijoin(join,s);
				} else {
					join = stmt_reverse(join);
					join = stmt_semijoin(join,s);
					join = stmt_reverse(join);
				}
				continue;
			}
			list_append(jns, s);
		}
		if (list_length(jns) > 1) {
			int o = 1, *O = &o;
			/* move all equi joins into a releqjoin */
			list *eqjns = list_select(jns, O, (fcmp)&equi_join, (fdup)stmt_dup);
			if (!idx && list_length(eqjns) > 1) {
				list *neqjns = list_select(jns, O, (fcmp)&not_equi_join, (fdup)stmt_dup);
				list_destroy(jns);
				join = stmt_reljoin(stmt_releqjoin1(eqjns), neqjns);
			} else {
				join = stmt_reljoin(NULL, jns);
			}
			list_destroy(eqjns);
		} else {
			join = stmt_dup(jns->h->data); 
			list_destroy(jns);
		}
	} else {
		stmt *l = bin_first_column(left);
		stmt *r = bin_first_column(right);
		join = stmt_join(l, stmt_reverse(r), cmp_all); 
	}

	/* construct relation */
	l = create_stmt_list();

	jl = stmt_reverse(stmt_mark_tail(stmt_dup(join),0));
	if (rel->op == op_left || rel->op == op_full) {
		/* we need to add the missing oid's */
		ld = stmt_diff(bin_first_column(left), stmt_reverse(stmt_dup(jl)));
		ld = stmt_mark(stmt_reverse(ld), 0);
	}
	jr = stmt_reverse(stmt_mark_tail(stmt_reverse(join),0));
	if (rel->op == op_right || rel->op == op_full) {
		/* we need to add the missing oid's */
		rd = stmt_diff(bin_first_column(right), stmt_reverse(stmt_dup(jr)));
		rd = stmt_mark(stmt_reverse(rd), 0);
	}

	for( n = left->op1.lval->h; n; n = n->next ) {
		stmt *c = n->data;
		char *rnme = table_name(c);
		char *nme = column_name(c);
		stmt *s = stmt_project(stmt_dup(jl), column(c) );

		/* as append isn't save, we append to a new copy */
		if (rel->op == op_left || rel->op == op_full || rel->op == op_right)
			s = Column(s);
		if (rel->op == op_left || rel->op == op_full)
			s = stmt_append(s, stmt_join(stmt_dup(ld), stmt_dup(c), cmp_equal));
		if (rel->op == op_right || rel->op == op_full) 
			s = stmt_append(s, stmt_const(stmt_dup(rd), stmt_atom(atom_general(tail_type(c), NULL, 0))));

		s = stmt_alias(s, rnme, nme);
		list_append(l, s);
	}
	stmt_destroy(jl);
	stmt_destroy(left);
	for( n = right->op1.lval->h; n; n = n->next ) {
		stmt *c = n->data;
		char *rnme = table_name(c);
		char *nme = column_name(c);
		stmt *s = stmt_project(stmt_dup(jr), column(c) );

		/* as append isn't save, we append to a new copy */
		if (rel->op == op_left || rel->op == op_full || rel->op == op_right)
			s = Column(s);
		if (rel->op == op_left || rel->op == op_full) 
			s = stmt_append(s, stmt_const(stmt_dup(ld), stmt_atom(atom_general(tail_type(c), NULL, 0))));
		if (rel->op == op_right || rel->op == op_full) 
			s = stmt_append(s, stmt_join(stmt_dup(rd), stmt_dup(c), cmp_equal));

		s = stmt_alias(s, rnme, nme);
		list_append(l, s);
	}
	stmt_destroy(jr);
	stmt_destroy(right);
	if (ld)
		stmt_destroy(ld);
	if (rd)
		stmt_destroy(rd);
	return stmt_list(l);
}

static stmt *
rel2bin_semijoin( mvc *sql, sql_rel *rel, list *refs)
{
	list *l; 
	node *en, *n;
	stmt *left = NULL, *right = NULL, *join = NULL;

	if (rel->l) /* first construct the left sub relation */
		left = subrel_bin(sql, rel->l, refs);
	if (rel->r) /* first construct the right sub relation */
		right = subrel_bin(sql, rel->r, refs);
	if (!left || !right) { 
		if (left) stmt_destroy(left);
		if (right) stmt_destroy(right);
		return NULL;	
	}
	left = row2cols(left);
	if (rel->exps) {
		for( en = rel->exps->h; en; en = en->next ) {
			stmt *s = exp_bin(sql, en->data, left, right, NULL, NULL);

			if (!s) {
				assert(0);
				stmt_destroy(left);
				stmt_destroy(right);
				if (join)
					stmt_destroy(join);
				return NULL;
			}
			if (!join) {
				join = s;
			} else {
				/* break column join */
				stmt *l = stmt_mark(stmt_reverse(join), 100);
				stmt *r = stmt_mark(stmt_dup(join), 100);
				stmt *ld = stmt_dup(s->op1.stval);
				stmt *rd = stmt_reverse(stmt_dup(s->op2.stval));
				stmt *le = stmt_join(l, ld, cmp_equal);
				stmt *re = stmt_join(r, rd, cmp_equal);

				sql_subfunc *f = sql_bind_func(sql->session->schema, compare_func((comp_type)s->flag), tail_type(le), tail_type(le));
				stmt * cmp;

				assert(f);

				cmp = stmt_binop(le, re, f);

				cmp = stmt_uselect(cmp, stmt_bool(1), cmp_equal);

				l = stmt_semijoin(stmt_dup(l), stmt_dup(cmp));
				r = stmt_semijoin(stmt_dup(r), cmp);
				join = stmt_join(stmt_reverse(l), r, cmp_equal);
				stmt_destroy(s);
			}
		}
	} else {
		/* TODO: this case could use some optimization */
		stmt *l = bin_first_column(left);
		stmt *r = bin_first_column(right);
		join = stmt_join(l, stmt_reverse(r), cmp_all); 
	}

	/* construct relation */
	l = create_stmt_list();

	/* We did a full join, thats too much. 
	   Reduce this using difference and semijoin */
	if (rel->op == op_anti) {
		stmt *c = left->op1.lval->h->data;
		join = stmt_diff(stmt_dup(c), join);
	} else {
		stmt *c = left->op1.lval->h->data;
		join = stmt_semijoin(stmt_dup(c), join);
	}

	join = stmt_reverse(stmt_mark_tail(join,0));

	/* semijoin all the left columns */
	for( n = left->op1.lval->h; n; n = n->next ) {
		stmt *c = n->data;
		char *rnme = table_name(c);
		char *nme = column_name(c);
		stmt *s = stmt_project(stmt_dup(join), column(c));

		s = stmt_alias(s, rnme, nme);
		list_append(l, s);
	}
	stmt_destroy(join);
	stmt_destroy(left);
	stmt_destroy(right);
	return stmt_list(l);
}

static stmt *
rel2bin_distinct(stmt *s)
{
	node *n;
	group *grp = NULL;
	list *rl = create_stmt_list(), *tids;

	/* single values are unique */
	if (s->key && s->nrcols == 0)
		return s;

	/* Use 'all' %TID% columns */
	if ((tids = bin_find_columns(s, "%TID%")) != NULL) {
		for (n = tids->h; n; n = n->next) {
			stmt *t = n->data;

			grp = grp_create(column(t), grp);
		}
		list_destroy(tids);
	} else {
		for (n = s->op1.lval->h; n; n = n->next) {
			stmt *t = n->data;

			grp = grp_create(column(t), grp);
		}
	}
	grp_done(grp);

	for (n = s->op1.lval->h; n; n = n->next) {
		stmt *t = n->data;

		list_append(rl, stmt_project(stmt_dup(grp->ext), stmt_dup(t)));
	}

	if (grp)
		grp_destroy(grp);
	stmt_destroy(s);
	s = stmt_list(rl);
	return s;
}

static stmt *
rel2bin_union( mvc *sql, sql_rel *rel, list *refs)
{
	list *l; 
	node *n, *m;
	stmt *left = NULL, *right = NULL, *sub;

	if (rel->l) /* first construct the left sub relation */
		left = subrel_bin(sql, rel->l, refs);
	if (rel->r) /* first construct the right sub relation */
		right = subrel_bin(sql, rel->r, refs);
	if (!left || !right) { 
		if (left) stmt_destroy(left);
		if (right) stmt_destroy(right);
		return NULL;	
	}

	/* construct relation */
	l = create_stmt_list();
	for( n = left->op1.lval->h, m = right->op1.lval->h; n && m; 
		n = n->next, m = m->next ) {
		stmt *c1 = n->data;
		stmt *c2 = stmt_dup(m->data);
		char *rnme = table_name(c1);
		char *nme = column_name(c1);
		stmt *s;

		/* append isn't save, ie use union 
			(also not save loses unique head oids) 

		   so we create append on copies.
		*/
		s = stmt_append(Column(c1), c2);
		s = stmt_alias(s, rnme, nme);
		list_append(l, s);
	}
	stmt_destroy(left);
	stmt_destroy(right);
	sub = stmt_list(l);

	/* union exp list is a rename only */
	if (rel->exps) {
		node *en, *n;
		list *l = create_stmt_list();

		for( en = rel->exps->h, n = sub->op1.lval->h; en && n; en = en->next, n = n->next ) {
			sql_exp *exp = en->data;
			stmt *s = stmt_dup(n->data);
			//stmt *s = exp_bin(sql, exp, sub, NULL, NULL, NULL);

			if (!s) {
				assert(0);
				cond_stmt_destroy(sub);
				list_destroy(l);
				return NULL;
			}
			s = stmt_rename(rel, exp, s);
			list_append(l, s);
		}
		stmt_destroy(sub);
		sub = stmt_list(l);
	}

	if (need_distinct(rel)) 
		sub = rel2bin_distinct(sub);
	return sub;
}

static stmt *
rel2bin_except( mvc *sql, sql_rel *rel, list *refs)
{
	list *stmts; 
	node *n, *m;
	stmt *left = NULL, *right = NULL, *sub;

	group *lgrp = NULL, *rgrp = NULL;
	stmt *s, *lm, *ls = NULL, *rs = NULL, *ld = NULL;
	sql_subaggr *a;

	if (rel->l) /* first construct the left sub relation */
		left = subrel_bin(sql, rel->l, refs);
	if (rel->r) /* first construct the right sub relation */
		right = subrel_bin(sql, rel->r, refs);
	if (!left || !right) { 
		if (left) stmt_destroy(left);
		if (right) stmt_destroy(right);
		return NULL;	
	}
	left = row2cols(left);

	/* construct relation */
	stmts = create_stmt_list();
	/*
	 * The multi column intersect is handled using group by's and
	 * group size counts on both sides of the intersect. We then
	 * return for each group of A with min(A.count,B.count), 
	 * number of rows.
	 * 
	 * The problem with this approach is that the groups should
	 * have equal group identifiers. So we take the union of all
	 * columns before the group by.
	 */
	for (n = left->op1.lval->h; n; n = n->next) 
		lgrp = grp_create(column(n->data), lgrp);
	for (n = right->op1.lval->h; n; n = n->next) 
		rgrp = grp_create(column(n->data), rgrp);

	if (!lgrp || !rgrp) {
		grp_destroy(lgrp);
		grp_destroy(rgrp);
		return NULL;
	}
	grp_done(lgrp);
	grp_done(rgrp);

 	a = sql_bind_aggr(sql->session->schema, "count", NULL);
	ls = stmt_aggr(stmt_dup(lgrp->grp), grp_dup(lgrp), a, 1); 
	a = sql_dup_aggr(a);
	rs = stmt_aggr(stmt_dup(rgrp->grp), grp_dup(rgrp), a, 1); 

	/* now find the matching groups */
	s = stmt_releqjoin_init();
	for (n = left->op1.lval->h, m = right->op1.lval->h; n && m; n = n->next, m = m->next) {
		stmt *l = column(n->data);
		stmt *r = column(m->data);

		l = stmt_join(stmt_dup(lgrp->ext), l, cmp_equal);
		r = stmt_join(stmt_dup(rgrp->ext), r, cmp_equal);
		stmt_releqjoin_fill(s, l, r);
	}

	/* the join of the groups removed those in A but not in B,
	 * we need these later so keep these in 'ld' */
	ld = stmt_diff(stmt_dup(ls), stmt_dup(s));
		
	/*if (!distinct) */
	{
		sql_subfunc *sub;

		lm = stmt_reverse(stmt_mark_tail(stmt_dup(s),0));
		ls = stmt_join(stmt_dup(lm),ls,cmp_equal);
		rs = stmt_join(stmt_mark(s,0),rs,cmp_equal);

 		sub = sql_bind_func(sql->session->schema, "sql_sub", tail_type(ls), tail_type(rs));
		/*s = sql_binop_(sql, NULL, "sql_sub", ls, rs);*/
		s = stmt_binop(ls, rs, sub);
		s = stmt_select(s, stmt_atom_wrd(0), cmp_gt);

		/* A ids */
		s = stmt_join(stmt_reverse(lm), s, cmp_equal);
		/* now we need to add the groups which weren't in B */
		s = stmt_union(ld,s);
		/* now we have gid,cnt, blowup to full groupsizes */
		s = stmt_gen_group(s);
	/*
	} else {
		stmt_destroy(s);
		s = ld;
	*/
	}
	s = stmt_mark_tail(s, 500); 
	/* from gid back to A id's */
	s = stmt_reverse(stmt_join(stmt_dup(lgrp->ext), s, cmp_equal));

	grp_destroy(lgrp);
	grp_destroy(rgrp);

	/* project columns of left hand expression */
	for (n = left->op1.lval->h; n; n = n->next) {
		stmt *c1 = column(n->data);
		char *rnme = NULL;
		char *nme = column_name(c1);

		/* retain name via the stmt_alias */
		c1 = stmt_join(stmt_dup(s), c1, cmp_equal);

		rnme = table_name(c1);
		c1 = stmt_alias(c1, rnme, nme);
		list_append(stmts, c1);
	}
	stmt_destroy(s);
	sub = stmt_list(stmts);

	/* TODO put in sep function !!!, and add to all is_project(op) */
	/* except can be a projection too */
	if (rel->exps) {
		node *en;
		list *l = create_stmt_list();

		for( en = rel->exps->h; en; en = en->next ) {
			sql_exp *exp = en->data;
			stmt *s = exp_bin(sql, exp, sub, NULL, NULL, NULL);

			if (!s) {
				assert(0);
				cond_stmt_destroy(sub);
				list_destroy(l);
				return NULL;
			}
			s = stmt_rename(rel, exp, s);
			list_append(l, s);
		}
		stmt_destroy(sub);
		sub = stmt_list(l);
	}

	stmt_destroy(left);
	stmt_destroy(right);
	if (need_distinct(rel))
		sub = rel2bin_distinct(sub);
	return sub;
}

static stmt *
rel2bin_inter( mvc *sql, sql_rel *rel, list *refs)
{
	list *stmts; 
	node *n, *m;
	stmt *left = NULL, *right = NULL, *sub;

	group *lgrp = NULL, *rgrp = NULL;
	stmt *s, *lm, *ls = NULL, *rs = NULL;
	sql_subaggr *a;

	if (rel->l) /* first construct the left sub relation */
		left = subrel_bin(sql, rel->l, refs);
	if (rel->r) /* first construct the right sub relation */
		right = subrel_bin(sql, rel->r, refs);
	if (!left || !right) { 
		if (left) stmt_destroy(left);
		if (right) stmt_destroy(right);
		return NULL;	
	}
	left = row2cols(left);

	/* construct relation */
	stmts = create_stmt_list();
	/*
	 * The multi column intersect is handled using group by's and
	 * group size counts on both sides of the intersect. We then
	 * return for each group of A with min(A.count,B.count), 
	 * number of rows.
	 * 
	 * The problem with this approach is that the groups should
	 * have equal group identifiers. So we take the union of all
	 * columns before the group by.
	 */
	for (n = left->op1.lval->h; n; n = n->next) 
		lgrp = grp_create(column(n->data), lgrp);
	for (n = right->op1.lval->h; n; n = n->next) 
		rgrp = grp_create(column(n->data), rgrp);

	if (!lgrp || !rgrp) {
		grp_destroy(lgrp);
		grp_destroy(rgrp);
		return NULL;
	}
	grp_done(lgrp);
	grp_done(rgrp);

 	a = sql_bind_aggr(sql->session->schema, "count", NULL);
	ls = stmt_aggr(stmt_dup(lgrp->grp), grp_dup(lgrp), a, 1); 
	a = sql_dup_aggr(a);
	rs = stmt_aggr(stmt_dup(rgrp->grp), grp_dup(rgrp), a, 1); 

	/* now find the matching groups */
	s = stmt_releqjoin_init();
	for (n = left->op1.lval->h, m = right->op1.lval->h; n && m; n = n->next, m = m->next) {
		stmt *l = column(n->data);
		stmt *r = column(m->data);

		l = stmt_join(stmt_dup(lgrp->ext), l, cmp_equal);
		r = stmt_join(stmt_dup(rgrp->ext), r, cmp_equal);
		stmt_releqjoin_fill(s, l, r);
	}
		
	/*if (!distinct) */
	{
		sql_subfunc *min;

		lm = stmt_reverse(stmt_mark_tail(stmt_dup(s),0));
		ls = stmt_join(stmt_dup(lm),ls,cmp_equal);
		rs = stmt_join(stmt_mark(s,0),rs,cmp_equal);

 		min = sql_bind_func(sql->session->schema, "sql_min", tail_type(ls), tail_type(rs));
		/*s = sql_binop_(sql, NULL, "sql_min", ls, rs);*/
		s = stmt_binop(ls, rs, min);
		/* A ids */
		s = stmt_join(stmt_reverse(lm), s, cmp_equal);
		/* now we have gid,cnt, blowup to full groupsizes */
		s = stmt_gen_group(s);
	}
	s = stmt_mark_tail(s, 500); 
	/* from gid back to A id's */
	s = stmt_reverse(stmt_join(stmt_dup(lgrp->ext), s, cmp_equal));

	grp_destroy(lgrp);
	grp_destroy(rgrp);

	/* project columns of left hand expression */
	for (n = left->op1.lval->h; n; n = n->next) {
		stmt *c1 = column(n->data);
		char *rnme = NULL;
		char *nme = column_name(c1);

		/* retain name via the stmt_alias */
		c1 = stmt_join(stmt_dup(s), c1, cmp_equal);

		rnme = table_name(c1);
		c1 = stmt_alias(c1, rnme, nme);
		list_append(stmts, c1);
	}
	stmt_destroy(s);
	sub = stmt_list(stmts);

	/* TODO put in sep function !!!, and add to all is_project(op) */
	/* intersection can be a projection too */
	if (rel->exps) {
		node *en;
		list *l = create_stmt_list();

		for( en = rel->exps->h; en; en = en->next ) {
			sql_exp *exp = en->data;
			stmt *s = exp_bin(sql, exp, sub, NULL, NULL, NULL);

			if (!s) {
				assert(0);
				cond_stmt_destroy(sub);
				list_destroy(l);
				return NULL;
			}
			s = stmt_rename(rel, exp, s);
			list_append(l, s);
		}
		stmt_destroy(sub);
		sub = stmt_list(l);
	}

	stmt_destroy(left);
	stmt_destroy(right);
	if (need_distinct(rel))
		sub = rel2bin_distinct(sub);
	return sub;
}

static stmt *
sql_reorder(stmt *order, stmt *s) 
{
	list *l = create_stmt_list();
	node *n;

	/* we need to keep the order by column, to propagate the sort property*/
	order = stmt_mark(stmt_reverse(order), 0);
	for (n = s->op1.lval->h; n; n = n->next) {
		stmt *sc = n->data;
		char *cname = column_name(sc);
		char *tname = table_name(sc);

		sc = stmt_project(stmt_dup(order), stmt_dup(sc));
		sc = stmt_alias(sc, tname, cname );
		list_append(l, sc);
	}
	stmt_destroy(s);
	stmt_destroy(order);
	return stmt_list(l);
}

static sql_exp*
topn_limit( sql_rel *rel )
{
	if (rel->exps) {
		sql_exp *limit = rel->exps->h->data;

		return limit;
	}
	return NULL;
}

static sql_exp*
topn_offset( sql_rel *rel )
{
	if (rel->exps && list_length(rel->exps) > 1) {
		sql_exp *offset = rel->exps->h->next->data;

		return offset;
	}
	return NULL;
}

static sql_table *
rel_alter_table_get(sql_rel *r)
{
	if (r->flag == DDL_ALTER_TABLE) {
		sql_exp *e = r->exps->t->data;
		atom *a = e->l;

		return a->data.val.pval;
	}
	return NULL;
}

static stmt *
rel2bin_project( mvc *sql, sql_rel *rel, list *refs, sql_rel *topn)
{
	list *pl; 
	node *en, *n;
	stmt *sub = NULL, *psub = NULL;
	stmt *l = NULL;

	if (topn) {
		sql_exp *le = topn_limit(topn);
		sql_exp *oe = topn_offset(topn);

		if (!le) { /* for now only handle topn 
				including limit, ie not just offset */
			topn = NULL;
		} else {
			l = exp_bin(sql, le, NULL, NULL, NULL, NULL);
			if (oe) {
				sql_subtype *wrd = sql_bind_localtype("wrd");
				sql_subfunc *add = sql_bind_func_result(sql->session->schema, "sql_add", wrd, wrd, wrd);
				stmt *o = exp_bin(sql, oe, NULL, NULL, NULL, NULL);
				l = stmt_binop(l, o, add);
			}
		}
	}

	if (!rel->exps) 
		return stmt_none();

	if (rel->l) { /* first construct the sub relation */
		sql_rel *l = rel->l;
		if (l->op == op_ddl) {
			sql_table *t = rel_alter_table_get(l);

			if (t)
				sub = rel2bin_sql_table(t);
		} else {
			sub = subrel_bin(sql, rel->l, refs);
		}
		if (!sub) 
			return NULL;	
		if (sub->type == st_ordered) {
			stmt *n = sql_reorder(stmt_dup(sub->op1.stval), stmt_dup(sub->op2.stval));
			stmt_destroy(sub);
			sub = n;
		}
	}

	pl = create_stmt_list();
	psub = stmt_list(pl);
	for( en = rel->exps->h; en; en = en->next ) {
		sql_exp *exp = en->data;
		stmt *s = exp_bin(sql, exp, sub, NULL, NULL, NULL);

		if (!s)
			s = exp_bin(sql, exp, sub, psub, NULL, NULL);
		if (!s) {
			assert(0);
			cond_stmt_destroy(sub);
			list_destroy(pl);
			return NULL;
		}
		if (sub && sub->nrcols >= 1 && s->nrcols == 0)
			s = stmt_const(bin_first_column(sub), s);
			
		s = stmt_rename(rel, exp, s);
		list_append(pl, s);
	}
	stmt_set_nrcols(psub);

	/* In case of a topn 
		if both order by and distinct: then get first order by col early
			do topn on it. Project all again! Then rest
         */
	if (topn && rel->r) {
		list *oexps = rel->r, *npl = create_stmt_list();
		/* including bounds, topn returns atleast N */
		int including = need_including(topn) || need_distinct(rel);
		stmt *limit = NULL; 

		for (n=oexps->h; n; n = n->next) {
			sql_exp *orderbycole = n->data; 
 			int inc = including || n->next;

			stmt *orderbycolstmt = exp_bin(sql, orderbycole, sub, psub, NULL, NULL); 

			if (!orderbycolstmt) {
				stmt_destroy(sub);
				stmt_destroy(limit);
				return NULL;
			}
			
			if (!limit) {	/* topn based on a single column */
				limit = stmt_limit(orderbycolstmt, stmt_atom_wrd(0), l, LIMIT_DIRECTION(is_ascending(orderbycole), 1, inc));
			} else { 	/* topn based on 2 columns */
				stmt *obc = stmt_project(stmt_mirror(limit), orderbycolstmt);
				limit = stmt_limit2(limit, obc, stmt_atom_wrd(0), l, LIMIT_DIRECTION(is_ascending(orderbycole), 1, inc));
			}
			if (!limit) {
				stmt_destroy(sub);
				return NULL;
			}
		}

		limit = stmt_mirror(limit);
		for ( n=pl->h ; n; n = n->next) 
			list_append(npl, stmt_project(stmt_dup(limit), column(stmt_dup(n->data))));
		stmt_destroy(psub);
		psub = stmt_list(npl);

		/* also rebuild sub as multiple orderby expressions may use the sub table (ie aren't part of the result columns) */
		pl = sub->op1.lval;
		npl = create_stmt_list();
		for ( n=pl->h ; n; n = n->next) {
			list_append(npl, stmt_project(stmt_dup(limit), column(stmt_dup(n->data)))); 
		}
		stmt_destroy(sub);
		stmt_destroy(limit);
		sub = stmt_list(npl);
	}
	if (need_distinct(rel))
		psub = rel2bin_distinct(psub);
	if ((!topn || need_distinct(rel)) && rel->r) {
		list *oexps = rel->r;
		stmt *orderby = NULL;

		for (en = oexps->h; en; en = en->next) {
			sql_exp *orderbycole = en->data; 
			stmt *orderbycolstmt = exp_bin(sql, orderbycole, sub, psub, NULL, NULL); 

			if (!orderbycolstmt) {
				assert(0);
				stmt_destroy(sub);
				cond_stmt_destroy(orderby);
				return NULL;
			}
			/* single values don't need sorting */
			if (orderbycolstmt->nrcols == 0) {
				stmt_destroy(orderbycolstmt);
				if (orderby)
					stmt_destroy(orderby);
				orderby = NULL;
				break;
			}
			if (orderby)
				orderby = stmt_reorder(orderby, orderbycolstmt, is_ascending(orderbycole));
			else
				orderby = stmt_order(orderbycolstmt, is_ascending(orderbycole));
		}
		if (orderby)
			//psub = stmt_ordered(orderby, psub);
			psub = sql_reorder(orderby, psub);
	}
	if (sub) 
		stmt_destroy(sub);
	return psub;
}

static stmt *
rel2bin_predicate(void) 
{
	return const_column(stmt_bool(1));
}

static stmt *
rel2bin_hash_lookup( mvc *sql, sql_rel *rel, stmt *sub, sql_idx *i, node *en ) 
{
	sql_subtype *it = sql_bind_localtype("int");
	sql_subtype *wrd = sql_bind_localtype("wrd");
	stmt *h = NULL;
	stmt *bits = stmt_atom_int(1 + ((sizeof(wrd)*8)-1)/(list_length(i->columns)+1));
	sql_exp *e = en->data;
	sql_exp *l = e->l;
	stmt *idx = bin_find_column(sub, l->l, i->base.name);

	/* TODO should be in key order! */
	for( en = rel->exps->h; en; en = en->next ) {
		sql_exp *e = en->data;
		stmt *s;

		assert(e->type == e_cmp && e->flag == cmp_equal);
		s = exp_bin(sql, e->r, NULL, NULL, NULL, NULL);
		if (s == NULL)
			return NULL;

		if (!s) {
			stmt_destroy(h);
			stmt_destroy(bits);
			return NULL;
		}
		if (h) {
			sql_subfunc *xor = sql_bind_func_result3(sql->session->schema, "rotate_xor_hash", wrd, it, tail_type(s), wrd);

			h = stmt_Nop(stmt_list( list_append( list_append(
				list_append(create_stmt_list(), h), 
				stmt_dup(bits)), s)), xor);
		} else {
			sql_subfunc *hf = sql_bind_func_result(sql->session->schema, "hash", tail_type(s), NULL, wrd);

			h = stmt_unop(s, hf);
		}
	}
	stmt_destroy(bits);
	return stmt_uselect(idx, h, cmp_equal);
}


static stmt *
rel2bin_select( mvc *sql, sql_rel *rel, list *refs)
{
	list *l; 
	node *en, *n;
	stmt *sub = NULL, *sel = NULL;
	stmt *predicate = NULL;

	if (!rel->exps) {
		assert(0);
		return NULL;
	}

	if (rel->l) { /* first construct the sub relation */
		sub = subrel_bin(sql, rel->l, refs);
		if (!sub) 
			return NULL;	
		sub = row2cols(sub);
	} else {
		predicate = rel2bin_predicate();
	}
	if (!rel->exps->h) {
		if (sub)
			return sub;
		return predicate;
	}
	/* handle possible index lookups */
	/* expressions are in index order ! */
	if (sub && (en = rel->exps->h) != NULL) { 
		sql_exp *e = en->data;
		prop *p;

		if ((p=find_prop(e->p, PROP_HASHIDX)) != NULL) {
			sql_idx *i = p->value;
			
			sel = rel2bin_hash_lookup(sql, rel, sub, i, en);
		}
	}
	sel = stmt_relselect_init();
	for( en = rel->exps->h; en; en = en->next ) {
		/*stmt *s = exp_bin(sql, en->data, sub, NULL, NULL, sel);*/
		stmt *s = exp_bin(sql, en->data, sub, NULL, NULL, NULL);

		if (!s) {
			assert(0);
			if (sub) stmt_destroy(sub);
			if (predicate) stmt_destroy(predicate);
			if (sel) stmt_destroy(sel);
			return NULL;
		}
		if (s->nrcols == 0){ 
			if (!predicate) 
				predicate = rel2bin_predicate();
			predicate = stmt_select(predicate, s, cmp_equal);
		} else {
			/*
			if (sel) 
				stmt_destroy(sel);
			sel = s;
			*/
			stmt_relselect_fill(sel, s);
		}
	}

	if (predicate && sel) {
		if (list_length(sel->op1.lval) == 0) {
			stmt_destroy(sel);
			sel = NULL;
		} else {
			sel = stmt_join(sel, stmt_dup(predicate), cmp_all);
		}
	}
	/* construct relation */
	l = create_stmt_list();
	if (sub && sel) {
		sel = stmt_mark(stmt_reverse(sel),0);
		for( n = sub->op1.lval->h; n; n = n->next ) {
			stmt *col = stmt_dup(n->data);
	
			sel = stmt_dup(sel);
			if (col->nrcols == 0) /* constant */
				col = stmt_const(sel, col);
			else
				col = stmt_project(sel, col);
			list_append(l, col);
		}
	} else if (sub && predicate) {
		stmt *h = NULL;
		n = sub->op1.lval->h;
		h = stmt_join( stmt_dup(n->data), stmt_dup(predicate), cmp_all);
		h = stmt_reverse(stmt_mark_tail(h, 0)); 
		for( n = sub->op1.lval->h; n; n = n->next ) {
			stmt *col = stmt_dup(n->data);
	
			h = stmt_dup(h);
			if (col->nrcols == 0) /* constant */
				col = stmt_const(h, col);
			else
				col = stmt_join(h, col, cmp_equal);
			list_append(l, col);
		}
		stmt_destroy(h);
	} else if (predicate) {
		list_append(l, stmt_dup(predicate));
	}
	if (predicate) stmt_destroy(predicate);
	if (sub) stmt_destroy(sub);
	if (sel) stmt_destroy(sel);
	return stmt_list(l);
}

static stmt *
rel2bin_groupby( mvc *sql, sql_rel *rel, list *refs)
{
	list *l, *aggrs;
	node *n, *en;
	stmt *sub = NULL, *cursub;
	group *groupby = NULL;

	if (rel->l) { /* first construct the sub relation */
		sub = subrel_bin(sql, rel->l, refs);
		if (!sub)
			return NULL;	
	}

	if (sub && sub->type == st_list && sub->op1.lval->h && !((stmt*)sub->op1.lval->h->data)->nrcols) {
		list *newl = create_stmt_list();
		node *n;

		for(n=sub->op1.lval->h; n; n = n->next) {
			char *cname = column_name(n->data);
			char *tname = table_name(n->data);
			stmt *s = column(stmt_dup(n->data));

			s = stmt_alias(s, tname, cname );
			append(newl, s);
		}
		stmt_destroy(sub);
		sub = stmt_list(newl);
	}

	/* groupby columns */
	if (rel->r) {
		list *exps = rel->r; 

		for( en = exps->h; en; en = en->next ) {
			sql_exp *e = en->data; 
			stmt *gbcol = exp_bin(sql, e, sub, NULL, NULL, NULL); 
	
			if (!gbcol) {
				assert(0);
				stmt_destroy(sub);
				if (groupby) 
					grp_destroy(groupby);
				return NULL;
			}
			groupby = grp_create(gbcol, groupby);
		}
	}
	grp_done(groupby);
	/* now aggregate */
	l = create_stmt_list();
	aggrs = rel->exps;
	cursub = stmt_list(l);
	for( n = aggrs->h; n; n = n->next ) {
		sql_exp *aggrexp = n->data;

		stmt *aggrstmt = exp_bin(sql, aggrexp, sub, NULL, groupby, NULL); 

		/* maybe the aggr uses intermediate results of this group by,
		   therefore we pass the group by columns too 
		 */
		if (!aggrstmt) 
			aggrstmt = exp_bin(sql, aggrexp, sub, cursub, groupby, NULL); 
		if (!aggrstmt) {
			assert(0);
			stmt_destroy(cursub);
			stmt_destroy(sub);
			if (groupby) grp_destroy(groupby);
			return NULL;
		}

		aggrstmt = stmt_rename(rel, aggrexp, aggrstmt);
		list_append(l, aggrstmt);
	}
	if (sub) stmt_destroy(sub);
	if (groupby) grp_destroy(groupby);
	stmt_set_nrcols(cursub);
	return cursub;
}

static stmt *
rel2bin_topn( mvc *sql, sql_rel *rel, list *refs)
{
	list *newl;
	sql_exp *oe = NULL, *le = NULL;
	stmt *sub = NULL, *order = NULL, *l = NULL, *o = NULL;
	node *n;

	if (rel->l) { /* first construct the sub relation */
		sql_rel *rl = rel->l;

		if (rl->op == op_project) {
			sub = rel2bin_project(sql, rl, refs, rel);
		} else {
			sub = subrel_bin(sql, rl, refs);
		}
	}
	if (!sub) 
		return NULL;	

	le = topn_limit(rel);
	oe = topn_offset(rel);

	if (sub->type == st_ordered) {
		stmt *s = stmt_dup(sub->op2.stval);
		order = column(stmt_dup(sub->op1.stval));
		stmt_destroy(sub);
		sub = s;
	}
	n = sub->op1.lval->h;
	newl = create_stmt_list();

	if (n) {
		stmt *limit = NULL;
		sql_rel *rl = rel->l;
		int including = (rl && need_distinct(rl)) || need_including(rel);

		if (le)
			l = exp_bin(sql, le, NULL, NULL, NULL, NULL);
		if (oe)
			o = exp_bin(sql, oe, NULL, NULL, NULL, NULL);

		if (!l) 
			l = stmt_atom_wrd_nil();
		if (!o)
			o = stmt_atom_wrd(0);

		if (order) {
		 	limit = stmt_limit(stmt_dup(order), o, l, LIMIT_DIRECTION(0,0,including));
		} else {
			stmt *sc = stmt_dup(n->data);
			char *cname = column_name(sc);
			char *tname = table_name(sc);

			sc = column(sc);
			limit = stmt_limit(stmt_alias(sc, tname, cname), o, l, LIMIT_DIRECTION(0,0,including));
		}

		limit = stmt_mirror(limit);
		for ( ; n; n = n->next) {
			stmt *sc = stmt_dup(n->data);
			char *cname = column_name(sc);
			char *tname = table_name(sc);
		
			sc = column(sc);
			sc = stmt_project(stmt_dup(limit), sc);
			list_append(newl, stmt_alias(sc, tname, cname));
		}
		if (order) 
			order = stmt_project(stmt_dup(limit), order);
	}
	stmt_destroy(sub);
	sub = stmt_list(newl);
	if (order) 
		return stmt_ordered(order, sub);
	return sub;
}

static stmt *
nth( list *l, int n)
{
	int i;
	node *m;

	for (i=0, m = l->h; i<n && m; i++, m = m->next) ; 
	if (m)
		return m->data;
	return NULL;
}

static stmt *
insert_check_ukey(mvc *sql, list *inserts, sql_key *k, stmt *idx_inserts)
{
/* pkey's cannot have NULLs, ukeys however can
   current implementation switches on 'NOT NULL' on primary key columns */

	char *msg = NULL;
	stmt *res;

	sql_subtype *wrd = sql_bind_localtype("wrd");
	sql_subaggr *cnt = sql_bind_aggr(sql->session->schema, "count", NULL);
	sql_subtype *bt = sql_bind_localtype("bit");
	stmt *ts = stmt_basetable(k->t, k->t->base.name);
	sql_subfunc *ne = sql_bind_func_result(sql->session->schema, "<>", wrd, wrd, bt);

	if (list_length(k->columns) > 1) {
		node *m;
		stmt *s = nth(inserts, 0)->op2.stval;
		sql_subaggr *sum;
		stmt *ssum = NULL;
		stmt *col = NULL;

		/* 1st stage: find out if original contains same values */
		if (s->key && s->nrcols == 0) {
			s = stmt_relselect_init();
			if (k->idx && hash_index(k->idx->type))
				stmt_relselect_fill(s, stmt_uselect(stmt_idxbat(k->idx, RDONLY), stmt_dup(idx_inserts), cmp_equal));
			for (m = k->columns->h; m; m = m->next) {
				sql_kc *c = m->data;

				col = stmt_bat(c->c, stmt_dup(ts), RDONLY);
				if ((k->type == ukey) && stmt_has_null(stmt_dup(col))) {
					sql_subtype *t = tail_type(col);
					stmt *n = stmt_atom(atom_general(t, NULL, 0));
					col = stmt_select2(col, n, stmt_dup(n), 0);
				}
				stmt_relselect_fill(s, stmt_uselect( col, stmt_dup(nth(inserts, c->c->colnr)->op2.stval), cmp_equal));
			}
		} else {
			s = stmt_releqjoin_init();
			if (k->idx && hash_index(k->idx->type))
				stmt_releqjoin_fill(s, stmt_idxbat(k->idx, RDONLY), stmt_dup(idx_inserts));
			for (m = k->columns->h; m; m = m->next) {
				sql_kc *c = m->data;

				col = stmt_bat(c->c, stmt_dup(ts), RDONLY);
				if ((k->type == ukey) && stmt_has_null(stmt_dup(col))) {
					sql_subtype *t = tail_type(col);
					stmt *n = stmt_atom(atom_general(t, NULL, 0));
					col = stmt_select2(col, n, stmt_dup(n), 0);
				}
				stmt_releqjoin_fill(s, col, stmt_dup(nth(inserts, c->c->colnr)->op2.stval));
			}
		}
		s = stmt_binop(stmt_aggr(s, NULL, cnt, 1), stmt_atom_wrd(0), ne);

		/* 2e stage: find out if inserted are unique */
		if ((!idx_inserts && nth(inserts,0)->nrcols) || (idx_inserts && idx_inserts->nrcols)) {	/* insert columns not atoms */
#if 0
			sql_subtype *Oid = sql_bind_localtype("oid");
			sql_subfunc *nu, *or = sql_bind_func_result(sql->session->schema, "or", bt, bt, bt);
			stmt *ss = NULL;

			/* implementation uses group, not_uniques,
				join, derive, not_uniques keyed check */
			for (m = k->columns->h; m; m = m->next) {
				sql_kc *c = m->data;

				if (ss) {
					ss = stmt_derive(ss, stmt_join(stmt_mirror(stmt_dup(ss)), stmt_dup(nth(inserts, c->c->colnr)->op2.stval), cmp_equal));
				} else {
					ss = stmt_group(stmt_dup(nth(inserts, c->c->colnr)->op2.stval));
				}
			}
 			nu = sql_bind_func_result(sql->session->schema, "not_uniques", tail_type(ss), NULL, Oid);
			ss = stmt_unop(ss, nu);

			sum = sql_bind_aggr(sql->session->schema, "not_unique", tail_type(ss));
			ssum = stmt_aggr(ss, NULL, sum, 1);
			/* combine results */
			s = stmt_binop(s, ssum, or);
#else
			stmt *ss = NULL;
			sql_subfunc *or = sql_bind_func_result(sql->session->schema, "or", bt, bt, bt);
			/* implementation uses sort,refine, key check */
			for (m = k->columns->h; m; m = m->next) {
				sql_kc *c = m->data;

				if (ss)  
					ss = stmt_reorder(ss, stmt_dup(nth(inserts, c->c->colnr)->op2.stval), 1);
				else
					ss = stmt_order(stmt_dup(nth(inserts, c->c->colnr)->op2.stval), 1);
			}

			sum = sql_bind_aggr(sql->session->schema, "not_unique", tail_type(ss));
			ssum = stmt_aggr(ss, NULL, sum, 1);
			/* combine results */
			s = stmt_binop(s, ssum, or);
#endif
		}

		if (k->type == pkey) {
			msg = sql_message( "INSERT INTO: PRIMARY KEY constraint '%s.%s' violated", k->t->base.name, k->base.name);
		} else {
			msg = sql_message( "INSERT INTO: UNIQUE constraint '%s.%s' violated", k->t->base.name, k->base.name);
		}
		res = stmt_exception(s, msg, 00001);
	} else {		/* single column key */
		sql_kc *c = k->columns->h->data;
		stmt *s, *h = nth(inserts, c->c->colnr)->op2.stval;

		s = stmt_bat(c->c, stmt_dup(ts), RDONLY);
		if ((k->type == ukey) && stmt_has_null(stmt_dup(s))) {
			sql_subtype *t = tail_type(h);
			stmt *n = stmt_atom(atom_general(t, NULL, 0));
			s = stmt_select2(s, n, stmt_dup(n), 0);
		}
		if (h->nrcols) {
			s = stmt_join(s, stmt_reverse(stmt_dup(h)), cmp_equal);
			/* s should be empty */
			s = stmt_aggr(s, NULL, cnt, 1);
		} else {
			s = stmt_uselect(s, stmt_dup(h), cmp_equal);
			/* s should be empty */
			s = stmt_aggr(s, NULL, cnt, 1);
		}
		/* s should be empty */
		s = stmt_binop(s, stmt_atom_wrd(0), ne);

		/* 2e stage: find out if inserts are unique */
		if (h->nrcols) {	/* insert multiple atoms */
			sql_subaggr *sum;
			stmt *count_sum = NULL;
			sql_subfunc *or = sql_bind_func_result(sql->session->schema, "or", bt, bt, bt);
			stmt *ssum, *ss;

			stmt *ins = stmt_dup(nth(inserts, c->c->colnr)->op2.stval);
			group *g = grp_create(ins, NULL);

			grp_done(g);
			ss = stmt_aggr(stmt_dup(g->grp), g, sql_dup_aggr(cnt), 1);
			/* (count(ss) <> sum(ss)) */
			sum = sql_bind_aggr(sql->session->schema, "sum", tail_type(ss));
			ssum = stmt_aggr(ss, NULL, sum, 1);
			ssum = sql_Nop_(sql, "ifthenelse", sql_unop_(sql, NULL, "isnull", ssum), stmt_atom_wrd(0), stmt_dup(ssum), NULL);
			count_sum = stmt_binop(check_types(sql, tail_type(ssum), stmt_aggr(stmt_dup(ss), NULL, sql_dup_aggr(cnt), 1), type_equal), ssum, sql_dup_func(ne));

			/* combine results */
			s = stmt_binop(s, count_sum, or);
		}
		if (k->type == pkey) {
			msg = sql_message( "INSERT INTO: PRIMARY KEY constraint '%s.%s' violated", k->t->base.name, k->base.name);
		} else {
			msg = sql_message( "INSERT INTO: UNIQUE constraint '%s.%s' violated", k->t->base.name, k->base.name);
		}
		res = stmt_exception(s, msg, 00001);
	}
	stmt_destroy(ts);
	return res;
}

static stmt *
insert_check_fkey(mvc *sql, list *inserts, sql_key *k, stmt *idx_inserts)
{
	char *msg = NULL;
	stmt *s = nth(inserts, 0)->op2.stval;
	sql_subtype *wrd = sql_bind_localtype("wrd");
	sql_subaggr *cnt = sql_bind_aggr(sql->session->schema, "count", NULL);
	sql_subtype *bt = sql_bind_localtype("bit");
	sql_subfunc *ne = sql_bind_func_result(sql->session->schema, "<>", wrd, wrd, bt);

	(void) sql;		/* unused! */

	if (s->key && s->nrcols == 0) {
		s = stmt_binop(stmt_aggr(stmt_dup(idx_inserts), NULL, cnt, 1), stmt_atom_wrd(1), ne);
	} else {
		/* releqjoin.count <> inserts[col1].count */
		stmt *ins = stmt_dup(nth(inserts, 0)->op2.stval);

		s = stmt_binop(stmt_aggr(stmt_dup(idx_inserts), NULL, cnt, 1), stmt_aggr(ins, NULL, sql_dup_aggr(cnt), 1), ne);
	}

	/* s should be empty */
	msg = sql_message( "INSERT INTO: FOREIGN KEY constraint '%s.%s' violated", k->t->base.name, k->base.name);
	return stmt_exception(s, msg, 00001);
}

static stmt *
sql_insert_key(mvc *sql, list *inserts, sql_key *k, stmt *idx_inserts)
{
	/* int insert = 1;
	 * while insert and has u/pkey and not defered then
	 *      if u/pkey values exist then
	 *              insert = 0
	 * while insert and has fkey and not defered then
	 *      find id of corresponding u/pkey  
	 *      if (!found)
	 *              insert = 0
	 * if insert
	 *      insert values
	 *      insert fkey/pkey index
	 */
	if (k->type == pkey || k->type == ukey) {
		return insert_check_ukey(sql, inserts, k, idx_inserts );
	} else {		/* foreign keys */
		return insert_check_fkey(sql, inserts, k, idx_inserts );
	}
}

static stmt *
hash_insert(mvc *sql, sql_idx * i, list *inserts)
{
	node *m;
	sql_subtype *it, *wrd;
	int bits = 1 + ((sizeof(wrd)*8)-1)/(list_length(i->columns)+1);
	stmt *h = NULL, *o = NULL;

	if (list_length(i->columns) <= 1)
		return NULL;

	it = sql_bind_localtype("int");
	wrd = sql_bind_localtype("wrd");
	for (m = i->columns->h; m; m = m->next) {
		sql_kc *c = m->data;
		stmt *is = stmt_dup(nth(inserts, c->c->colnr)->op2.stval);

		if (h && i->type == hash_idx)  { 
			sql_subfunc *xor = sql_bind_func_result3(sql->session->schema, "rotate_xor_hash", wrd, it, &c->c->type, wrd);

			h = stmt_Nop(stmt_list( list_append( list_append(
				list_append(create_stmt_list(), h), 
				stmt_atom_int(bits)), 
				(o)?stmt_join(stmt_dup(o), is, cmp_equal):is)), 
				xor);
		} else if (h)  { 
			stmt *h2;
			sql_subfunc *lsh = sql_bind_func_result(sql->session->schema, "left_shift", wrd, it, wrd);
			sql_subfunc *lor = sql_bind_func_result(sql->session->schema, "bit_or", wrd, wrd, wrd);
			sql_subfunc *hf = sql_bind_func_result(sql->session->schema, "hash", &c->c->type, NULL, wrd);

			h = stmt_binop(h, stmt_atom_int(bits), lsh); 
			h2 = stmt_unop(
			    (o)?stmt_join(stmt_dup(o), is, cmp_equal):is, hf);
			h = stmt_binop(h, h2, lor);
		} else {
			sql_subfunc *hf = sql_bind_func_result(sql->session->schema, "hash", &c->c->type, NULL, wrd);
			if (is->nrcols == 0 && is->key) {
				h = stmt_unop(is, hf);
			} else {
				o = stmt_mark(stmt_reverse(is), 40); 
				h = stmt_unop(stmt_mark(stmt_dup(is), 40), hf);
			}
			if (i->type == oph_idx)
				break;
		}
	}
	if (o)
		return stmt_join(stmt_reverse(o), h, cmp_equal);
	return h;
}

static stmt *
join_idx_insert(mvc *sql, sql_idx * i, list *inserts)
{
	node *m, *o;
	sql_key *rk = &((sql_fkey *) i->key)->rkey->k;
	stmt *rts = stmt_basetable(rk->t, rk->t->base.name);
	stmt *s = nth(inserts, 0)->op2.stval;

	if (s->key && s->nrcols == 0) {
		sql_subtype *bt = sql_bind_localtype("bit");
		sql_subfunc *or = sql_bind_func_result(sql->session->schema, "or", bt, bt, bt);
		stmt *cond = NULL;

		s = stmt_relselect_init();
		for (m = i->columns->h, o = rk->columns->h; m && o; m = m->next, o = o->next) {
			sql_kc *c = m->data;
			sql_kc *rc = o->data;

			stmt_relselect_fill(s, stmt_uselect(stmt_bat(rc->c, stmt_dup(rts), RDONLY), stmt_dup(nth(inserts, c->c->colnr)->op2.stval), cmp_equal));

			if (c->c->null) {
				sql_subfunc *isnil = sql_bind_func(sql->session->schema, "isnull", &c->c->type, NULL);
				stmt *ins = stmt_dup(nth(inserts, c->c->colnr)->op2.stval);

				ins = stmt_unop(ins, isnil);
				if (!cond) {
					cond = ins;
				} else {
					cond = stmt_binop(cond, ins, sql_dup_func(or));
				}
			}
		}
		sql_subfunc_destroy(or);

		/* add missing nulls (and NULLs only (SIMPLE MATCH)) */
		s = stmt_mark(stmt_reverse(s), 0);
		if (cond) {
			stmt *notnull = s;
			stmt *isnull = const_column(stmt_atom(atom_general(tail_type(s), NULL, 0)));
			stmt *t = const_column(cond);
			isnull = stmt_join(t, stmt_reverse(stmt_const(stmt_reverse(isnull), stmt_bool(1))), cmp_equal);
			notnull = stmt_join(stmt_dup(t), stmt_reverse(stmt_const(stmt_reverse(notnull), stmt_bool(0))), cmp_equal);
			s = stmt_union(isnull, notnull);
		}
	} else {
		int nulls = 0;

		s = stmt_releqjoin_init();
		for (m = i->columns->h, o = rk->columns->h; m && o; m = m->next, o = o->next) {
			sql_kc *c = m->data;
			sql_kc *rc = o->data;

			stmt_releqjoin_fill(s, stmt_dup(nth(inserts, c->c->colnr)->op2.stval), stmt_bat(rc->c, stmt_dup(rts), RDONLY));
			if (c->c->null)
				nulls = 1;
		}
		/* add missing nulls (and NULLs only (SIMPLE MATCH)) */
		if (nulls) {
			stmt *cur = NULL, *missing = stmt_diff(stmt_dup(nth(inserts, 0)->op2.stval),
							       stmt_dup(s));

			for (m = i->columns->h; m; m = m->next) {

				sql_kc *c = m->data;
				sql_subfunc *isnil = sql_bind_func(sql->session->schema, "isnull",
								   &c->c->type, NULL);

				stmt *n;

				n = stmt_dup(nth(inserts, c->c->colnr)->op2.stval);
				n = stmt_semijoin(n, stmt_dup(missing));
				n = stmt_unop(n, isnil);
				n = stmt_uselect(n, stmt_bool(1), cmp_equal);
				if (cur)
					cur = stmt_union(stmt_diff(cur, stmt_dup(n)), n);
				else
					cur = n;
			}
			stmt_destroy(missing);
			/* There is no merge union, so we need the expensive sort ! */
			s = stmt_union(s, stmt_const(cur, stmt_atom(atom_general(sql_bind_localtype("oid"), NULL, 0))));
		}
		s = stmt_reverse(stmt_order(stmt_reverse(s), 1));
	}
	stmt_destroy(rts);
	return s;
}

static void
sql_insert_idx(mvc *sql, list *inserts, sql_idx * i, list *l )
{
	stmt *is = NULL;

	if (hash_index(i->type)) {
		is = hash_insert(sql, i, inserts);
	} else if (i->type == join_idx) {
		is = join_idx_insert(sql, i, inserts);
	}
	if (i->key) {
		stmt *ckeys = sql_insert_key(sql, inserts, i->key, is);

		list_prepend(l, ckeys);
	}
	/* append to the pre-computed join-index */
	if (is)
		list_append(l, stmt_append_idx(i, is));
}

static int
sql_insert_idxs(mvc *sql, sql_table *t, list *inserts, list *l)
{
	node *n;
	int res = 1;

	if (!t->idxs.set)
		return res;

	for (n = t->idxs.set->h; n; n = n->next) {
		sql_idx *i = n->data;

		sql_insert_idx(sql, inserts, i, l);
	}
	return res;
}

static void
sql_stack_add_inserted( mvc *sql, char *name, sql_table *t) 
{
	sql_rel *r = rel_basetable(t, name );
		
	stack_push_rel_view(sql, name, r);
}

static int
sql_insert_triggers(mvc *sql, sql_table *t, list *l)
{
	node *n;
	int res = 1;

	if (!t->triggers.set)
		return res;

	for (n = t->triggers.set->h; n; n = n->next) {
		sql_trigger *trigger = n->data;
		int *trigger_id = NEW(int);
		*trigger_id = trigger->base.id;

		stack_push_frame(sql, "OLD-NEW");
		if (trigger->event == 0) { 
			stmt *s = NULL;
			char *n = trigger->new_name;

			/* add name for the 'inserted' to the stack */
			if (!n) n = "new"; 
	
			sql_stack_add_inserted(sql, n, t);
			s = sql_parse(sql, NULL, trigger->statement, m_instantiate);
			
			if (!s) 
				return 0;
			if (trigger -> time )
				list_append(l, s);
			else
				list_prepend(l, s);
		}
		stack_pop_frame(sql);
	}
	return res;
}

static void 
sql_insert_check_null(mvc *sql, sql_table *t, list *inserts, list *l) 
{
	node *m, *n;
	sql_subaggr *cnt = sql_bind_aggr(sql->session->schema, "count", NULL);

	for (n = t->columns.set->h, m = inserts->h; n && m; 
		n = n->next, m = m->next) {
		stmt *i = m->data;
		sql_column *c = n->data;

		if (!c->null) {
			stmt *s = i->op2.stval;
			char *msg = NULL;

			if (!(s->key && s->nrcols == 0)) {
				s = stmt_atom(atom_general(&c->type, NULL, 0));
				s = stmt_uselect(stmt_dup(i->op2.stval), s, cmp_equal);
				s = stmt_aggr(s, NULL, sql_dup_aggr(cnt), 1);
			} else {
				sql_subfunc *isnil = sql_bind_func(sql->session->schema, "isnull", &c->type, NULL);

				s = stmt_unop(stmt_dup(i->op2.stval), isnil);
			}
			msg = sql_message( "INSERT INTO: NOT NULL constraint violated for column %s.%s", c->t->base.name, c->base.name);
			s = stmt_exception(s, msg, 00001);

			list_prepend(l, s);
		}
	}
	sql_subaggr_destroy(cnt);
}

static sql_table *
rel_create_table_get(sql_rel *r)
{
	if (r->flag == DDL_CREATE_TABLE || r->flag == DDL_CREATE_VIEW) {
		sql_exp *e = r->exps->t->data;
		atom *a = e->l;

		return a->data.val.pval;
	}
	return NULL;
}

static stmt *
rel2bin_insert( mvc *sql, sql_rel *rel, list *refs)
{
	list *newl, *l;
	stmt *inserts = NULL, *insert = NULL, *s, *ddl = NULL;
	node *n, *m;
	sql_rel *tr = rel->l;
	sql_table *t = NULL;

	if (tr->op == op_basetable) {
		t = tr->l;
	} else {
		assert(tr->flag == DDL_CREATE_TABLE || tr->flag == DDL_CREATE_VIEW);

		ddl = subrel_bin(sql, rel->l, refs);
		if (!ddl)
			return NULL;
		t = rel_create_table_get(tr);
	}

	if (rel->r) /* first construct the inserts relation */
		inserts = subrel_bin(sql, rel->r, refs);

	if (!inserts)  
		return NULL;	

	if (inserts->type == st_ordered) {
		stmt *n = sql_reorder(stmt_dup(inserts->op1.stval), stmt_dup(inserts->op2.stval));
		stmt_destroy(inserts);
		inserts = n;
	}

	newl = create_stmt_list();
	for (n = t->columns.set->h, m = inserts->op1.lval->h; 
		n && m; n = n->next, m = m->next) {

		stmt *i = stmt_dup(m->data);
		sql_column *c = n->data;

		insert = i = stmt_append_col(c, i);
		list_append(newl, i);
	}
	stmt_destroy(inserts);
	if (!insert)
		return NULL;
	l = create_stmt_list();

	if (!sql_insert_idxs(sql, t, newl, l)) 
		return sql_error(sql, 02, "INSERT INTO: failed to update indexes for table '%s'", t->base.name);
	l = list_append(l, stmt_list(newl));
	sql_insert_check_null(sql, t, newl, l);
	if (!sql_insert_triggers(sql, t, l)) 
		return sql_error(sql, 02, "INSERT INTO: triggers failed for table '%s'", t->base.name);
	if (insert->op2.stval->nrcols == 0) {
		s = stmt_atom_wrd(1);
	} else {
		s = stmt_aggr(stmt_dup(insert->op2.stval), NULL, sql_bind_aggr(sql->session->schema, "count", NULL), 1);
	}
	if (ddl)
		list_prepend(l, ddl);
	else
		list_append(l, stmt_affected_rows(s));
	return stmt_list(l);
}

static int
is_idx_updated(sql_idx * i, stmt **updates)
{
	int update = 0;
	node *m;

	for (m = i->columns->h; m; m = m->next) {
		sql_kc *ic = m->data;

		if (updates[ic->c->colnr]) {
			update = 1;
			break;
		}
	}
	return update;
}

static int
first_updated_col(stmt **updates, int cnt)
{
	int i;

	for (i = 0; i < cnt; i++) {
		if (updates[i])
			return i;
	}
	return -1;
}

static stmt ** 
table_update_array(sql_table *t, int *Len)
{
	stmt **updates;
	int i, len = list_length(t->columns.set);
	node *m;

	*Len = len;
	updates = NEW_ARRAY(stmt *, len);
	for (m = t->columns.set->h, i = 0; m; m = m->next, i++) {
		sql_column *c = m->data;

		/* update the column number, for correct array access */
		c->colnr = i;
		updates[i] = NULL;
	}
	return updates;
}

static list * sql_update(mvc *sql, sql_table *t, stmt **updates, int len);

static stmt *
update_check_ukey(mvc *sql, stmt **updates, sql_key *k, stmt *idx_updates, int updcol)
{
	char *msg = NULL;
	stmt *res = NULL;

	sql_subtype *wrd = sql_bind_localtype("wrd");
	sql_subaggr *cnt = sql_bind_aggr(sql->session->schema, "count", NULL);
	sql_subtype *bt = sql_bind_localtype("bit");
	sql_subfunc *ne;

	ne = sql_bind_func_result(sql->session->schema, "<>", wrd, wrd, bt);
	if (list_length(k->columns) > 1) {
		stmt *ts = stmt_basetable(k->t, k->t->base.name);
		node *m;
		stmt *s = NULL;

		/* 1st stage: find out if original (without the updated) 
			do not contain the same values as the updated values. 
			This is done using a relation join and a count (which 
			should be zero)
	 	*/
		/* TODO split null removal and join/group (to make mkey save) */
		if (!isNew(k)) {
			s = stmt_releqjoin_init();
			if (k->idx && hash_index(k->idx->type))
				stmt_releqjoin_fill(s, stmt_diff(stmt_idxbat(k->idx, RDONLY), stmt_dup(idx_updates)), stmt_dup(idx_updates));
			for (m = k->columns->h; m; m = m->next) {
				sql_kc *c = m->data;
				stmt *upd, *l;

				assert(updates);
				if (updates[c->c->colnr]) {
					upd = stmt_dup(updates[c->c->colnr]->op2.stval);
				} else {
					upd = stmt_semijoin(stmt_bat(c->c, stmt_dup(ts), RDONLY), stmt_dup(updates[updcol]->op2.stval));
				}
				if ((k->type == ukey) && stmt_has_null(stmt_dup(upd))) {
					sql_subtype *t = tail_type(upd);
					stmt *n = stmt_atom(atom_general(t, NULL, 0));
					upd = stmt_select2(upd, n, stmt_dup(n), 0);
				}

				l = stmt_diff(stmt_bat(c->c, stmt_dup(ts), RDONLY), stmt_dup(upd));
				stmt_releqjoin_fill(s, l, upd);

			}
			s = stmt_binop(stmt_aggr(s, NULL, cnt, 1), stmt_atom_wrd(0), ne);
		}

		/* 2e stage: find out if the updated are unique */
		if (!updates || updates[updcol]->op2.stval->nrcols) {	/* update columns not atoms */
			sql_subaggr *sum;
			stmt *count_sum = NULL, *ssum;
			group *g = NULL;
			stmt *ss;
			sql_subfunc *or = sql_bind_func_result(sql->session->schema, "or", bt, bt, bt);

			/* also take the hopefully unique hash keys, to reduce
			   (re)group costs */
			if (k->idx && hash_index(k->idx->type))
				g = grp_create(stmt_dup(idx_updates), g);
			for (m = k->columns->h; m; m = m->next) {
				sql_kc *c = m->data;
				stmt *upd;

				if (updates && updates[c->c->colnr]) {
					upd = stmt_dup(updates[c->c->colnr]->op2.stval);
				} else if (updates) {
					upd = stmt_dup(updates[updcol]->op2.stval);
					upd = stmt_semijoin(stmt_bat(c->c, stmt_dup(ts), RDONLY), upd);
				} else {
					upd = stmt_bat(c->c, stmt_dup(ts), RDONLY);
				}
				/* remove nulls */
				if ((k->type == ukey) && stmt_has_null(stmt_dup(upd))) {
					sql_subtype *t = tail_type(upd);
					stmt *n = stmt_atom(atom_general(t, NULL, 0));
					upd = stmt_select2(upd, n, stmt_dup(n), 0);
				}

				g = grp_create(upd, g);
			}
			grp_done(g);
			ss = stmt_aggr(stmt_dup(g->grp), grp_dup(g), sql_dup_aggr(cnt), 1);
			grp_destroy(g);
			/* (count(ss) <> sum(ss)) */
			sum = sql_bind_aggr(sql->session->schema, "sum", tail_type(ss));
			ssum = stmt_aggr(ss, NULL, sum, 1);
			ssum = sql_Nop_(sql, "ifthenelse", sql_unop_(sql, NULL, "isnull", ssum), stmt_atom_wrd(0), stmt_dup(ssum), NULL);
			count_sum = stmt_binop(stmt_aggr(stmt_dup(ss), NULL, sql_dup_aggr(cnt), 1), check_types(sql, wrd, ssum, type_equal), sql_dup_func(ne));

			/* combine results */
			if (s) 
				s = stmt_binop(s, count_sum, or);
			else
				s = count_sum;
		}

		if (k->type == pkey) {
			msg = sql_message( "UPDATE: PRIMARY KEY constraint '%s.%s' violated", k->t->base.name, k->base.name);
		} else {
			msg = sql_message( "UPDATE: UNIQUE constraint '%s.%s' violated", k->t->base.name, k->base.name);
		}
		res = stmt_exception(s, msg, 00001);
	} else {		/* single column key */
		stmt *ts = stmt_basetable(k->t, k->t->base.name);
		sql_kc *c = k->columns->h->data;
		stmt *s = NULL, *h = NULL, *o;

		/* s should be empty */
		if (!isNew(k)) {
			assert (updates);

			h = stmt_dup(updates[c->c->colnr]->op2.stval);
			o = stmt_diff(stmt_bat(c->c, ts, RDONLY), stmt_dup(h));
			s = stmt_join(o, stmt_reverse(h), cmp_equal);
			s = stmt_binop(stmt_aggr(s, NULL, cnt, 1), stmt_atom_wrd(0), ne);
		}

		/* 2e stage: find out if updated are unique */
		if (!h || h->nrcols) {	/* update columns not atoms */
			sql_subaggr *sum;
			stmt *count_sum = NULL;
			sql_subfunc *or = sql_bind_func_result(sql->session->schema, "or", bt, bt, bt);
			stmt *ssum, *ss;
			stmt *upd;
			group *g;

			if (updates) {
 				upd = stmt_dup(updates[c->c->colnr]->op2.stval);
			} else {
 				upd = stmt_bat(c->c, ts, RDONLY);
			}
			/* remove nulls */
			if ((k->type == ukey) && stmt_has_null(stmt_dup(upd))) {
				sql_subtype *t = tail_type(upd);
				stmt *n = stmt_atom(atom_general(t, NULL, 0));
				upd = stmt_select2(upd, n, stmt_dup(n), 0);
			}

			g = grp_create(upd, NULL);
			grp_done(g);
			ss = stmt_aggr(stmt_dup(g->grp), g, sql_dup_aggr(cnt), 1);

			/* (count(ss) <> sum(ss)) */
			sum = sql_bind_aggr(sql->session->schema, "sum", tail_type(ss));
			ssum = stmt_aggr(ss, NULL, sum, 1);
			ssum = sql_Nop_(sql, "ifthenelse", sql_unop_(sql, NULL, "isnull", ssum), stmt_atom_wrd(0), stmt_dup(ssum), NULL);
			count_sum = stmt_binop(check_types(sql, tail_type(ssum), stmt_aggr(stmt_dup(ss), NULL, sql_dup_aggr(cnt), 1), type_equal), ssum, sql_dup_func(ne));

			/* combine results */
			if (s)
				s = stmt_binop(s, count_sum, or);
			else
				s = count_sum;
		}

		if (k->type == pkey) {
			msg = sql_message( "UPDATE: PRIMARY KEY constraint '%s.%s' violated", k->t->base.name, k->base.name);
		} else {
			msg = sql_message( "UPDATE: UNIQUE constraint '%s.%s' violated", k->t->base.name, k->base.name);
		}
		res = stmt_exception(s, msg, 00001);
	}
	return res;
}

static stmt *
update_check_fkey(mvc *sql, stmt **updates, sql_key *k, stmt *idx_updates, int updcol)
{
	char *msg = NULL;
	stmt *s;
	sql_subtype *wrd = sql_bind_localtype("wrd");
	sql_subaggr *cnt = sql_bind_aggr(sql->session->schema, "count", NULL);
	sql_subtype *bt = sql_bind_localtype("bit");
	sql_subfunc *ne = sql_bind_func_result(sql->session->schema, "<>", wrd, wrd, bt);
	stmt *cur;

	if (!idx_updates)
		return NULL;
	/* releqjoin.count <> updates[updcol].count */
	if (updates) {
		cur = stmt_dup(updates[updcol]->op2.stval);
	} else {
		sql_kc *c = k->columns->h->data;
		stmt *ts = stmt_basetable(k->t, k->t->base.name);
		cur = stmt_bat(c->c, ts, RDONLY);
	}
	s = stmt_binop(stmt_aggr(stmt_dup(idx_updates), NULL, cnt, 1), stmt_aggr(cur, NULL, sql_dup_aggr(cnt), 1), ne);

	/* s should be empty */
	msg = sql_message( "UPDATE: FOREIGN KEY constraint '%s.%s' violated", k->t->base.name, k->base.name);
	return stmt_exception(s, msg, 00001);
}

static stmt *
join_updated_pkey(mvc *sql, sql_key * k, stmt **updates, int updcol)
{
	char *msg = NULL;
	int nulls = 0;
	node *m, *o;
	sql_key *rk = &((sql_fkey*)k)->rkey->k;
	stmt *s = NULL, *ts = stmt_basetable(rk->t, rk->t->base.name), *fts;
	stmt *null = NULL, *rows;
	sql_subtype *wrd = sql_bind_localtype("wrd");
	sql_subtype *bt = sql_bind_localtype("bit");
	sql_subaggr *cnt = sql_bind_aggr(sql->session->schema, "count", NULL);
	sql_subfunc *ne = sql_bind_func_result(sql->session->schema, "<>", wrd, wrd, bt);

	fts = stmt_basetable(k->idx->t, k->idx->t->base.name);
	s = stmt_releqjoin_init();

	rows = stmt_idxbat(k->idx, RDONLY);
	rows = stmt_semijoin(stmt_reverse(rows), stmt_dup(updates[updcol]->op2.stval));
	rows = stmt_reverse(rows);

	for (m = k->idx->columns->h, o = rk->columns->h; m && o; m = m->next, o = o->next) {
		sql_kc *fc = m->data;
		sql_kc *c = o->data;
		stmt *upd;

		if (updates[c->c->colnr]) {
			upd = stmt_dup(updates[c->c->colnr]->op2.stval);
		} else {
			upd = stmt_dup(updates[updcol]->op2.stval);
			upd = stmt_semijoin(stmt_bat(c->c, stmt_dup(ts), RDONLY), upd);
		}
		if (c->c->null) {	/* new nulls (MATCH SIMPLE) */
			stmt *nn = stmt_dup(upd);

			nn = stmt_uselect(nn, stmt_atom(atom_general(&c->c->type, NULL, 0)), cmp_equal);
			if (null)
				null = stmt_semijoin(null, nn);
			else
				null = nn;
			nulls = 1;
		}
		stmt_releqjoin_fill(s, upd, stmt_semijoin(stmt_dup(
		  stmt_bat(fc->c, stmt_dup(fts), RDONLY)), stmt_dup(rows) ));
	}
	/* add missing nulls */
	if (nulls)
		s = stmt_union(s, stmt_const(null, stmt_atom(atom_general(sql_bind_localtype("oid"), NULL, 0))));

	stmt_destroy(ts);
	stmt_destroy(fts);

	/* releqjoin.count <> updates[updcol].count */
	s = stmt_binop(stmt_aggr(stmt_dup(s), NULL, cnt, 1), stmt_aggr(rows, NULL, sql_dup_aggr(cnt), 1), ne);

	/* s should be empty */
	msg = sql_message( "UPDATE: FOREIGN KEY constraint '%s.%s' violated", k->t->base.name, k->base.name);
	return stmt_exception(s, msg, 00001);
}

static stmt*
sql_delete_set_Fkeys(mvc *sql, sql_key *k, stmt *rows, int action)
{
	list *l = NULL;
	int len = 0;
	node *m, *o;
	sql_key *rk = &((sql_fkey*)k)->rkey->k;
	stmt *ts = stmt_basetable(rk->t, rk->t->base.name), *fts, **new_updates;
	sql_table *t = mvc_bind_table(sql, k->t->s, k->t->base.name);

	fts = stmt_basetable(k->idx->t, k->idx->t->base.name);

	new_updates = table_update_array(t, &len);
	for (m = k->idx->columns->h, o = rk->columns->h; m && o; m = m->next, o = o->next) {
		sql_kc *fc = m->data;
		stmt *upd = NULL;

		if (action == ACT_SET_DEFAULT) {
			if (fc->c->def) {
				stmt *sq;
				char *msg = sql_message( "select %s;", fc->c->def);
				sq = rel_parse_value(sql, msg, sql->emode);
				_DELETE(msg);
				if (!sq) 
					return NULL;
				upd = stmt_dup(sq);
			}  else {
				upd = stmt_atom(atom_general(&fc->c->type, NULL, 0));
			}
		} else {
			upd = stmt_atom(atom_general(&fc->c->type, NULL, 0));
		}
		
		if (!upd || (upd = check_types(sql, &fc->c->type, upd, type_equal)) == NULL) {
			stmt_destroy(rows);
			return NULL;
		}

		if (upd->nrcols <= 0) 
			upd = stmt_const(stmt_dup(rows), upd);
		
		new_updates[fc->c->colnr] = stmt_update_col(fc->c, upd);
	}

	stmt_destroy(rows);
	stmt_destroy(ts);
	stmt_destroy(fts);
	
	if ((l = sql_update(sql, t, new_updates, len)) == NULL) 
		return NULL;
	return stmt_list(l);
}

static stmt*
sql_update_cascade_Fkeys(mvc *sql, sql_key *k, int updcol, stmt **updates, int action)
{
	list *l = NULL;
	int len = 0;
	node *m, *o;
	sql_key *rk = &((sql_fkey*)k)->rkey->k;
	stmt *ts = stmt_basetable(rk->t, rk->t->base.name), *fts, **new_updates;
	stmt *rows;
	sql_table *t = mvc_bind_table(sql, k->t->s, k->t->base.name);

	fts = stmt_basetable(k->idx->t, k->idx->t->base.name);

	rows = stmt_idxbat(k->idx, RDONLY);
	rows = stmt_semijoin(stmt_reverse(rows), stmt_dup(updates[updcol]->op2.stval));
	rows = stmt_reverse(rows);
		
	new_updates = table_update_array(t, &len);
	for (m = k->idx->columns->h, o = rk->columns->h; m && o; m = m->next, o = o->next) {
		sql_kc *fc = m->data;
		sql_kc *c = o->data;
		stmt *upd = NULL;

		if (!updates[c->c->colnr]) {
			continue;
		} else if (action == ACT_CASCADE) {
			upd = stmt_dup(updates[c->c->colnr]->op2.stval);
		} else if (action == ACT_SET_DEFAULT) {
			if (fc->c->def) {
				stmt *sq;
				char *msg = sql_message( "select %s;", fc->c->def);
				sq = rel_parse_value(sql, msg, sql->emode);
				_DELETE(msg);
				if (!sq) 
					return NULL;
				upd = stmt_dup(sq);
			} else {
				upd = stmt_atom(atom_general(&fc->c->type, NULL, 0));
			}
		} else if (action == ACT_SET_NULL) {
			upd = stmt_atom(atom_general(&fc->c->type, NULL, 0));
		}

		if (!upd || (upd = check_types(sql, &fc->c->type, upd, type_equal)) == NULL) {
			stmt_destroy(rows);
			return NULL;
		}

		if (upd->nrcols <= 0) 
			upd = stmt_const(stmt_dup(rows), upd);
		else
			upd = stmt_join(stmt_dup(rows), upd, cmp_equal);
		
		new_updates[fc->c->colnr] = stmt_update_col(fc->c, upd);
	}

	stmt_destroy(rows);
	stmt_destroy(ts);
	stmt_destroy(fts);
	
	if ((l = sql_update(sql, t, new_updates, len)) == NULL) 
		return NULL;
	return stmt_list(l);
}


static void 
cascade_ukey(mvc *sql, stmt **updates, sql_key *k, int updcol, list *cascade) 
{
	sql_ukey *uk = (sql_ukey*)k;

	if (uk->keys && list_length(uk->keys) > 0) {
		node *n;
		for(n = uk->keys->h; n; n = n->next) {
			sql_key *fk = n->data;
			stmt *s = NULL;

			/* All rows of the foreign key table which are
			   affected by the primary key update should all
			   match one of the updated primary keys again.
			 */
			switch (((sql_fkey*)fk)->on_update) {
				case ACT_NO_ACTION: 
					break;
				case ACT_SET_NULL: 
				case ACT_SET_DEFAULT: 
				case ACT_CASCADE: 
					s = sql_update_cascade_Fkeys(sql, fk, updcol, updates, ((sql_fkey*)fk)->on_update);
					list_append(cascade, s);
					break;
				default:	/*RESTRICT*/
					s = join_updated_pkey(sql, fk, updates, updcol);
					list_append(cascade, s);
			}
		}
	}
}

static void
sql_update_check_key(mvc *sql, stmt **updates, sql_key *k, stmt *idx_updates, int updcol, list *l, list *cascade)
{
	stmt *ckeys;

	if (k->type == pkey || k->type == ukey) {
		ckeys = update_check_ukey(sql, updates, k, idx_updates, updcol);
		if (cascade)
			cascade_ukey(sql, updates, k, updcol, cascade);
	} else { /* foreign keys */
		ckeys = update_check_fkey(sql, updates, k, idx_updates, updcol);
	}
	list_append(l, ckeys);
}

static stmt *
hash_update(mvc *sql, sql_idx * i, stmt **updates, int updcol)
{
	/* calculate new value */
	node *m;
	sql_subtype *it, *wrd;
	int bits = 1 + ((sizeof(wrd)*8)-1)/(list_length(i->columns)+1);
	stmt *h = NULL, *ts, *o = NULL;

	if (list_length(i->columns) <= 1)
		return NULL;

	ts = stmt_basetable(i->t, i->t->base.name);
	it = sql_bind_localtype("int");
	wrd = sql_bind_localtype("wrd");
	for (m = i->columns->h; m; m = m->next ) {
		sql_kc *c = m->data;
		stmt *upd;

		if (updates && updates[c->c->colnr]) {
			upd = stmt_dup(updates[c->c->colnr]->op2.stval);
		} else if (updates && updcol >= 0) {
			upd = stmt_dup(updates[updcol]->op2.stval);
			upd = stmt_semijoin(stmt_bat(c->c, stmt_dup(ts), RDONLY), upd);
		} else { /* created idx/key using alter */ 
			upd = stmt_bat(c->c, stmt_dup(ts), RDONLY);
		}

		if (h && i->type == hash_idx)  { 
			sql_subfunc *xor = sql_bind_func_result3(sql->session->schema, "rotate_xor_hash", wrd, it, &c->c->type, wrd);

			h = stmt_Nop(stmt_list( list_append( list_append(
				list_append(create_stmt_list(), h), 
				stmt_atom_int(bits)), 
				stmt_join(stmt_dup(o), upd, cmp_equal))), 
				xor);
		} else if (h)  { 
			stmt *h2;
			sql_subfunc *lsh = sql_bind_func_result(sql->session->schema, "left_shift", wrd, it, wrd);
			sql_subfunc *lor = sql_bind_func_result(sql->session->schema, "bit_or", wrd, wrd, wrd);
			sql_subfunc *hf = sql_bind_func_result(sql->session->schema, "hash", &c->c->type, NULL, wrd);

			h = stmt_binop(h, stmt_atom_int(bits), lsh); 
			h2 = stmt_unop(
				stmt_join(stmt_dup(o), upd, cmp_equal), hf);
			h = stmt_binop(h, h2, lor);
		} else {
			sql_subfunc *hf = sql_bind_func_result(sql->session->schema, "hash", &c->c->type, NULL, wrd);
			o = stmt_mark(stmt_reverse(stmt_dup(upd)), 40); 
			h = stmt_unop(stmt_mark(upd, 40), hf);
			if (i->type == oph_idx)
				break;
		}
	}
	stmt_destroy(ts);
	return stmt_join(stmt_reverse(o), h, cmp_equal);
}

/*
         A referential constraint is satisfied if one of the following con-
         ditions is true, depending on the <match option> specified in the
         <referential constraint definition>:

         -  If no <match type> was specified then, for each row R1 of the
            referencing table, either at least one of the values of the
            referencing columns in R1 shall be a null value, or the value of
            each referencing column in R1 shall be equal to the value of the
            corresponding referenced column in some row of the referenced
            table.

         -  If MATCH FULL was specified then, for each row R1 of the refer-
            encing table, either the value of every referencing column in R1
            shall be a null value, or the value of every referencing column
            in R1 shall not be null and there shall be some row R2 of the
            referenced table such that the value of each referencing col-
            umn in R1 is equal to the value of the corresponding referenced
            column in R2.

         -  If MATCH PARTIAL was specified then, for each row R1 of the
            referencing table, there shall be some row R2 of the refer-
            enced table such that the value of each referencing column in
            R1 is either null or is equal to the value of the corresponding
            referenced column in R2.
*/

static stmt *
join_idx_update(sql_idx * i, stmt **updates, int updcol)
{
	int nulls = 0, len, j;
	node *m, *o;
	sql_key *rk = &((sql_fkey *) i->key)->rkey->k;
	stmt *s = NULL, *rts = stmt_basetable(rk->t, rk->t->base.name), *ts;
	stmt *null = NULL, *nnull = NULL;
	stmt **new_updates = table_update_array(i->t, &len);
	sql_column *updcolumn = NULL; 

	ts = stmt_basetable(i->t, i->t->base.name);
	for (m = i->columns->h, o = rk->columns->h; m && o; m = m->next, o = o->next) {
		sql_kc *c = m->data;
		stmt *upd;

		if (updates && updates[c->c->colnr]) {
			upd = stmt_dup(updates[c->c->colnr]->op2.stval);
		} else if (updates && updcol >= 0) {
			upd = stmt_dup(updates[updcol]->op2.stval);
			upd = stmt_semijoin(stmt_bat(c->c, stmt_dup(ts), RDONLY), upd);
		} else { /* created idx/key using alter */ 
			upd = stmt_bat(c->c, stmt_dup(ts), RDONLY);
			updcolumn = c->c;
		}
		new_updates[c->c->colnr] = upd;

		/* FOR MATCH FULL/SIMPLE/PARTIAL see above */
		/* Currently only the default MATCH SIMPLE is supported */
		if (c->c->null) {
			stmt *nn = stmt_dup(upd);

			nn = stmt_uselect(nn, stmt_atom(atom_general(&c->c->type, NULL, 0)), cmp_equal);
			if (null)
				null = stmt_union(null, nn);
			else
				null = nn;
			nulls = 1;
		}

	}

	/* we only need to check non null values */
	if (nulls && updates) 
		/* convert nulls to table ids */
		nnull = stmt_diff(stmt_dup(updates[updcol]->op2.stval), stmt_dup(null));
	else if (nulls) /* no updates (only new idx/key) */
		nnull = stmt_diff(stmt_bat(updcolumn, stmt_dup(ts), RDONLY), stmt_dup(null));


	s = stmt_releqjoin_init();
	for (m = i->columns->h, o = rk->columns->h; m && o; m = m->next, o = o->next) {
		sql_kc *c = m->data;
		sql_kc *rc = o->data;
		stmt *upd = stmt_dup(new_updates[c->c->colnr]);

		if (nulls) /* remove nulls */
			upd = stmt_semijoin(upd, stmt_dup(nnull)); 
		stmt_releqjoin_fill(s, upd, stmt_dup(stmt_bat(rc->c, stmt_dup(rts), RDONLY)));
	}
	/* add missing nulls */
	if (nulls)
		s = stmt_union(s, stmt_const(null, stmt_atom(atom_general(sql_bind_localtype("oid"), NULL, 0))));

	stmt_destroy(ts);
	stmt_destroy(rts);
	for (j = 0; j < len; j++) 
		if (new_updates[j])
			stmt_destroy(new_updates[j]);
	_DELETE(new_updates);
	return s;
}

static list *
create_idxs_and_check_keys(mvc *sql, sql_table *t, list *l)
{
	node *n;
	list *idx_updates = create_stmt_list();

	if (!t->idxs.nelm)
		return idx_updates;

	for (n = t->idxs.nelm; n; n = n->next) {
		sql_idx *i = n->data;
		stmt *is = NULL;

		if (hash_index(i->type)) {
			is = hash_update(sql, i, NULL, -1);
		} else if (i->type == join_idx) {
			is = join_idx_update(i, NULL, -1);
		}
		if (i->key) 
			sql_update_check_key(sql, NULL, i->key, is, -1, l, NULL);
		if (is) 
			list_append(idx_updates, stmt_update_idx(i, is));
	}
	return idx_updates;
}

static list *
update_idxs_and_check_keys(mvc *sql, sql_table *t, stmt **updates, list *l, list **cascades)
{
	node *n;
	int updcol;
	list *idx_updates = create_stmt_list();

	*cascades = create_stmt_list();
	if (!t->idxs.set)
		return idx_updates;

	updcol = first_updated_col(updates, list_length(t->columns.set));
	for (n = t->idxs.set->h; n; n = n->next) {
		sql_idx *i = n->data;
		stmt *is = NULL;

		/* check if update is needed, 
		 * ie atleast on of the idx columns is updated 
		 */
		if (is_idx_updated(i, updates) == 0)
			continue;

		if (hash_index(i->type)) {
			is = hash_update(sql, i, updates, updcol);
		} else if (i->type == join_idx) {
			is = join_idx_update(i, updates, updcol);
		}
		if (i->key) {
			if (!(sql->cascade_action && list_find_id(sql->cascade_action, i->key->base.id))) {
				int *local_id = GDKmalloc(sizeof(int));
				if (!sql->cascade_action) 
					sql->cascade_action = list_create((fdestroy) GDKfree);
				
				*local_id = i->key->base.id;
				list_append(sql->cascade_action, local_id);
				sql_update_check_key(sql, updates, i->key, is, updcol, l, *cascades);
			}
		}
		if (is) 
			list_append(idx_updates, stmt_update_idx(i, is));
	}
	return idx_updates;
}

static void
sql_stack_add_updated(mvc *sql, char *on, char *nn, sql_table *t)
{
	sql_rel *or = rel_basetable(t, on );
	sql_rel *nr = rel_basetable(t, nn );
		
	stack_push_rel_view(sql, on, or);
	stack_push_rel_view(sql, nn, nr);
}

static int
sql_update_triggers(mvc *sql, sql_table *t, list *l, int time )
{
	node *n;
	int res = 1;

	if (!t->triggers.set)
		return res;

	for (n = t->triggers.set->h; n; n = n->next) {
		sql_trigger *trigger = n->data;
		int *trigger_id = NEW(int);
		*trigger_id = trigger->base.id;

		if (trigger->event == 2 && trigger->time == time) {
			stmt *s = NULL;
	
			/* add name for the 'inserted' to the stack */
			char *n = trigger->new_name;
			char *o = trigger->old_name;
	
			if (!n) n = "new"; 
			if (!o) o = "old"; 
	
			sql_stack_add_updated(sql, o, n, t);
			s = sql_parse(sql, NULL, trigger->statement, m_instantiate);

			if (!s) 
				return 0;
			list_append(l, s);
		}
	}
	return res;
}


static list *
sql_update(mvc *sql, sql_table *t, stmt **updates, int len)
{
	node *n;
	sql_subaggr *cnt;
	list *l = create_stmt_list();
	list *idx_updates = NULL, *cascades = NULL;
	int i, nr_cols = list_length(t->columns.set);

 	cnt = sql_bind_aggr(sql->session->schema, "count", NULL);
	for (n = t->columns.set->h; n; n = n->next) {
		sql_column *c = n->data;

		if (updates[c->colnr] && !c->null) {
			stmt *s = updates[c->colnr]->op2.stval;
			char *msg = NULL;

			if (!(s->key && s->nrcols == 0)) {
				s = stmt_atom(atom_general(&c->type, NULL, 0));
				s = stmt_uselect(stmt_dup(updates[c->colnr]->op2.stval), s, cmp_equal);
				s = stmt_aggr(s, NULL, sql_dup_aggr(cnt), 1);
			} else {
				sql_subfunc *isnil = sql_bind_func(sql->session->schema, "isnull", &c->type, NULL);

				s = stmt_unop(stmt_dup(updates[c->colnr]->op2.stval), isnil);
			}
			msg = sql_message( "UPDATE: NOT NULL constraint violated for column '%s.%s'", c->t->base.name, c->base.name);
			s = stmt_exception(s, msg, 00001);

			list_append(l, s);
		}
	}
	sql_subaggr_destroy(cnt);

	/* check keys + get idx */
	idx_updates = update_idxs_and_check_keys(sql, t, updates, l, &cascades);
	if (!idx_updates) {
		list_destroy(l);
		return sql_error(sql, 02, "UPDATE: failed to update indexes for table '%s'", t->base.name);
	}

/* before */
	if (!sql_update_triggers(sql, t, l, 0)) {
		list_destroy(l);
		list_destroy(idx_updates);
		list_destroy(cascades);
		return sql_error(sql, 02, "UPDATE: triggers failed for table '%s'", t->base.name);
	}

/* apply updates */
	list_merge(l, idx_updates, (fdup)&stmt_dup);
	list_destroy(idx_updates);
	for (i = 0; i < nr_cols; i++) 
		if (updates[i])
			list_append(l, stmt_dup(updates[i]));

/* after */
	if (!sql_update_triggers(sql, t, l, 1)) {
		list_destroy(l);
		list_destroy(cascades);
		return sql_error(sql, 02, "UPDATE: triggers failed for table '%s'", t->base.name);
	}

/* cascade */
	list_merge(l, cascades, (fdup)&stmt_dup);
	list_destroy(cascades);
	for (i = 0; i < len; i++) 
		if (updates[i])
			stmt_destroy(updates[i]);
	_DELETE(updates);
	return l;
}

/* updates with empty list is alter with create idx or keys */
static stmt *
rel2bin_update( mvc *sql, sql_rel *rel, list *refs)
{
	stmt *update = NULL, **updates = NULL, *tid, *s, *ddl = NULL;
	list *l;
	int len;
	node *n, *m;
	sql_rel *tr = rel->l;
	sql_table *t = NULL;

	if (tr->op == op_basetable) {
		t = tr->l;
	} else {
		list *idx_updates;
		assert(tr->flag == DDL_ALTER_TABLE);

		ddl = subrel_bin(sql, rel->l, refs);
		if (!ddl)
			return NULL;

		t = rel_alter_table_get(tr);
		if (!rel->r) {
 			l = create_stmt_list();
			list_append(l, ddl);

			/* check keys + get idx */
			idx_updates = create_idxs_and_check_keys(sql, t, l);
			if (!idx_updates) {
				list_destroy(l);
				return sql_error(sql, 02, "UPDATE: failed to update indexes for table '%s'", t->base.name);
			}
			list_append(l, stmt_list(idx_updates));
			return stmt_list(l);
		}
	}

	if (rel->r) /* first construct the update relation */
		update = subrel_bin(sql, rel->r, refs);
	if (!update)
		return NULL;	
	updates = table_update_array(t, &len);
	tid = update->op1.lval->h->data;
	for (n = update->op1.lval->h->next, m = rel->exps->h; 
		n && m; n = n->next, m = m->next) {
		stmt *s = n->data;
		sql_exp *ce = m->data;
		sql_column *c = find_sql_column(t, ce->name);
		
		s = stmt_join(stmt_reverse(stmt_dup(tid)), stmt_dup(s), cmp_equal);
		updates[c->colnr] = stmt_update_col( c, s);
	}
	l = sql_update(sql, t, updates, len);
	if (ddl) {
		list_prepend(l, ddl);
	} else {
		s = stmt_aggr(stmt_dup(tid), NULL, sql_bind_aggr(sql->session->schema, "count", NULL), 1);
		list_append(l, stmt_affected_rows(s));
	}

	if (sql->cascade_action) {
		list_destroy(sql->cascade_action);
		sql->cascade_action = NULL;
	}
	return stmt_list(l);
}
 
static void
sql_stack_add_deleted(mvc *sql, char *name, sql_table *t)
{
	sql_rel *r = rel_basetable(t, name );
		
	stack_push_rel_view(sql, name, r);
}

static int
sql_delete_triggers(mvc *sql, sql_table *t, list *l)
{
	node *n;
	int res = 1;

	if (!t->triggers.set)
		return res;

	for (n = t->triggers.set->h; n; n = n->next) {
		sql_trigger *trigger = n->data;
		int *trigger_id = NEW(int);
		*trigger_id = trigger->base.id;

		if (trigger->event == 1) {
			stmt *s = NULL;
	
			/* add name for the 'deleted' to the stack */
			char *o = trigger->old_name;
		
			if (!o) o = "old"; 
		
			sql_stack_add_deleted(sql, o, t);
			s = sql_parse(sql, NULL, trigger->statement, m_instantiate);

			if (!s) 
				return 0;
			if (trigger -> time )
				list_append(l, s);
			else
				list_prepend(l, s);
		}
	}
	return res;
}

static stmt * sql_delete(mvc *sql, sql_table *t, stmt *delete);

static stmt *
sql_delete_cascade_Fkeys(mvc *sql, stmt *s, sql_key *fk)
{
	sql_table *t = mvc_bind_table(sql, fk->t->s, fk->t->base.name);
	return sql_delete(sql, t, s);
}

static void 
sql_delete_ukey(mvc *sql, stmt *deletes, sql_key *k, list *l) 
{
	sql_ukey *uk = (sql_ukey*)k;

	if (uk->keys && list_length(uk->keys) > 0) {
		sql_subtype *wrd = sql_bind_localtype("wrd");
		sql_subtype *bt = sql_bind_localtype("bit");
		node *n;
		for(n = uk->keys->h; n; n = n->next) {
			char *msg = NULL;
			sql_subaggr *cnt = sql_bind_aggr(sql->session->schema, "count", NULL);
			sql_subfunc *ne = sql_bind_func_result(sql->session->schema, "<>", wrd, wrd, bt);
			sql_key *fk = n->data;
			stmt *s;

			s = stmt_idxbat(fk->idx, RDONLY);
			s = stmt_semijoin(stmt_reverse(s), stmt_dup(deletes));
			switch (((sql_fkey*)fk)->on_delete) {
				case ACT_NO_ACTION: 
					break;
				case ACT_SET_NULL: 
				case ACT_SET_DEFAULT: 
					s = stmt_reverse(s);
					s = sql_delete_set_Fkeys(sql, fk, s, ((sql_fkey*)fk)->on_delete);
            			        list_prepend(l, s);
					break;
				case ACT_CASCADE: 
					s = sql_delete_cascade_Fkeys(sql, s, fk);
            			        list_prepend(l, s);
					break;
				default:	/*RESTRICT*/
					/* The overlap between deleted primaries and foreign should be empty */
		                        s = stmt_binop(stmt_aggr(s, NULL, cnt, 1), stmt_atom_wrd(0), ne);
					msg = sql_message( "DELETE: FOREIGN KEY constraint '%s.%s' violated", fk->t->base.name, fk->base.name);
		                        s = stmt_exception(s, msg, 00001);
            			        list_prepend(l, s);
			}
		}
	}
}

static int
sql_delete_keys(mvc *sql, sql_table *t, stmt *deletes, list *l)
{
	int res = 1;
	node *n;

	if (!t->keys.set)
		return res;

	for (n = t->keys.set->h; n; n = n->next) {
		sql_key *k = n->data;

		if (k->type == pkey || k->type == ukey) {
			if (!(sql->cascade_action && list_find_id(sql->cascade_action, k->base.id))) {
				int *local_id = GDKmalloc(sizeof(int));
				if (!sql->cascade_action) 
					sql->cascade_action = list_create((fdestroy) GDKfree);
				
				*local_id = k->base.id;
				list_append(sql->cascade_action, local_id); 
				sql_delete_ukey(sql, deletes, k, l);
			}
		}
	}
	return res;
}

static stmt * 
sql_delete(mvc *sql, sql_table *t, stmt *delete)
{
	stmt *v, *s = NULL;
	list *l = create_stmt_list();

	if (delete) { 
		sql_subtype to;

		sql_find_subtype(&to, "oid", 0, 0);
		v = stmt_const(stmt_reverse(delete), stmt_atom(atom_general(&to, NULL, 0)));
		list_append(l, stmt_delete(t, stmt_reverse(v)));
	} else { /* delete all */
		/* first column */
		v = stmt_mirror(stmt_bat(t->columns.set->h->data, stmt_basetable(t, t->base.name), RDONLY));
		s = stmt_table_clear(t);
		list_append(l, stmt_dup(s));
	}

	if (!sql_delete_triggers(sql, t, l)) 
		return sql_error(sql, 02, "DELETE: triggers failed for table '%s'", t->base.name);
	if (!sql_delete_keys(sql, t, v, l)) 
		return sql_error(sql, 02, "DELETE: failed to delete indexes for table '%s'", t->base.name);
	if (delete) 
		s = stmt_aggr(stmt_dup(delete), NULL, sql_bind_aggr(sql->session->schema, "count", NULL), 1);
	list_append(l, stmt_affected_rows(s));
	return stmt_list(l);
}

static stmt *
rel2bin_delete( mvc *sql, sql_rel *rel, list *refs)
{
	stmt *delete = NULL;
	sql_rel *tr = rel->l;
	sql_table *t = NULL;

	if (tr->op == op_basetable)
		t = tr->l;
	else
		assert(0/*ddl statement*/);

	if (rel->r) { /* first construct the deletes relation */
		delete = subrel_bin(sql, rel->r, refs);
		if (!delete) 
			return NULL;	
	}
	if (delete && delete->type == st_list) {
		stmt *s = delete;
		delete = stmt_dup(s->op1.lval->h->data);
		stmt_destroy(s);
	}
	delete = sql_delete(sql, t, delete); 
	if (sql->cascade_action) {
		list_destroy(sql->cascade_action);
		sql->cascade_action = NULL;
	}
	return delete;
}

static stmt *
refs_find_rel(list *refs, sql_rel *rel)
{
	node *n;

	for(n=refs->h; n; n = n->next->next) {
		sql_rel *ref = n->data;
		stmt *s = n->next->data;
		
		if (rel == ref) 
			return stmt_dup(s);
	}
	return NULL;
}

#define E_ATOM_INT(e) ((atom*)((sql_exp*)e)->l)->data.val.lval
#define E_ATOM_STRING(e) ((atom*)((sql_exp*)e)->l)->data.val.sval

static stmt *
rel2bin_output(mvc *sql, sql_rel *rel, list *refs) 
{
	node *n = rel->exps->h;
	char *tsep = _strdup(E_ATOM_STRING(n->data));
	char *rsep = _strdup(E_ATOM_STRING(n->next->data));
	char *ssep = _strdup(E_ATOM_STRING(n->next->next->data));
	char *ns   = _strdup(E_ATOM_STRING(n->next->next->next->data));
	char *fn   = NULL;
	stmt *s = NULL, *fns = NULL;
	list *slist = create_stmt_list();

	if (rel->l)  /* first construct the sub relation */
		s = subrel_bin(sql, rel->l, refs);
	if (!s) 
		return NULL;	

	if (n->next->next->next->next) {
		fn = E_ATOM_STRING(n->next->next->next->next->data);
		fns = stmt_atom_string(_strdup(fn));
	}
	list_append(slist, stmt_export(s, tsep, rsep, ssep, ns, fns));
	if (s->type == st_list && ((stmt*)s->op1.lval->h->data)->nrcols != 0) {
		stmt *cnt = stmt_aggr(stmt_dup(s->op1.lval->h->data), NULL, sql_bind_aggr(sql->session->schema, "count", NULL), 1);
		list_append(slist, stmt_affected_rows(cnt));
	} else {
		list_append(slist, stmt_affected_rows(stmt_atom_wrd(1)));
	}
	return stmt_list(slist);
}

static stmt *
rel2bin_list(mvc *sql, sql_rel *rel, list *refs) 
{
	stmt *l = NULL, *r = NULL;
	list *slist = create_stmt_list();

	(void)refs;
	if (rel->l)  /* first construct the sub relation */
		l = subrel_bin(sql, rel->l, refs);
	if (rel->r)  /* first construct the sub relation */
		r = subrel_bin(sql, rel->r, refs);
	if (!l || !r)
		return NULL;
	list_append(slist, l);
	list_append(slist, r);
	return stmt_list(slist);
}

static stmt *
rel2bin_seq(mvc *sql, sql_rel *rel, list *refs) 
{
	node *en = rel->exps->h;
	stmt *restart, *sname, *seq, *l = NULL;

	if (rel->l)  /* first construct the sub relation */
		l = subrel_bin(sql, rel->l, refs);

	restart = exp_bin(sql, en->data, l, NULL, NULL, NULL);
	sname = exp_bin(sql, en->next->data, l, NULL, NULL, NULL);
	seq = exp_bin(sql, en->next->next->data, l, NULL, NULL, NULL);

	(void)refs;
	return stmt_catalog(rel->flag, sname, seq, restart);
}

static stmt *
rel2bin_drop_seq(mvc *sql, sql_rel *rel, list *refs) 
{
	node *en = rel->exps->h;
	stmt *action = exp_bin(sql, en->data, NULL, NULL, NULL, NULL);
	stmt *sname = exp_bin(sql, en->next->data, NULL, NULL, NULL, NULL);
	stmt *name = exp_bin(sql, en->next->next->data, NULL, NULL, NULL, NULL);

	(void)refs;
	return stmt_catalog(rel->flag, sname, name, action);
}

static stmt *
rel2bin_trans(mvc *sql, sql_rel *rel, list *refs) 
{
	node *en = rel->exps->h;
	stmt *chain = exp_bin(sql, en->data, NULL, NULL, NULL, NULL);
	stmt *name = NULL;

	(void)refs;
	if (en->next)
		name = exp_bin(sql, en->next->data, NULL, NULL, NULL, NULL);
	return stmt_trans(rel->flag, chain, name);
}

static stmt *
rel2bin_catalog(mvc *sql, sql_rel *rel, list *refs) 
{
	node *en = rel->exps->h;
	stmt *action = exp_bin(sql, en->data, NULL, NULL, NULL, NULL);
	stmt *sname = NULL, *name = NULL;

	(void)refs;
	en = en->next;
	sname = exp_bin(sql, en->data, NULL, NULL, NULL, NULL);
	if (en->next) 
		name = exp_bin(sql, en->next->data, NULL, NULL, NULL, NULL);
	return stmt_catalog(rel->flag, sname, name, action);
}

static stmt *
rel2bin_catalog_table(mvc *sql, sql_rel *rel, list *refs) 
{
	node *en = rel->exps->h;
	stmt *action = exp_bin(sql, en->data, NULL, NULL, NULL, NULL);
	stmt *table = NULL, *sname;

	(void)refs;
	en = en->next;
	sname = exp_bin(sql, en->data, NULL, NULL, NULL, NULL);
	en = en->next;
	if (en) 
		table = exp_bin(sql, en->data, NULL, NULL, NULL, NULL);
	return stmt_catalog(rel->flag, sname, table, action);
}

static stmt *
rel2bin_ddl(mvc *sql, sql_rel *rel, list *refs) 
{
	stmt *s = NULL;

	if (rel->flag == DDL_OUTPUT) {
		s = rel2bin_output(sql, rel, refs);
		sql->type = Q_TABLE;
	} else if (rel->flag <= DDL_LIST) {
		s = rel2bin_list(sql, rel, refs);
	} else if (rel->flag <= DDL_ALTER_SEQ) {
		s = rel2bin_seq(sql, rel, refs);
		sql->type = Q_SCHEMA;
	} else if (rel->flag <= DDL_DROP_SEQ) {
		s = rel2bin_drop_seq(sql, rel, refs);
		sql->type = Q_SCHEMA;
	} else if (rel->flag <= DDL_TRANS) {
		s = rel2bin_trans(sql, rel, refs);
		sql->type = Q_TRANS;
	} else if (rel->flag <= DDL_DROP_SCHEMA) {
		s = rel2bin_catalog(sql, rel, refs);
		sql->type = Q_SCHEMA;
	} else if (rel->flag <= DDL_ALTER_TABLE) {
		s = rel2bin_catalog_table(sql, rel, refs);
		sql->type = Q_SCHEMA;
	}
	return s;
}

static stmt *
subrel_bin(mvc *sql, sql_rel *rel, list *refs) 
{
	stmt *s = NULL;

	if (THRhighwater())
		return NULL;

	if (!rel)
		return s;
	if (rel_is_ref(rel)) {
		s = refs_find_rel(refs, rel);
		/* needs a proper fix!! */
		if (s)
			return s;
	}
	switch (rel->op) {
	case op_basetable:
		s = rel2bin_basetable(sql, rel, refs);
		sql->type = Q_TABLE;
		break;
	case op_table:
		s = rel2bin_table(sql, rel, refs);
		sql->type = Q_TABLE;
		break;
	case op_join: 
	case op_left: 
	case op_right: 
	case op_full: 
		s = rel2bin_join(sql, rel, refs);
		sql->type = Q_TABLE;
		break;
	case op_semi:
	case op_anti:
		s = rel2bin_semijoin(sql, rel, refs);
		sql->type = Q_TABLE;
		break;
	case op_union: 
		s = rel2bin_union(sql, rel, refs);
		sql->type = Q_TABLE;
		break;
	case op_except: 
		s = rel2bin_except(sql, rel, refs);
		sql->type = Q_TABLE;
		break;
	case op_inter: 
		s = rel2bin_inter(sql, rel, refs);
		sql->type = Q_TABLE;
		break;
	case op_project:
		s = rel2bin_project(sql, rel, refs, NULL);
		sql->type = Q_TABLE;
		break;
	case op_select: 
		s = rel2bin_select(sql, rel, refs);
		sql->type = Q_TABLE;
		break;
	case op_groupby: 
		s = rel2bin_groupby(sql, rel, refs);
		sql->type = Q_TABLE;
		break;
	case op_topn: 
		s = rel2bin_topn(sql, rel, refs);
		sql->type = Q_TABLE;
		break;
	case op_insert: 
		s = rel2bin_insert(sql, rel, refs);
		if (sql->type == Q_TABLE)
			sql->type = Q_UPDATE;
		break;
	case op_update: 
		s = rel2bin_update(sql, rel, refs);
		if (sql->type == Q_TABLE)
			sql->type = Q_UPDATE;
		break;
	case op_delete: 
		s = rel2bin_delete(sql, rel, refs);
		if (sql->type == Q_TABLE)
			sql->type = Q_UPDATE;
		break;
	case op_ddl:
		s = rel2bin_ddl(sql, rel, refs);
		break;
	}
	if (s && rel_is_ref(rel)) {
		list_append(refs, rel);
		list_append(refs, stmt_dup(s));
	}
	return s;
}

stmt *
rel_bin(mvc *sql, sql_rel *rel) 
{
	node *n;
	list *refs = list_create(NULL);
	int sqltype = sql->type;
	stmt *s = subrel_bin( sql, rel, refs);

	if (sqltype == Q_SCHEMA)
		sql->type = sqltype;  /* reset */

	/* clean stmts properly (but don't touch the rels) ! */
	for (n = refs->h; n; n = n->next->next) 
		stmt_destroy(n->next->data);
	list_destroy(refs);

	if (s && s->type == st_list) {
		stmt *cnt = s->op1.lval->t->data;
		if (cnt && cnt->type == st_affected_rows)
			list_remove_data(s->op1.lval, cnt);
	}
	return s;
}

stmt *
output_rel_bin(mvc *sql, sql_rel *rel ) 
{
	list *refs = list_create(NULL);
	int sqltype = sql->type;
	stmt *s = subrel_bin( sql, rel, refs);

	if (sqltype == Q_SCHEMA)
		sql->type = sqltype;  /* reset */

	if (!is_ddl(rel->op) && s && s->type != st_none && sql->type == Q_TABLE)
		s = stmt_output(s);
	list_destroy(refs);
	return s;
}
