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
#include "jaqlgencode.h"
#include "opt_prelude.h"

typedef struct _json_var {
	const char *name;
	char preserve;
	int j1;
	int j2;
	int j3;
	int j4;
	int j5;
	int j6;
	int j7;
	int ids;
} json_var;

typedef struct _join_result {
	int bat;
	json_var *headvar;
	json_var *tailvar;
	struct _join_result *next;
} join_result;

static int dumpvariabletransformation(MalBlkPtr mb, tree *t, int elems, int *j1, int *j2, int *j3, int *j4, int *j5, int *j6, int *j7);
static int dumpnextid(MalBlkPtr mb, int j1);

/* returns a bat with subset from kind bat (:oid,:chr) which are
 * referenced by the first array of the JSON structure (oid 0@0 of kind
 * bat, pointing to array, so all oids from array bat that have head oid
 * value 0@0) */
static int
dumpwalkvar(MalBlkPtr mb, int j1, int j5)
{
	InstrPtr q;
	int a;

	q = newInstruction(mb, ASSIGNsymbol);
	setModuleId(q, algebraRef);
	setFunctionId(q, putName("selectH", 7));
	q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
	q = pushArgument(mb, q, j1);
	q = pushOid(mb, q, 0);
	a = getArg(q, 0);
	pushInstruction(mb, q);
	q = newInstruction(mb, ASSIGNsymbol);
	setModuleId(q, algebraRef);
	setFunctionId(q, semijoinRef);
	q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
	q = pushArgument(mb, q, j5);
	q = pushArgument(mb, q, a);
	a = getArg(q, 0);
	pushInstruction(mb, q);
	q = newInstruction(mb, ASSIGNsymbol);
	setModuleId(q, batRef);
	setFunctionId(q, reverseRef);
	q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
	q = pushArgument(mb, q, a);
	a = getArg(q, 0);
	pushInstruction(mb, q);
	q = newInstruction(mb, ASSIGNsymbol);
	setModuleId(q, algebraRef);
	setFunctionId(q, semijoinRef);
	q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
	q = pushArgument(mb, q, j1);
	q = pushArgument(mb, q, a);
	a = getArg(q, 0);
	pushInstruction(mb, q);
	return a;
}

/* returns a bat with in the head the oid of the values (kind bat), and
 * in the tail, the oid of the corresponding element from elems
 * (typically array bat, head oid 0@0) */
static int
dumprefvar(MalBlkPtr mb, tree *t, int elems, int j1, int j6, int j7)
{
	InstrPtr q;
	int a, b;

	assert(t && t->type == j_var);

	q = newInstruction(mb, ASSIGNsymbol);
	setModuleId(q, batRef);
	setFunctionId(q, mirrorRef);
	q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
	q = pushArgument(mb, q, elems);
	b = getArg(q, 0);
	pushInstruction(mb, q);

	if (t->tval1 == NULL) /* just var, has no derefs, so all */
		return b;

	a = elems;
	for (t = t->tval1; t != NULL; t = t->tval1) {
		if (a != elems) { /* retrieve kinds on multiple indirections */
			q = newInstruction(mb, ASSIGNsymbol);
			setModuleId(q, algebraRef);
			setFunctionId(q, semijoinRef);
			q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
			q = pushArgument(mb, q, j1);
			q = pushArgument(mb, q, b);
			a = getArg(q, 0);
			pushInstruction(mb, q);
		}
		q = newInstruction(mb, ASSIGNsymbol);
		setModuleId(q, algebraRef);
		setFunctionId(q, uselectRef);
		q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
		q = pushArgument(mb, q, a);
		q = pushChr(mb, q, 'o');  /* deref requires object */
		a = getArg(q, 0);
		pushInstruction(mb, q);
		q = newInstruction(mb, ASSIGNsymbol);
		setModuleId(q, algebraRef);
		setFunctionId(q, semijoinRef);
		q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
		q = pushArgument(mb, q, j6);
		q = pushArgument(mb, q, a);
		a = getArg(q, 0);
		pushInstruction(mb, q);
		q = newInstruction(mb, ASSIGNsymbol);
		setModuleId(q, batRef);
		setFunctionId(q, reverseRef);
		q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
		q = pushArgument(mb, q, a);
		a = getArg(q, 0);
		pushInstruction(mb, q);
		q = newInstruction(mb, ASSIGNsymbol);
		setModuleId(q, algebraRef);
		setFunctionId(q, joinRef);
		q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
		q = pushArgument(mb, q, a);
		q = pushArgument(mb, q, b);
		b = getArg(q, 0);
		pushInstruction(mb, q);
		q = newInstruction(mb, ASSIGNsymbol);
		setModuleId(q, algebraRef);
		setFunctionId(q, semijoinRef);
		q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
		q = pushArgument(mb, q, j7);
		q = pushArgument(mb, q, a);
		a = getArg(q, 0);
		pushInstruction(mb, q);
		q = newInstruction(mb, ASSIGNsymbol);
		setModuleId(q, algebraRef);
		setFunctionId(q, uselectRef);
		q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
		q = pushArgument(mb, q, a);
		q = pushStr(mb, q, t->sval);
		a = getArg(q, 0);
		pushInstruction(mb, q);
		q = newInstruction(mb, ASSIGNsymbol);
		setModuleId(q, algebraRef);
		setFunctionId(q, semijoinRef);
		q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
		q = pushArgument(mb, q, b);
		q = pushArgument(mb, q, a);
		b = getArg(q, 0);
		pushInstruction(mb, q);
	}
	return b;
}

/* returns bat with in the head the oids from elems that match the
 * comparison */
static int
dumpcomp(MalBlkPtr mb, tree *t, int elems, int *j1, int *j2, int *j3, int *j4, int *j5, int *j6, int *j7)
{
	InstrPtr q;
	int a, b, c, d, e, f, g;

	assert(t != NULL);
	assert(t->tval1->type == j_var || t->tval1->type == j_operation);
	assert(t->tval2->type == j_comp);
	assert(t->tval3->type == j_var || t->tval3->type == j_operation
			|| t->tval3->type == j_num || t->tval3->type == j_dbl
			|| t->tval3->type == j_str || t->tval3->type == j_bool);

	if (t->tval1->type == j_operation) {
		a = dumpvariabletransformation(mb, t->tval1, elems,
				j1, j2, j3, j4, j5, j6, j7);
	} else {
		a = dumprefvar(mb, t->tval1, elems, *j1, *j6, *j7);
	}

	if (t->tval3->type != j_var && t->tval3->type != j_operation) {
		q = newInstruction(mb, ASSIGNsymbol);
		setModuleId(q, batRef);
		setFunctionId(q, reverseRef);
		q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
		q = pushArgument(mb, q, a);
		a = getArg(q, 0);
		pushInstruction(mb, q);
	}

	switch (t->tval3->type) {
		case j_var:
			b = dumprefvar(mb, t->tval3, elems, *j1, *j6, *j7);
			c = -1;
			break;
		case j_operation:
			b = dumpvariabletransformation(mb, t->tval3, elems,
					j1, j2, j3, j4, j5, j6, j7);
			c = -1;
			break;
		case j_num:
			q = newInstruction(mb, ASSIGNsymbol);
			setModuleId(q, algebraRef);
			setFunctionId(q, joinRef);
			q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
			q = pushArgument(mb, q, a);
			q = pushArgument(mb, q, *j3);
			b = getArg(q, 0);
			pushInstruction(mb, q);
			q = newInstruction(mb, ASSIGNsymbol);
			q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
			q = pushLng(mb, q, t->tval3->nval);
			c = getArg(q, 0);
			pushInstruction(mb, q);
			break;
		case j_dbl:
			q = newInstruction(mb, ASSIGNsymbol);
			setModuleId(q, algebraRef);
			setFunctionId(q, joinRef);
			q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
			q = pushArgument(mb, q, a);
			q = pushArgument(mb, q, *j4);
			b = getArg(q, 0);
			pushInstruction(mb, q);
			q = newInstruction(mb, ASSIGNsymbol);
			q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
			q = pushDbl(mb, q, t->tval3->dval);
			c = getArg(q, 0);
			pushInstruction(mb, q);
			break;
		case j_str:
			q = newInstruction(mb, ASSIGNsymbol);
			setModuleId(q, algebraRef);
			setFunctionId(q, joinRef);
			q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
			q = pushArgument(mb, q, a);
			q = pushArgument(mb, q, *j2);
			b = getArg(q, 0);
			pushInstruction(mb, q);
			q = newInstruction(mb, ASSIGNsymbol);
			q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
			q = pushStr(mb, q, t->tval3->sval);
			c = getArg(q, 0);
			pushInstruction(mb, q);
			break;
		case j_bool:
			q = newInstruction(mb, ASSIGNsymbol);
			setModuleId(q, algebraRef);
			setFunctionId(q, joinRef);
			q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
			q = pushArgument(mb, q, a);
			q = pushArgument(mb, q, *j1);
			b = getArg(q, 0);
			pushInstruction(mb, q);
			q = newInstruction(mb, ASSIGNsymbol);
			setModuleId(q, algebraRef);
			setFunctionId(q, uselectRef);
			q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
			q = pushArgument(mb, q, b);
			/* boolean conditions can only be j_equals or j_nequal */
			if (t->tval2->cval == j_equals) {
				q = pushChr(mb, q, t->tval3->nval == 0 ? 'f' : 't');
			} else {
				q = pushChr(mb, q, t->tval3->nval != 0 ? 'f' : 't');
			}
			b = getArg(q, 0);
			pushInstruction(mb, q);
			return b;
		default:
			assert(0);
	}
	if (c >= 0) {
		switch (t->tval2->cval) {
			case j_equals:
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, uselectRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, b);
				q = pushArgument(mb, q, c);
				g = getArg(q, 0);
				pushInstruction(mb, q);
				break;
			case j_nequal:
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, uselectRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, b);
				q = pushArgument(mb, q, c);
				c = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, putName("kdifference", 11));
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, b);
				q = pushArgument(mb, q, c);
				g = getArg(q, 0);
				pushInstruction(mb, q);
				break;
			case j_greater:
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, thetauselectRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, b);
				q = pushArgument(mb, q, c);
				q = pushStr(mb, q, ">");
				g = getArg(q, 0);
				pushInstruction(mb, q);
				break;
			case j_gequal:
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, thetauselectRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, b);
				q = pushArgument(mb, q, c);
				q = pushStr(mb, q, ">=");
				g = getArg(q, 0);
				pushInstruction(mb, q);
				break;
			case j_less:
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, thetauselectRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, b);
				q = pushArgument(mb, q, c);
				q = pushStr(mb, q, "<");
				g = getArg(q, 0);
				pushInstruction(mb, q);
				break;
			case j_lequal:
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, thetauselectRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, b);
				q = pushArgument(mb, q, c);
				q = pushStr(mb, q, "<=");
				g = getArg(q, 0);
				pushInstruction(mb, q);
				break;
			default:
				assert(0);
		}
	} else {  /* var <cmp> var */
		int lv[4] = {*j2, *j3, *j4, 0}, *lp = lv;
		/* FIXME: we need to check that a and b have at most one value
		 * per elem here, further code assumes that, because its
		 * semantically unclear what one should do with multiple values
		 * per element (e.g. $.reviews[*].rating), in fact I believe its
		 * impossible to say something useful about it */

		q = newInstruction(mb, ASSIGNsymbol);
		setModuleId(q, batRef);
		setFunctionId(q, reverseRef);
		q = pushArgument(mb, q, a);
		q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
		c = getArg(q, 0);
		pushInstruction(mb, q);
		q = newInstruction(mb, ASSIGNsymbol);
		setModuleId(q, algebraRef);
		setFunctionId(q, joinRef);
		q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
		q = pushArgument(mb, q, b);
		q = pushArgument(mb, q, c);
		c = getArg(q, 0);
		pushInstruction(mb, q);
		q = newInstruction(mb, ASSIGNsymbol);
		setModuleId(q, batRef);
		setFunctionId(q, reverseRef);
		q = pushArgument(mb, q, c);
		q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
		d = getArg(q, 0);
		pushInstruction(mb, q);

		q = newInstruction(mb, ASSIGNsymbol);
		setModuleId(q, algebraRef);
		setFunctionId(q, semijoinRef);
		q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
		q = pushArgument(mb, q, *j1);
		q = pushArgument(mb, q, c);
		c = getArg(q, 0);
		pushInstruction(mb, q);
		q = newInstruction(mb, ASSIGNsymbol);
		setModuleId(q, batRef);
		setFunctionId(q, reverseRef);
		q = pushArgument(mb, q, b);
		q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
		e = getArg(q, 0);
		pushInstruction(mb, q);
		q = newInstruction(mb, ASSIGNsymbol);
		setModuleId(q, algebraRef);
		setFunctionId(q, joinRef);
		q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
		q = pushArgument(mb, q, e);
		q = pushArgument(mb, q, c);
		c = getArg(q, 0);
		pushInstruction(mb, q);

		q = newInstruction(mb, ASSIGNsymbol);
		setModuleId(q, algebraRef);
		setFunctionId(q, semijoinRef);
		q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
		q = pushArgument(mb, q, *j1);
		q = pushArgument(mb, q, d);
		d = getArg(q, 0);
		pushInstruction(mb, q);
		q = newInstruction(mb, ASSIGNsymbol);
		setModuleId(q, batRef);
		setFunctionId(q, reverseRef);
		q = pushArgument(mb, q, a);
		q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
		e = getArg(q, 0);
		pushInstruction(mb, q);
		q = newInstruction(mb, ASSIGNsymbol);
		setModuleId(q, algebraRef);
		setFunctionId(q, joinRef);
		q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
		q = pushArgument(mb, q, e);
		q = pushArgument(mb, q, d);
		d = getArg(q, 0);
		pushInstruction(mb, q);

		/* booleans can only be compared with j_equals and
		 * j_nequal */
		q = newInstruction(mb, ASSIGNsymbol);
		setModuleId(q, algebraRef);
		setFunctionId(q, selectRef);
		q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
		q = pushArgument(mb, q, c);
		q = pushChr(mb, q, 't');
		e = getArg(q, 0);
		pushInstruction(mb, q);
		q = newInstruction(mb, ASSIGNsymbol);
		setModuleId(q, algebraRef);
		setFunctionId(q, selectRef);
		q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
		q = pushArgument(mb, q, c);
		q = pushChr(mb, q, 'f');
		f = getArg(q, 0);
		pushInstruction(mb, q);
		q = newInstruction(mb, ASSIGNsymbol);
		setModuleId(q, algebraRef);
		setFunctionId(q, putName("kunion", 6););
		q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
		q = pushArgument(mb, q, e);
		q = pushArgument(mb, q, f);
		c = getArg(q, 0);
		pushInstruction(mb, q);

		q = newInstruction(mb, ASSIGNsymbol);
		setModuleId(q, algebraRef);
		setFunctionId(q, selectRef);
		q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
		q = pushArgument(mb, q, d);
		q = pushChr(mb, q, 't');
		e = getArg(q, 0);
		pushInstruction(mb, q);
		q = newInstruction(mb, ASSIGNsymbol);
		setModuleId(q, algebraRef);
		setFunctionId(q, selectRef);
		q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
		q = pushArgument(mb, q, d);
		q = pushChr(mb, q, 'f');
		f = getArg(q, 0);
		pushInstruction(mb, q);
		q = newInstruction(mb, ASSIGNsymbol);
		setModuleId(q, algebraRef);
		setFunctionId(q, putName("kunion", 6););
		q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
		q = pushArgument(mb, q, e);
		q = pushArgument(mb, q, f);
		d = getArg(q, 0);
		pushInstruction(mb, q);

		q = newInstruction(mb, ASSIGNsymbol);
		setModuleId(q, batRef);
		setFunctionId(q, reverseRef);
		q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
		q = pushArgument(mb, q, d);
		e = getArg(q, 0);
		pushInstruction(mb, q);
		q = newInstruction(mb, ASSIGNsymbol);
		setModuleId(q, algebraRef);
		setFunctionId(q, joinRef);
		q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
		q = pushArgument(mb, q, c);
		q = pushArgument(mb, q, e);
		e = getArg(q, 0);
		pushInstruction(mb, q);

		if (t->tval2->cval == j_nequal) {
			q = newInstruction(mb, ASSIGNsymbol);
			setModuleId(q, algebraRef);
			setFunctionId(q, putName("kdifference", 11));
			q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
			q = pushArgument(mb, q, c);
			q = pushArgument(mb, q, e);
			e = getArg(q, 0);
			pushInstruction(mb, q);
			q = newInstruction(mb, ASSIGNsymbol);
			setModuleId(q, algebraRef);
			setFunctionId(q, projectRef);
			q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
			q = pushArgument(mb, q, e);
			q = pushNil(mb, q, TYPE_oid);
			e = getArg(q, 0);
			pushInstruction(mb, q);
		}

		g = e;

		for (; *lp != 0; lp++) {
			q = newInstruction(mb, ASSIGNsymbol);
			setModuleId(q, algebraRef);
			setFunctionId(q, semijoinRef);
			q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
			q = pushArgument(mb, q, *lp);
			q = pushArgument(mb, q, a);
			f = getArg(q, 0);
			pushInstruction(mb, q);
			q = newInstruction(mb, ASSIGNsymbol);
			setModuleId(q, algebraRef);
			setFunctionId(q, semijoinRef);
			q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
			q = pushArgument(mb, q, *lp);
			q = pushArgument(mb, q, b);
			e = getArg(q, 0);
			pushInstruction(mb, q);
			q = newInstruction(mb, ASSIGNsymbol);
			setModuleId(q, batRef);
			setFunctionId(q, reverseRef);
			q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
			q = pushArgument(mb, q, e);
			e = getArg(q, 0);
			pushInstruction(mb, q);
			q = newInstruction(mb, ASSIGNsymbol);
			setModuleId(q, algebraRef);
			switch (t->tval2->cval) {
				case j_equals:
				case j_nequal: /* difference handled later */
					setFunctionId(q, joinRef);
					q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
					q = pushArgument(mb, q, f);
					q = pushArgument(mb, q, e);
					e = getArg(q, 0);
					pushInstruction(mb, q);
					break;
				case j_greater:
					setFunctionId(q, thetajoinRef);
					q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
					q = pushArgument(mb, q, f);
					q = pushArgument(mb, q, e);
					q = pushInt(mb, q, 1);
					e = getArg(q, 0);
					pushInstruction(mb, q);
					break;
				case j_gequal:
					setFunctionId(q, thetajoinRef);
					q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
					q = pushArgument(mb, q, f);
					q = pushArgument(mb, q, e);
					q = pushInt(mb, q, 2);
					e = getArg(q, 0);
					pushInstruction(mb, q);
					break;
				case j_less:
					setFunctionId(q, thetajoinRef);
					q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
					q = pushArgument(mb, q, f);
					q = pushArgument(mb, q, e);
					q = pushInt(mb, q, -1);
					e = getArg(q, 0);
					pushInstruction(mb, q);
					break;
				case j_lequal:
					setFunctionId(q, thetajoinRef);
					q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
					q = pushArgument(mb, q, f);
					q = pushArgument(mb, q, e);
					q = pushInt(mb, q, -2);
					e = getArg(q, 0);
					pushInstruction(mb, q);
					break;
				default:
					assert(0);
			}
			q = newInstruction(mb, ASSIGNsymbol);
			setModuleId(q, algebraRef);
			setFunctionId(q, semijoinRef);
			q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
			q = pushArgument(mb, q, a);
			q = pushArgument(mb, q, e);
			d = getArg(q, 0);
			pushInstruction(mb, q);
			q = newInstruction(mb, ASSIGNsymbol);
			setModuleId(q, batRef);
			setFunctionId(q, reverseRef);
			q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
			q = pushArgument(mb, q, e);
			e = getArg(q, 0);
			pushInstruction(mb, q);
			q = newInstruction(mb, ASSIGNsymbol);
			setModuleId(q, algebraRef);
			setFunctionId(q, semijoinRef);
			q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
			q = pushArgument(mb, q, b);
			q = pushArgument(mb, q, e);
			e = getArg(q, 0);
			pushInstruction(mb, q);
			q = newInstruction(mb, ASSIGNsymbol);
			setModuleId(q, batRef);
			setFunctionId(q, reverseRef);
			q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
			q = pushArgument(mb, q, e);
			e = getArg(q, 0);
			pushInstruction(mb, q);
			q = newInstruction(mb, ASSIGNsymbol);
			setModuleId(q, batRef);
			setFunctionId(q, mirrorRef);
			q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
			q = pushArgument(mb, q, e);
			e = getArg(q, 0);
			pushInstruction(mb, q);
			q = newInstruction(mb, ASSIGNsymbol);
			setModuleId(q, algebraRef);
			setFunctionId(q, joinRef);
			q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
			q = pushArgument(mb, q, d);
			q = pushArgument(mb, q, e);
			e = getArg(q, 0);
			pushInstruction(mb, q);
			if (t->tval2->cval == j_nequal) {
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, semijoinRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, a);
				q = pushArgument(mb, q, f);
				f = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, putName("kdifference", 11));
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, f);
				q = pushArgument(mb, q, e);
				e = getArg(q, 0);
				pushInstruction(mb, q);
			}
			q = newInstruction(mb, ASSIGNsymbol);
			setModuleId(q, batRef);
			setFunctionId(q, reverseRef);
			q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
			q = pushArgument(mb, q, e);
			e = getArg(q, 0);
			pushInstruction(mb, q);
			q = newInstruction(mb, ASSIGNsymbol);
			setModuleId(q, batRef);
			setFunctionId(q, insertRef);
			q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
			q = pushArgument(mb, q, g);
			q = pushArgument(mb, q, e);
			g = getArg(q, 0);
			pushInstruction(mb, q);
		}
	}
	return g;
}

static void
dumppredjoin(MalBlkPtr mb, json_var *js, tree *t, int *j1, int *j2, int *j3, int *j4, int *j5, int *j6, int *j7)
{
	InstrPtr q;
	int a, b, c, d, l, r;
	tree *pred;
	json_var *vars, *ljv, *rjv;
	join_result *jrs = NULL, *jrw;

	/* iterate through all predicates and load the set from the correct
	 * JSON variable */
	for (pred = t->tval2; pred != NULL; pred = pred->next) {
		assert(pred->tval1->type == j_var);
		assert(pred->tval2->cval == j_equals);
		assert(pred->tval3->type == j_var);

#define locate_var(X, Y) \
		X = NULL; \
		for (vars = js; vars->name != NULL; vars++) { \
			if (strcmp(vars->name, Y) == 0) { \
				X = vars; \
				break; \
			} \
		}
		locate_var(ljv, pred->tval1->sval);
		a = dumpwalkvar(mb, ljv->j1, ljv->j5);
		l = dumprefvar(mb, pred->tval1, a, ljv->j1, ljv->j6, ljv->j7);

		q = newInstruction(mb, ASSIGNsymbol);
		setModuleId(q, batRef);
		setFunctionId(q, reverseRef);
		q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
		q = pushArgument(mb, q, l);
		d = getArg(q, 0);
		pushInstruction(mb, q);

		locate_var(rjv, pred->tval3->sval);
		a = dumpwalkvar(mb, rjv->j1, rjv->j5);
		r = dumprefvar(mb, pred->tval3, a, rjv->j1, rjv->j6, rjv->j7);

		/* strings */
		q = newInstruction(mb, ASSIGNsymbol);
		setModuleId(q, algebraRef);
		setFunctionId(q, semijoinRef);
		q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
		q = pushArgument(mb, q, ljv->j2);
		q = pushArgument(mb, q, l);
		b = getArg(q, 0);
		pushInstruction(mb, q);
		q = newInstruction(mb, ASSIGNsymbol);
		setModuleId(q, algebraRef);
		setFunctionId(q, semijoinRef);
		q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
		q = pushArgument(mb, q, rjv->j2);
		q = pushArgument(mb, q, r);
		c = getArg(q, 0);
		pushInstruction(mb, q);
		q = newInstruction(mb, ASSIGNsymbol);
		setModuleId(q, batRef);
		setFunctionId(q, reverseRef);
		q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
		q = pushArgument(mb, q, c);
		c = getArg(q, 0);
		pushInstruction(mb, q);
		q = newInstruction(mb, ASSIGNsymbol);
		setModuleId(q, algebraRef);
		setFunctionId(q, joinRef);
		q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
		q = pushArgument(mb, q, b);
		q = pushArgument(mb, q, c);
		a = getArg(q, 0);
		pushInstruction(mb, q);
		q = newInstruction(mb, ASSIGNsymbol);
		setModuleId(q, algebraRef);
		setFunctionId(q, joinRef);
		q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
		q = pushArgument(mb, q, d);
		q = pushArgument(mb, q, a);
		a = getArg(q, 0);
		pushInstruction(mb, q);
		q = newInstruction(mb, ASSIGNsymbol);
		setModuleId(q, algebraRef);
		setFunctionId(q, joinRef);
		q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
		q = pushArgument(mb, q, a);
		q = pushArgument(mb, q, r);
		a = getArg(q, 0);
		pushInstruction(mb, q);

		/* ints */
		q = newInstruction(mb, ASSIGNsymbol);
		setModuleId(q, algebraRef);
		setFunctionId(q, semijoinRef);
		q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
		q = pushArgument(mb, q, ljv->j3);
		q = pushArgument(mb, q, l);
		b = getArg(q, 0);
		pushInstruction(mb, q);
		q = newInstruction(mb, ASSIGNsymbol);
		setModuleId(q, algebraRef);
		setFunctionId(q, semijoinRef);
		q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
		q = pushArgument(mb, q, rjv->j3);
		q = pushArgument(mb, q, r);
		c = getArg(q, 0);
		pushInstruction(mb, q);
		q = newInstruction(mb, ASSIGNsymbol);
		setModuleId(q, batRef);
		setFunctionId(q, reverseRef);
		q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
		q = pushArgument(mb, q, c);
		c = getArg(q, 0);
		pushInstruction(mb, q);
		q = newInstruction(mb, ASSIGNsymbol);
		setModuleId(q, algebraRef);
		setFunctionId(q, joinRef);
		q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
		q = pushArgument(mb, q, b);
		q = pushArgument(mb, q, c);
		b = getArg(q, 0);
		pushInstruction(mb, q);
		q = newInstruction(mb, ASSIGNsymbol);
		setModuleId(q, algebraRef);
		setFunctionId(q, joinRef);
		q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
		q = pushArgument(mb, q, d);
		q = pushArgument(mb, q, b);
		b = getArg(q, 0);
		pushInstruction(mb, q);
		q = newInstruction(mb, ASSIGNsymbol);
		setModuleId(q, algebraRef);
		setFunctionId(q, joinRef);
		q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
		q = pushArgument(mb, q, b);
		q = pushArgument(mb, q, r);
		b = getArg(q, 0);
		pushInstruction(mb, q);
		q = newInstruction(mb, ASSIGNsymbol);
		setModuleId(q, algebraRef);
		setFunctionId(q, kunionRef);
		q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
		q = pushArgument(mb, q, a);
		q = pushArgument(mb, q, b);
		a = getArg(q, 0);
		pushInstruction(mb, q);

		/* dbls */
		q = newInstruction(mb, ASSIGNsymbol);
		setModuleId(q, algebraRef);
		setFunctionId(q, semijoinRef);
		q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
		q = pushArgument(mb, q, ljv->j4);
		q = pushArgument(mb, q, l);
		b = getArg(q, 0);
		pushInstruction(mb, q);
		q = newInstruction(mb, ASSIGNsymbol);
		setModuleId(q, algebraRef);
		setFunctionId(q, semijoinRef);
		q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
		q = pushArgument(mb, q, rjv->j4);
		q = pushArgument(mb, q, r);
		c = getArg(q, 0);
		pushInstruction(mb, q);
		q = newInstruction(mb, ASSIGNsymbol);
		setModuleId(q, batRef);
		setFunctionId(q, reverseRef);
		q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
		q = pushArgument(mb, q, c);
		c = getArg(q, 0);
		pushInstruction(mb, q);
		q = newInstruction(mb, ASSIGNsymbol);
		setModuleId(q, algebraRef);
		setFunctionId(q, joinRef);
		q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
		q = pushArgument(mb, q, b);
		q = pushArgument(mb, q, c);
		b = getArg(q, 0);
		pushInstruction(mb, q);
		q = newInstruction(mb, ASSIGNsymbol);
		setModuleId(q, algebraRef);
		setFunctionId(q, joinRef);
		q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
		q = pushArgument(mb, q, d);
		q = pushArgument(mb, q, b);
		b = getArg(q, 0);
		pushInstruction(mb, q);
		q = newInstruction(mb, ASSIGNsymbol);
		setModuleId(q, algebraRef);
		setFunctionId(q, joinRef);
		q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
		q = pushArgument(mb, q, b);
		q = pushArgument(mb, q, r);
		b = getArg(q, 0);
		pushInstruction(mb, q);
		q = newInstruction(mb, ASSIGNsymbol);
		setModuleId(q, algebraRef);
		setFunctionId(q, kunionRef);
		q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
		q = pushArgument(mb, q, a);
		q = pushArgument(mb, q, b);
		a = getArg(q, 0);
		pushInstruction(mb, q);

		/* a now contains matching oids in head l, in tail r */
		if (jrs == NULL) {
			jrw = jrs = GDKzalloc(sizeof(join_result));
		} else {
			jrw = jrw->next = GDKzalloc(sizeof(join_result));
		}
		jrw->bat = a;
		jrw->headvar = ljv;
		jrw->tailvar = rjv;
	}

	/* intersect the joins */
	for (jrw = jrs; jrw != NULL; jrw = jrw->next) {
		join_result *jrv;
		for (jrv = jrw->next; jrv != NULL; jrv = jrv->next) {
			if (jrw->headvar == jrv->headvar) {
			} else if (jrw->headvar == jrv->headvar) {
				/* head-on-head */
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, semijoinRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, jrw->bat);
				q = pushArgument(mb, q, jrv->bat);
				jrw->bat = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, semijoinRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, jrv->bat);
				q = pushArgument(mb, q, jrw->bat);
				jrv->bat = getArg(q, 0);
				pushInstruction(mb, q);
			} else if (jrw->headvar == jrv->tailvar) {
				/* head-on-tail */
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, batRef);
				setFunctionId(q, reverseRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, jrv->bat);
				b = getArg(q, 0);
				pushInstruction(mb, q);

				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, semijoinRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, jrw->bat);
				q = pushArgument(mb, q, b);
				jrw->bat = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, semijoinRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, b);
				q = pushArgument(mb, q, jrw->bat);
				b = getArg(q, 0);
				pushInstruction(mb, q);

				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, batRef);
				setFunctionId(q, reverseRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, b);
				jrv->bat = getArg(q, 0);
				pushInstruction(mb, q);
			} else if (jrw->tailvar == jrv->headvar) {
				/* tail-on-head */
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, batRef);
				setFunctionId(q, reverseRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, jrw->bat);
				b = getArg(q, 0);
				pushInstruction(mb, q);

				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, semijoinRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, b);
				q = pushArgument(mb, q, jrv->bat);
				b = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, semijoinRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, jrv->bat);
				q = pushArgument(mb, q, b);
				jrv->bat = getArg(q, 0);
				pushInstruction(mb, q);

				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, batRef);
				setFunctionId(q, reverseRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, b);
				jrw->bat = getArg(q, 0);
				pushInstruction(mb, q);
			} else if (jrw->tailvar == jrv->tailvar) {
				/* tail-on-tail */
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, batRef);
				setFunctionId(q, reverseRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, jrv->bat);
				b = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, batRef);
				setFunctionId(q, reverseRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, jrw->bat);
				c = getArg(q, 0);
				pushInstruction(mb, q);

				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, semijoinRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, b);
				q = pushArgument(mb, q, c);
				b = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, semijoinRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, c);
				q = pushArgument(mb, q, b);
				c = getArg(q, 0);
				pushInstruction(mb, q);

				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, batRef);
				setFunctionId(q, reverseRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, b);
				jrv->bat = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, batRef);
				setFunctionId(q, reverseRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, c);
				jrw->bat = getArg(q, 0);
				pushInstruction(mb, q);
			}
		}
	}

	/* for each column extract elems */
	q = newInstruction(mb, ASSIGNsymbol);
	q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
	q = pushOid(mb, q, 0);
	b = getArg(q, 0);
	pushInstruction(mb, q);
	for (vars = js; vars->name != NULL; vars++) {
		d = -1;
		for (jrw = jrs; jrw != NULL; jrw = jrw->next) {
			if (vars == jrw->tailvar) {
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, putName("json", 4));
				setFunctionId(q, putName("extract", 7));
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, vars->j1);
				q = pushArgument(mb, q, vars->j2);
				q = pushArgument(mb, q, vars->j3);
				q = pushArgument(mb, q, vars->j4);
				q = pushArgument(mb, q, vars->j5);
				q = pushArgument(mb, q, vars->j6);
				q = pushArgument(mb, q, vars->j7);
				q = pushArgument(mb, q, jrw->bat);
				q = pushArgument(mb, q, b);
				vars->j1 = getArg(q, 0);
				vars->j2 = getArg(q, 1);
				vars->j3 = getArg(q, 2);
				vars->j4 = getArg(q, 3);
				vars->j5 = getArg(q, 4);
				vars->j6 = getArg(q, 5);
				vars->j7 = getArg(q, 6);
				pushInstruction(mb, q);

				d = 1;
			} else if (vars == jrw->headvar) {
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, batRef);
				setFunctionId(q, reverseRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, jrw->bat);
				c = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, putName("json", 4));
				setFunctionId(q, putName("extract", 7));
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, vars->j1);
				q = pushArgument(mb, q, vars->j2);
				q = pushArgument(mb, q, vars->j3);
				q = pushArgument(mb, q, vars->j4);
				q = pushArgument(mb, q, vars->j5);
				q = pushArgument(mb, q, vars->j6);
				q = pushArgument(mb, q, vars->j7);
				q = pushArgument(mb, q, c);
				q = pushArgument(mb, q, b);
				vars->j1 = getArg(q, 0);
				vars->j2 = getArg(q, 1);
				vars->j3 = getArg(q, 2);
				vars->j4 = getArg(q, 3);
				vars->j5 = getArg(q, 4);
				vars->j6 = getArg(q, 5);
				vars->j7 = getArg(q, 6);
				pushInstruction(mb, q);

				d = 1;
			}

			if (d == 1) {
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, batRef);
				setFunctionId(q, reverseRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, vars->j5);
				l = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, selectRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, l);
				q = pushArgument(mb, q, b);
				l = getArg(q, 0);
				pushInstruction(mb, q);

				vars->ids = l;

				/* remove outer array */
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, batRef);
				setFunctionId(q, deleteRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, vars->j5);
				q = pushArgument(mb, q, b);
				vars->j5 = getArg(q, 0);
				pushInstruction(mb, q);

				b = dumpnextid(mb, vars->j1);

				break;
			}
		}
		assert(d == 1);  /* join input/where check was done before */
	}

	/* create new objects */
	for (vars = js; vars->name != NULL; vars++) {
		/* names of the pairs we create (for each var) */
		q = newInstruction(mb, ASSIGNsymbol);
		setModuleId(q, algebraRef);
		setFunctionId(q, projectRef);
		q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
		q = pushArgument(mb, q, vars->ids);
		q = pushStr(mb, q, vars->name);
		l = getArg(q, 0);
		pushInstruction(mb, q);
		q = newInstruction(mb, ASSIGNsymbol);
		setModuleId(q, batRef);
		setFunctionId(q, insertRef);
		q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
		q = pushArgument(mb, q, vars->j7);
		q = pushArgument(mb, q, l);
		vars->j7 = getArg(q, 0);
		pushInstruction(mb, q);

		/* create object ref holding the pair */
		q = newInstruction(mb, ASSIGNsymbol);
		setModuleId(q, algebraRef);
		setFunctionId(q, markTRef);
		q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
		q = pushArgument(mb, q, vars->ids);
		q = pushArgument(mb, q, b);
		l = getArg(q, 0);
		pushInstruction(mb, q);
		q = newInstruction(mb, ASSIGNsymbol);
		setModuleId(q, batRef);
		setFunctionId(q, reverseRef);
		q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
		q = pushArgument(mb, q, l);
		l = getArg(q, 0);
		pushInstruction(mb, q);
		q = newInstruction(mb, ASSIGNsymbol);
		setModuleId(q, batRef);
		setFunctionId(q, insertRef);
		q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
		q = pushArgument(mb, q, vars->j6);
		q = pushArgument(mb, q, l);
		vars->j6 = getArg(q, 0);
		pushInstruction(mb, q);
	}

	q = newInstruction(mb, ASSIGNsymbol);
	setModuleId(q, algebraRef);
	setFunctionId(q, markTRef);
	q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
	q = pushArgument(mb, q, js->ids);
	q = pushArgument(mb, q, b);
	r = getArg(q, 0);
	pushInstruction(mb, q);
	q = newInstruction(mb, ASSIGNsymbol);
	setModuleId(q, batRef);
	setFunctionId(q, reverseRef);
	q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
	q = pushArgument(mb, q, r);
	r = getArg(q, 0);
	pushInstruction(mb, q);

	/* generate kind entries for the new objects */
	q = newInstruction(mb, ASSIGNsymbol);
	setModuleId(q, algebraRef);
	setFunctionId(q, projectRef);
	q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
	q = pushArgument(mb, q, r);
	q = pushChr(mb, q, 'o');
	l = getArg(q, 0);
	pushInstruction(mb, q);
	q = newInstruction(mb, ASSIGNsymbol);
	setModuleId(q, batRef);
	setFunctionId(q, insertRef);
	q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
	q = pushArgument(mb, q, js->j1);
	q = pushArgument(mb, q, l);
	js->j1 = getArg(q, 0);
	pushInstruction(mb, q);

	/* generate outermost array */
	q = newInstruction(mb, ASSIGNsymbol);
	setModuleId(q, algebraRef);
	setFunctionId(q, projectRef);
	q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
	q = pushArgument(mb, q, r);
	q = pushOid(mb, q, 0);
	l = getArg(q, 0);
	pushInstruction(mb, q);
	q = newInstruction(mb, ASSIGNsymbol);
	setModuleId(q, batRef);
	setFunctionId(q, reverseRef);
	q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
	q = pushArgument(mb, q, l);
	l = getArg(q, 0);
	pushInstruction(mb, q);
	q = newInstruction(mb, ASSIGNsymbol);
	setModuleId(q, batRef);
	setFunctionId(q, insertRef);
	q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
	q = pushArgument(mb, q, js->j5);
	q = pushArgument(mb, q, l);
	js->j5 = getArg(q, 0);
	pushInstruction(mb, q);

	/* merge everything into one */
	for (vars = &js[1]; vars->name != NULL; vars++) {
		q = newInstruction(mb, ASSIGNsymbol);
		setModuleId(q, batRef);
		setFunctionId(q, insertRef);
		q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
		q = pushArgument(mb, q, js->j1);
		q = pushArgument(mb, q, vars->j1);
		*j1 = js->j1 = getArg(q, 0);
		pushInstruction(mb, q);
		q = newInstruction(mb, ASSIGNsymbol);
		setModuleId(q, batRef);
		setFunctionId(q, insertRef);
		q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
		q = pushArgument(mb, q, js->j2);
		q = pushArgument(mb, q, vars->j2);
		*j2 = js->j2 = getArg(q, 0);
		pushInstruction(mb, q);
		q = newInstruction(mb, ASSIGNsymbol);
		setModuleId(q, batRef);
		setFunctionId(q, insertRef);
		q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
		q = pushArgument(mb, q, js->j3);
		q = pushArgument(mb, q, vars->j3);
		*j3 = js->j3 = getArg(q, 0);
		pushInstruction(mb, q);
		q = newInstruction(mb, ASSIGNsymbol);
		setModuleId(q, batRef);
		setFunctionId(q, insertRef);
		q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
		q = pushArgument(mb, q, js->j4);
		q = pushArgument(mb, q, vars->j4);
		*j4 = js->j4 = getArg(q, 0);
		pushInstruction(mb, q);
		q = newInstruction(mb, ASSIGNsymbol);
		setModuleId(q, batRef);
		setFunctionId(q, insertRef);
		q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
		q = pushArgument(mb, q, js->j5);
		q = pushArgument(mb, q, vars->j5);
		*j5 = js->j5 = getArg(q, 0);
		pushInstruction(mb, q);
		q = newInstruction(mb, ASSIGNsymbol);
		setModuleId(q, batRef);
		setFunctionId(q, insertRef);
		q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
		q = pushArgument(mb, q, js->j6);
		q = pushArgument(mb, q, vars->j6);
		*j6 = js->j6 = getArg(q, 0);
		pushInstruction(mb, q);
		q = newInstruction(mb, ASSIGNsymbol);
		setModuleId(q, batRef);
		setFunctionId(q, insertRef);
		q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
		q = pushArgument(mb, q, js->j7);
		q = pushArgument(mb, q, vars->j7);
		*j7 = js->j7 = getArg(q, 0);
		pushInstruction(mb, q);
	}
}

static int
dumppred(MalBlkPtr mb, tree *t, int elems, int *j1, int *j2, int *j3, int *j4, int *j5, int *j6, int *j7)
{
	int a, l, r;
	InstrPtr q;

	assert(t != NULL && t->tval2->type == j_comp);

	/* comparisons only take place between tval1 = var and tval3 = val/var
	 * for the rest, only boolean logic is applied */
	if (t->tval2->cval != j_and && t->tval2->cval != j_or)
		return dumpcomp(mb, t, elems, j1, j2, j3, j4, j5, j6, j7);

	assert(t->tval1->type == j_pred);
	assert(t->tval2->cval == j_and || t->tval2->cval == j_or);
	assert(t->tval3->type == j_pred);

	l = dumppred(mb, t->tval1, elems, j1, j2, j3, j4, j5, j6, j7);
	r = dumppred(mb, t->tval3, elems, j1, j2, j3, j4, j5, j6, j7);
	/* l,r = oid from elems that match in head */

	if (t->tval2->cval == j_and) {
		q = newInstruction(mb, ASSIGNsymbol);
		setModuleId(q, algebraRef);
		setFunctionId(q, putName("kintersect", 10));
		q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
		q = pushArgument(mb, q, l);
		q = pushArgument(mb, q, r);
		a = getArg(q, 0);
		pushInstruction(mb, q);
	} else { /* j_or */
		q = newInstruction(mb, ASSIGNsymbol);
		setModuleId(q, algebraRef);
		setFunctionId(q, putName("kunion", 6));
		q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
		q = pushArgument(mb, q, l);
		q = pushArgument(mb, q, r);
		a = getArg(q, 0);
		pushInstruction(mb, q);
	}

	return a;
}

static int
dumpnextid(MalBlkPtr mb, int j1)
{
	int a;
	InstrPtr q;

	q = newInstruction(mb, ASSIGNsymbol);
	setModuleId(q, batRef);
	setFunctionId(q, reverseRef);
	q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
	q = pushArgument(mb, q, j1);
	a = getArg(q, 0);
	pushInstruction(mb, q);
	q = newInstruction(mb, ASSIGNsymbol);
	setModuleId(q, aggrRef);
	setFunctionId(q, maxRef);
	q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
	q = pushArgument(mb, q, a);
	a = getArg(q, 0);
	pushInstruction(mb, q);
	q = newInstruction(mb, ASSIGNsymbol);
	setModuleId(q, calcRef);
	setFunctionId(q, putName("wrd", 3));
	q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
	q = pushArgument(mb, q, a);
	a = getArg(q, 0);
	pushInstruction(mb, q);
	q = newInstruction(mb, ASSIGNsymbol);
	setModuleId(q, calcRef);
	setFunctionId(q, putName("+", 1));
	q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
	q = pushArgument(mb, q, a);
	q = pushWrd(mb, q, 1);
	a = getArg(q, 0);
	pushInstruction(mb, q);
	q = newInstruction(mb, ASSIGNsymbol);
	setModuleId(q, calcRef);
	setFunctionId(q, oidRef);
	q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
	q = pushArgument(mb, q, a);
	a = getArg(q, 0);
	pushInstruction(mb, q);

	return a;
}

/* returns a BAT which contains the ids of j1 that refer to the values
 * returned from the variable and its optional calculation applied to it
 * the tail is the element id from the head of elems
 * the j{1..7} variables are updated to point to the updated BATs as
 * insertions of new values (the serialised versions of the variable) */
static int
dumpvariabletransformation(MalBlkPtr mb, tree *t, int elems,
		int *j1, int *j2, int *j3, int *j4, int *j5, int *j6, int *j7)
{
	InstrPtr q;
	int a, b, c, d, e, f, g;

	assert (t != NULL);

	switch (t->type) {
		case j_str:
		case j_num:
		case j_dbl:
			/* fill up elems size of this constant */
			a = dumpnextid(mb, *j1);

			q = newInstruction(mb, ASSIGNsymbol);
			setModuleId(q, algebraRef);
			setFunctionId(q, markHRef);
			q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
			q = pushArgument(mb, q, elems);
			q = pushArgument(mb, q, a);
			c = getArg(q, 0);
			pushInstruction(mb, q);

			q = newInstruction(mb, ASSIGNsymbol);
			setModuleId(q, algebraRef);
			setFunctionId(q, projectRef);
			q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
			q = pushArgument(mb, q, c);
			if (t->type == j_num) {
				q = pushLng(mb, q, t->nval);
			} else if (t->type == j_dbl) {
				q = pushDbl(mb, q, t->dval);
			} else {
				q = pushStr(mb, q, t->sval);
			}
			b = getArg(q, 0);
			pushInstruction(mb, q);
			q = newInstruction(mb, ASSIGNsymbol);
			setModuleId(q, algebraRef);
			setFunctionId(q, projectRef);
			q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
			q = pushArgument(mb, q, c);
			if (t->type == j_num) {
				q = pushChr(mb, q, 'i');
				d = *j3;
			} else if (t->type == j_dbl) {
				q = pushChr(mb, q, 'd');
				d = *j4;
			} else {
				q = pushChr(mb, q, 's');
				d = *j2;
			}
			e = getArg(q, 0);
			pushInstruction(mb, q);
			q = newInstruction(mb, ASSIGNsymbol);
			setModuleId(q, algebraRef);
			setFunctionId(q, putName("kunion", 6));
			q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
			q = pushArgument(mb, q, d);
			q = pushArgument(mb, q, b);
			if (t->type == j_num) {
				*j3 = getArg(q, 0);
			} else if (t->type == j_dbl) {
				*j4 = getArg(q, 0);
			} else if (t->type == j_str) {
				*j2 = getArg(q, 0);
			}
			pushInstruction(mb, q);
			q = newInstruction(mb, ASSIGNsymbol);
			setModuleId(q, algebraRef);
			setFunctionId(q, putName("kunion", 6));
			q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
			q = pushArgument(mb, q, *j1);
			q = pushArgument(mb, q, e);
			*j1 = getArg(q, 0);
			pushInstruction(mb, q);

			q = newInstruction(mb, ASSIGNsymbol);
			setModuleId(q, algebraRef);
			setFunctionId(q, markTRef);
			q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
			q = pushArgument(mb, q, elems);
			q = pushArgument(mb, q, a);
			b = getArg(q, 0);
			pushInstruction(mb, q);
			q = newInstruction(mb, ASSIGNsymbol);
			setModuleId(q, batRef);
			setFunctionId(q, reverseRef);
			q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
			q = pushArgument(mb, q, b);
			b = getArg(q, 0);
			pushInstruction(mb, q);

			return b;
		case j_var:
			b = dumprefvar(mb, t, elems, *j1, *j6, *j7);

			/* add back missing vars as null */
			a = dumpnextid(mb, *j1);

			q = newInstruction(mb, ASSIGNsymbol);
			setModuleId(q, batRef);
			setFunctionId(q, mirrorRef);
			q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
			q = pushArgument(mb, q, elems);
			e = getArg(q, 0);
			pushInstruction(mb, q);
			q = newInstruction(mb, ASSIGNsymbol);
			setModuleId(q, batRef);
			setFunctionId(q, reverseRef);
			q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
			q = pushArgument(mb, q, b);
			d = getArg(q, 0);
			pushInstruction(mb, q);
			q = newInstruction(mb, ASSIGNsymbol);
			setModuleId(q, algebraRef);
			setFunctionId(q, putName("kdifference", 11));
			q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
			q = pushArgument(mb, q, e);
			q = pushArgument(mb, q, d);
			c = getArg(q, 0);
			pushInstruction(mb, q);
			q = newInstruction(mb, ASSIGNsymbol);
			setModuleId(q, algebraRef);
			setFunctionId(q, projectRef);
			q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
			q = pushArgument(mb, q, c);
			q = pushChr(mb, q, 'n');
			c = getArg(q, 0);
			pushInstruction(mb, q);
			q = newInstruction(mb, ASSIGNsymbol);
			setModuleId(q, algebraRef);
			setFunctionId(q, markHRef);
			q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
			q = pushArgument(mb, q, c);
			q = pushArgument(mb, q, a);
			d = getArg(q, 0);
			pushInstruction(mb, q);
			q = newInstruction(mb, ASSIGNsymbol);
			setModuleId(q, algebraRef);
			setFunctionId(q, putName("kunion", 6));
			q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
			q = pushArgument(mb, q, *j1);
			q = pushArgument(mb, q, d);
			*j1 = getArg(q, 0);
			pushInstruction(mb, q);

			/* create return mapping between original elem and newly
			 * created ids */
			q = newInstruction(mb, ASSIGNsymbol);
			setModuleId(q, algebraRef);
			setFunctionId(q, markTRef);
			q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
			q = pushArgument(mb, q, c);
			q = pushArgument(mb, q, a);
			d = getArg(q, 0);
			pushInstruction(mb, q);
			q = newInstruction(mb, ASSIGNsymbol);
			setModuleId(q, batRef);
			setFunctionId(q, reverseRef);
			q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
			q = pushArgument(mb, q, d);
			d = getArg(q, 0);
			pushInstruction(mb, q);
			q = newInstruction(mb, ASSIGNsymbol);
			setModuleId(q, algebraRef);
			setFunctionId(q, putName("kunion", 6));
			q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
			q = pushArgument(mb, q, b);
			q = pushArgument(mb, q, d);
			b = getArg(q, 0);
			pushInstruction(mb, q);

			/* return in original elem order */
			q = newInstruction(mb, ASSIGNsymbol);
			setModuleId(q, batRef);
			setFunctionId(q, reverseRef);
			q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
			q = pushArgument(mb, q, b);
			b = getArg(q, 0);
			pushInstruction(mb, q);
			q = newInstruction(mb, ASSIGNsymbol);
			setModuleId(q, algebraRef);
			setFunctionId(q, leftjoinRef);
			q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
			q = pushArgument(mb, q, e);
			q = pushArgument(mb, q, b);
			b = getArg(q, 0);
			pushInstruction(mb, q);
			q = newInstruction(mb, ASSIGNsymbol);
			setModuleId(q, batRef);
			setFunctionId(q, reverseRef);
			q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
			q = pushArgument(mb, q, b);
			b = getArg(q, 0);
			pushInstruction(mb, q);

			return b;
		case j_operation: {
			int r, s;
			int u, v;
			int h, i, j, k, l;
			InstrPtr p;
			switch (t->tval1->type) {
				case j_var:
				case j_operation:
					b = dumpvariabletransformation(mb, t->tval1, elems,
							j1, j2, j3, j4, j5, j6, j7);

					q = newInstruction(mb, ASSIGNsymbol);
					setModuleId(q, algebraRef);
					setFunctionId(q, semijoinRef);
					q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
					q = pushArgument(mb, q, *j3);
					q = pushArgument(mb, q, b);
					d = getArg(q, 0);
					pushInstruction(mb, q);
					q = newInstruction(mb, ASSIGNsymbol);
					setModuleId(q, algebraRef);
					setFunctionId(q, semijoinRef);
					q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
					q = pushArgument(mb, q, *j4);
					q = pushArgument(mb, q, b);
					e = getArg(q, 0);
					pushInstruction(mb, q);
					break;
				default:
					assert(0);
			}
			/* d:int and e:dbl are values from val1 */

			switch (t->tval3->type) {
				case j_var:
				case j_operation:
					c = dumpvariabletransformation(mb, t->tval3, elems,
							j1, j2, j3, j4, j5, j6, j7);

					q = newInstruction(mb, ASSIGNsymbol);
					setModuleId(q, algebraRef);
					setFunctionId(q, semijoinRef);
					q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
					q = pushArgument(mb, q, *j3);
					q = pushArgument(mb, q, c);
					f = getArg(q, 0);
					pushInstruction(mb, q);
					q = newInstruction(mb, ASSIGNsymbol);
					setModuleId(q, algebraRef);
					setFunctionId(q, semijoinRef);
					q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
					q = pushArgument(mb, q, *j4);
					q = pushArgument(mb, q, c);
					g = getArg(q, 0);
					pushInstruction(mb, q);
					q = newInstruction(mb, ASSIGNsymbol);
					break;
				case j_num:
					c = -1;
					q = newInstruction(mb, ASSIGNsymbol);
					q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
					q = pushLng(mb, q, t->tval3->nval);
					f = getArg(q, 0);
					pushInstruction(mb, q);
					q = newInstruction(mb, ASSIGNsymbol);
					setModuleId(q, calcRef);
					setFunctionId(q, putName("dbl", 3));
					q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
					q = pushArgument(mb, q, f);
					g = getArg(q, 0);
					pushInstruction(mb, q);
					break;
				case j_dbl:
					c = -1;
					f = -1;
					q = newInstruction(mb, ASSIGNsymbol);
					q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
					q = pushDbl(mb, q, t->tval3->dval);
					g = getArg(q, 0);
					pushInstruction(mb, q);
					break;
				default:
					assert(0);
			}
			/* f:int and g:dbl are values from val3, bats if c != -1 */

			a = dumpnextid(mb, *j1);
			q = newInstruction(mb, ASSIGNsymbol);
			setModuleId(q, algebraRef);
			setFunctionId(q, markTRef);
			q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
			q = pushArgument(mb, q, b);
			q = pushArgument(mb, q, a);
			r = getArg(q, 0);
			pushInstruction(mb, q);
			q = newInstruction(mb, ASSIGNsymbol);
			setModuleId(q, batRef);
			setFunctionId(q, reverseRef);
			q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
			q = pushArgument(mb, q, r);
			j = getArg(q, 0);
			pushInstruction(mb, q);

			if (c != -1) {
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, semijoinRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, r);
				q = pushArgument(mb, q, d);
				s = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, batRef);
				setFunctionId(q, reverseRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, s);
				k = getArg(q, 0);
				pushInstruction(mb, q);

				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, markTRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, c);
				q = pushArgument(mb, q, a);
				u = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, semijoinRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, u);
				q = pushArgument(mb, q, f);
				v = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, batRef);
				setFunctionId(q, reverseRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, v);
				l = v = getArg(q, 0);
				pushInstruction(mb, q);

				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, joinRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, s);
				q = pushArgument(mb, q, v);
				s = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, batRef);
				setFunctionId(q, reverseRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, s);
				v = getArg(q, 0);
				pushInstruction(mb, q);

				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, semijoinRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, d);
				q = pushArgument(mb, q, s);
				s = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, semijoinRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, f);
				q = pushArgument(mb, q, v);
				v = getArg(q, 0);
				pushInstruction(mb, q);
			} else {
				s = d;
				v = f;
			}

			q = newInstruction(mb, ASSIGNsymbol);
			setModuleId(q, batcalcRef);
			switch (t->tval2->cval) {
				case j_plus:
					setFunctionId(q, putName("+", 1));
					break;
				case j_min:
					setFunctionId(q, putName("-", 1));
					break;
				case j_multiply:
					setFunctionId(q, putName("*", 1));
					break;
				case j_divide:
					setFunctionId(q, putName("/", 1));
					break;
				default:
					assert(0);
			}
			q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
			q = pushArgument(mb, q, s);
			q = pushArgument(mb, q, v);
			if (v != -1) {
				h = getArg(q, 0);
				pushInstruction(mb, q);
			} else {
				h = -1;
			}
			p = copyInstruction(q); /* reuse for dbl case below */

			if (h != -1) {
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, joinRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, j);
				q = pushArgument(mb, q, h);
				h = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, putName("kunion", 6));
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, *j3);
				q = pushArgument(mb, q, h);
				*j3 = getArg(q, 0);
				pushInstruction(mb, q);
			}

			if (c != -1) {
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, semijoinRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, r);
				q = pushArgument(mb, q, e);
				s = getArg(q, 0);
				pushInstruction(mb, q);

				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, semijoinRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, u);
				q = pushArgument(mb, q, g);
				v = getArg(q, 0);
				pushInstruction(mb, q);

				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, batRef);
				setFunctionId(q, reverseRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, v);
				c = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, semijoinRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, k);
				q = pushArgument(mb, q, c);
				k = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, batRef);
				setFunctionId(q, reverseRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, k);
				k = getArg(q, 0);
				pushInstruction(mb, q);

				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, semijoinRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, d);
				q = pushArgument(mb, q, k);
				i = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, batcalcRef);
				setFunctionId(q, putName("dbl", 3));
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, i);
				i = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, putName("kunion", 6));
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, e);
				q = pushArgument(mb, q, i);
				e = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, putName("kunion", 6));
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, *j4);
				q = pushArgument(mb, q, i);
				*j4 = getArg(q, 0);
				pushInstruction(mb, q);

				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, putName("kunion", 6));
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, s);
				q = pushArgument(mb, q, k);
				k = getArg(q, 0);
				pushInstruction(mb, q);

				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, batRef);
				setFunctionId(q, reverseRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, s);
				c = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, semijoinRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, l);
				q = pushArgument(mb, q, c);
				l = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, batRef);
				setFunctionId(q, reverseRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, l);
				l = getArg(q, 0);
				pushInstruction(mb, q);

				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, semijoinRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, f);
				q = pushArgument(mb, q, l);
				i = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, batcalcRef);
				setFunctionId(q, putName("dbl", 3));
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, i);
				i = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, putName("kunion", 6));
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, g);
				q = pushArgument(mb, q, i);
				g = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, putName("kunion", 6));
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, *j4);
				q = pushArgument(mb, q, i);
				*j4 = getArg(q, 0);
				pushInstruction(mb, q);

				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, putName("kunion", 6));
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, v);
				q = pushArgument(mb, q, l);
				l = getArg(q, 0);
				pushInstruction(mb, q);

				s = k;
				v = l;

				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, batRef);
				setFunctionId(q, reverseRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, v);
				v = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, joinRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, s);
				q = pushArgument(mb, q, v);
				s = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, batRef);
				setFunctionId(q, reverseRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, s);
				v = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, batRef);
				setFunctionId(q, mirrorRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, s);
				s = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, batRef);
				setFunctionId(q, mirrorRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, v);
				v = getArg(q, 0);
				pushInstruction(mb, q);

				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, joinRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, s);
				q = pushArgument(mb, q, e);
				s = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, joinRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, v);
				q = pushArgument(mb, q, g);
				v = getArg(q, 0);
				pushInstruction(mb, q);
			} else {
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, semijoinRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, e);
				q = pushArgument(mb, q, r);
				v = getArg(q, 0);
				pushInstruction(mb, q);

				if (f == -1) {
					q = newInstruction(mb, ASSIGNsymbol);
					setModuleId(q, batcalcRef);
					setFunctionId(q, putName("dbl", 3));
					q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
					q = pushArgument(mb, q, s);
					s = getArg(q, 0);
					pushInstruction(mb, q);
					q = newInstruction(mb, ASSIGNsymbol);
					setModuleId(q, algebraRef);
					setFunctionId(q, putName("kunion", 6));
					q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
					q = pushArgument(mb, q, v);
					q = pushArgument(mb, q, s);
					s = getArg(q, 0);
					pushInstruction(mb, q);
				} else {
					s = v;
				}

				v = g;
			}

			q = p;
			getArg(q, 1) = s;
			getArg(q, 2) = v;
			s = getArg(q, 0) = newTmpVariable(mb, TYPE_any);
			pushInstruction(mb, q);

			q = newInstruction(mb, ASSIGNsymbol);
			setModuleId(q, algebraRef);
			setFunctionId(q, joinRef);
			q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
			q = pushArgument(mb, q, j);
			q = pushArgument(mb, q, s);
			i = getArg(q, 0);
			pushInstruction(mb, q);
			q = newInstruction(mb, ASSIGNsymbol);
			setModuleId(q, algebraRef);
			setFunctionId(q, putName("kunion", 6));
			q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
			q = pushArgument(mb, q, *j4);
			q = pushArgument(mb, q, i);
			*j4 = getArg(q, 0);
			pushInstruction(mb, q);

			q = newInstruction(mb, ASSIGNsymbol);
			setModuleId(q, algebraRef);
			setFunctionId(q, projectRef);
			q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
			q = pushArgument(mb, q, j);
			q = pushChr(mb, q, 'n');
			r = getArg(q, 0);
			pushInstruction(mb, q);
			if (h != -1) {
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, projectRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, h);
				q = pushChr(mb, q, 'i');
				h = getArg(q, 0);
				pushInstruction(mb, q);
			}
			q = newInstruction(mb, ASSIGNsymbol);
			setModuleId(q, algebraRef);
			setFunctionId(q, projectRef);
			q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
			q = pushArgument(mb, q, i);
			q = pushChr(mb, q, 'd');
			i = getArg(q, 0);
			pushInstruction(mb, q);

			if (h != -1) {
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, putName("kunion", 6));
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, h);
				q = pushArgument(mb, q, i);
				h = getArg(q, 0);
				pushInstruction(mb, q);
			} else {
				h = i;
			}
			q = newInstruction(mb, ASSIGNsymbol);
			setModuleId(q, algebraRef);
			setFunctionId(q, putName("kdifference", 11));
			q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
			q = pushArgument(mb, q, r);
			q = pushArgument(mb, q, h);
			r = getArg(q, 0);
			pushInstruction(mb, q);
			q = newInstruction(mb, ASSIGNsymbol);
			setModuleId(q, algebraRef);
			setFunctionId(q, putName("kunion", 6));
			q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
			q = pushArgument(mb, q, r);
			q = pushArgument(mb, q, h);
			c = getArg(q, 0);
			pushInstruction(mb, q);
			q = newInstruction(mb, ASSIGNsymbol);
			setModuleId(q, algebraRef);
			setFunctionId(q, putName("kunion", 6));
			q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
			q = pushArgument(mb, q, *j1);
			q = pushArgument(mb, q, c);
			*j1 = getArg(q, 0);
			pushInstruction(mb, q);

			/* prepare return, new ids (head) with elem ids (tail) in
			 * original elems order */
			q = newInstruction(mb, ASSIGNsymbol);
			setModuleId(q, algebraRef);
			setFunctionId(q, markHRef);
			q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
			q = pushArgument(mb, q, b);
			q = pushArgument(mb, q, a);
			b = getArg(q, 0);
			pushInstruction(mb, q);
			q = newInstruction(mb, ASSIGNsymbol);
			setModuleId(q, batRef);
			setFunctionId(q, reverseRef);
			q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
			q = pushArgument(mb, q, b);
			b = getArg(q, 0);
			pushInstruction(mb, q);
			q = newInstruction(mb, ASSIGNsymbol);
			setModuleId(q, batRef);
			setFunctionId(q, mirrorRef);
			q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
			q = pushArgument(mb, q, c);
			c = getArg(q, 0);
			pushInstruction(mb, q);
			q = newInstruction(mb, ASSIGNsymbol);
			setModuleId(q, algebraRef);
			setFunctionId(q, leftjoinRef);
			q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
			q = pushArgument(mb, q, b);
			q = pushArgument(mb, q, c);
			a = getArg(q, 0);
			pushInstruction(mb, q);
			q = newInstruction(mb, ASSIGNsymbol);
			setModuleId(q, batRef);
			setFunctionId(q, reverseRef);
			q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
			q = pushArgument(mb, q, a);
			a = getArg(q, 0);
			pushInstruction(mb, q);

			return a;
		}
		case j_pair:
			b = dumpvariabletransformation(mb, t->tval1, elems,
					j1, j2, j3, j4, j5, j6, j7);
			/* only need to copy if tval1 is a var, otherwise we have a
			 * new copy already */
			if (t->tval1->type == j_var) {
				int *lv[] = {j2, j3, j4, j5, j6, NULL};
				int **lp = lv;

				/* jaql tool seems not to be able to return null in pairs,
				 * instead it shows empty pairs instead of null, but it
				 * however does for arrays, so we filter them out here if
				 * they weren't a real null (jaql tool crashes on that) */
				a = dumprefvar(mb, t->tval1, elems, *j1, *j6, *j7);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, batRef);
				setFunctionId(q, reverseRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, a);
				a = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, batRef);
				setFunctionId(q, reverseRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, b);
				c = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, putName("kintersect", 10));
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, c);
				q = pushArgument(mb, q, a);
				c = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, batRef);
				setFunctionId(q, reverseRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, c);
				b = getArg(q, 0);
				pushInstruction(mb, q);

				a = dumpnextid(mb, *j1);

				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, semijoinRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, *j1);
				q = pushArgument(mb, q, b);
				c = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, markHRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, c);
				q = pushArgument(mb, q, a);
				c = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, putName("kunion", 6));
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, *j1);
				q = pushArgument(mb, q, c);
				*j1 = getArg(q, 0);
				pushInstruction(mb, q);

				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, markTRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, b);
				q = pushArgument(mb, q, a);
				d = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, batRef);
				setFunctionId(q, reverseRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, d);
				d = getArg(q, 0);
				pushInstruction(mb, q);

				for (; *lp != NULL; lp++) {
					q = newInstruction(mb, ASSIGNsymbol);
					setModuleId(q, algebraRef);
					setFunctionId(q, semijoinRef);
					q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
					q = pushArgument(mb, q, **lp);
					q = pushArgument(mb, q, b);
					e = getArg(q, 0);
					pushInstruction(mb, q);
					q = newInstruction(mb, ASSIGNsymbol);
					setModuleId(q, algebraRef);
					setFunctionId(q, joinRef);
					q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
					q = pushArgument(mb, q, d);
					q = pushArgument(mb, q, e);
					e = getArg(q, 0);
					pushInstruction(mb, q);
					q = newInstruction(mb, ASSIGNsymbol);
					setModuleId(q, algebraRef);
					setFunctionId(q, putName("kunion", 6));
					q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
					q = pushArgument(mb, q, **lp);
					q = pushArgument(mb, q, e);
					**lp = getArg(q, 0);
					pushInstruction(mb, q);
				}

				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, markHRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, b);
				q = pushArgument(mb, q, a);
				b = getArg(q, 0);
				pushInstruction(mb, q);
			}

			/* add pair names */
			q = newInstruction(mb, ASSIGNsymbol);
			setModuleId(q, algebraRef);
			setFunctionId(q, projectRef);
			q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
			q = pushArgument(mb, q, b);
			q = pushStr(mb, q, t->sval);
			c = getArg(q, 0);
			pushInstruction(mb, q);
			q = newInstruction(mb, ASSIGNsymbol);
			setModuleId(q, algebraRef);
			setFunctionId(q, putName("kunion", 6));
			q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
			q = pushArgument(mb, q, *j7);
			q = pushArgument(mb, q, c);
			*j7 = getArg(q, 0);
			pushInstruction(mb, q);

			return b;
		case j_json_obj:
			g = 1;
		case j_json_arr:
			g++;

			f = dumpnextid(mb, *j1);
			q = newInstruction(mb, ASSIGNsymbol);
			setModuleId(q, algebraRef);
			setFunctionId(q, markHRef);
			q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
			q = pushArgument(mb, q, elems);
			q = pushArgument(mb, q, f);
			d = getArg(q, 0);
			pushInstruction(mb, q);
			q = newInstruction(mb, ASSIGNsymbol);
			setModuleId(q, algebraRef);
			setFunctionId(q, projectRef);
			q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
			q = pushArgument(mb, q, d);
			q = pushChr(mb, q, g == 2 ? 'o' : 'a');
			a = getArg(q, 0);
			pushInstruction(mb, q);
			q = newInstruction(mb, ASSIGNsymbol);
			setModuleId(q, algebraRef);
			setFunctionId(q, putName("kunion", 6));
			q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
			q = pushArgument(mb, q, *j1);
			q = pushArgument(mb, q, a);
			*j1 = getArg(q, 0);
			pushInstruction(mb, q);

			/* prepare return set with aligned elements */
			q = newInstruction(mb, ASSIGNsymbol);
			setModuleId(q, algebraRef);
			setFunctionId(q, markTRef);
			q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
			q = pushArgument(mb, q, elems);
			q = pushArgument(mb, q, f);
			d = getArg(q, 0);
			pushInstruction(mb, q);
			q = newInstruction(mb, ASSIGNsymbol);
			setModuleId(q, batRef);
			setFunctionId(q, reverseRef);
			q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
			q = pushArgument(mb, q, d);
			d = getArg(q, 0);
			pushInstruction(mb, q);

			c = -1;
			t = t->tval1;
			while (t != NULL) {
				b = dumpvariabletransformation(mb, t, elems,
						j1, j2, j3, j4, j5, j6, j7);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, batRef);
				setFunctionId(q, reverseRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, b);
				b = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, joinRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, d);
				q = pushArgument(mb, q, b);
				b = getArg(q, 0);
				pushInstruction(mb, q);
				if (c == -1) {
					c = b;
				} else {
					q = newInstruction(mb, ASSIGNsymbol);
					setModuleId(q, batRef);
					setFunctionId(q, insertRef);
					/* can't use sunion, this may have duplicates [$,$] */
					q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
					q = pushArgument(mb, q, c);
					q = pushArgument(mb, q, b);
					c = getArg(q, 0);
					pushInstruction(mb, q);
				}
				t = t->next;
			}

			if (c != -1) {
				/* can have duplicates, see above */
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, copyRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, g == 2 ? *j6 : *j5);
				a = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, batRef);
				setFunctionId(q, insertRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, a);
				q = pushArgument(mb, q, c);
				if (g == 2) {
					*j6 = getArg(q, 0);
				} else {
					*j5 = getArg(q, 0);
				}
				pushInstruction(mb, q);
			}

			return d;
		default:
			assert(0);
	}
}

static void
dumpgetvar(MalBlkPtr mb, const char *v, int *j1, int *j2, int *j3, int *j4, int *j5, int *j6, int *j7)
{
	InstrPtr q;

	q = newInstruction(mb, ASSIGNsymbol);
	setModuleId(q, putName("jaql", 4));
	setFunctionId(q, putName("getVar", 6));
	q = pushReturn(mb, q,
			newTmpVariable(mb, newBatType(TYPE_oid, TYPE_chr)));
	*j1 = getArg(q, 0);
	setVarUDFtype(mb, *j1);
	q = pushReturn(mb, q,
			newTmpVariable(mb, newBatType(TYPE_oid, TYPE_str)));
	*j2 = getArg(q, 1);
	setVarUDFtype(mb, *j2);
	q = pushReturn(mb, q,
			newTmpVariable(mb, newBatType(TYPE_oid, TYPE_lng)));
	*j3 = getArg(q, 2);
	setVarUDFtype(mb, *j3);
	q = pushReturn(mb, q,
			newTmpVariable(mb, newBatType(TYPE_oid, TYPE_dbl)));
	*j4 = getArg(q, 3);
	setVarUDFtype(mb, *j4);
	q = pushReturn(mb, q,
			newTmpVariable(mb, newBatType(TYPE_oid, TYPE_oid)));
	*j5 = getArg(q, 4);
	setVarUDFtype(mb, *j5);
	q = pushReturn(mb, q,
			newTmpVariable(mb, newBatType(TYPE_oid, TYPE_oid)));
	*j6 = getArg(q, 5);
	setVarUDFtype(mb, *j6);
	q = pushReturn(mb, q,
			newTmpVariable(mb, newBatType(TYPE_oid, TYPE_str)));
	*j7 = getArg(q, 6);
	setVarUDFtype(mb, *j7);

	q = pushStr(mb, q, v);
	pushInstruction(mb, q);
}

static json_var *
bindjsonvars(MalBlkPtr mb, tree *t)
{
	int i;
	tree *w;
	json_var *res;

	assert(t != NULL && t->type == j_join_input);

	for (w = t, i = 0; w != NULL; w = w->next, i++)
		;
	
	res = GDKmalloc(sizeof(json_var) * (i + 1));
	for (w = t, i = 0; w != NULL; w = w->next, i++) {
		res[i].name = w->tval2->sval; /* always _IDENT */
		res[i].preserve = w->nval;
		dumpgetvar(mb, w->tval1->sval,
				&res[i].j1, &res[i].j2, &res[i].j3, &res[i].j4,
				&res[i].j5, &res[i].j6, &res[i].j7);
	}
	res[i].name = NULL;

	return res;
}

static void
changetmplrefsjoin(tree *t)
{
	tree *w;

	for (w = t; w != NULL; w = w->next) {
		if (w->type == j_var) {
			/* inject an indirection to match the join output */
			tree *n = GDKzalloc(sizeof(tree));
			n->type = j_var;
			n->tval1 = w->tval1;
			w->tval1 = n;
			n->sval = w->sval;
			w->sval = GDKstrdup("$");
			continue;
		}
		if (w->tval1 != NULL)
			changetmplrefsjoin(w->tval1);
		if (w->tval2 != NULL)
			changetmplrefsjoin(w->tval2);
		if (w->tval3 != NULL)
			changetmplrefsjoin(w->tval3);
	}
}

int
dumptree(jc *j, MalBlkPtr mb, tree *t)
{
	InstrPtr q;
	int j1 = 0, j2 = 0, j3 = 0, j4 = 0, j5 = 0, j6 = 0, j7 = 0;
	int a, b, c, d, e, f, g;

	/* each iteration in this loop is a pipe (a JSON document)
	 * represented by the j1..7 vars */
	while (t != NULL) {
		switch (t->type) {
			case j_output_var:
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, putName("jaql", 4));
				setFunctionId(q, putName("setVar", 6));
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushStr(mb, q, t->sval);
				q = pushArgument(mb, q, j1);
				q = pushArgument(mb, q, j2);
				q = pushArgument(mb, q, j3);
				q = pushArgument(mb, q, j4);
				q = pushArgument(mb, q, j5);
				q = pushArgument(mb, q, j6);
				q = pushArgument(mb, q, j7);
				a = getArg(q, 0);
				pushInstruction(mb, q);
				break;
			case j_output:
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, ioRef);
				setFunctionId(q, putName("stdout", 6));
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				a = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, putName("json", 4));
				setFunctionId(q, printRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_void));
				q = pushArgument(mb, q, a);
				q = pushArgument(mb, q, j1);
				q = pushArgument(mb, q, j2);
				q = pushArgument(mb, q, j3);
				q = pushArgument(mb, q, j4);
				q = pushArgument(mb, q, j5);
				q = pushArgument(mb, q, j6);
				q = pushArgument(mb, q, j7);
				pushInstruction(mb, q);
				break;
			case j_json:
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, putName("json", 4));
				setFunctionId(q, putName("shred", 5));
				q = pushReturn(mb, q,
						newTmpVariable(mb, newBatType(TYPE_oid, TYPE_chr)));
				j1 = getArg(q, 0);
				setVarUDFtype(mb, j1);
				q = pushReturn(mb, q,
						newTmpVariable(mb, newBatType(TYPE_oid, TYPE_str)));
				j2 = getArg(q, 1);
				setVarUDFtype(mb, j2);
				q = pushReturn(mb, q,
						newTmpVariable(mb, newBatType(TYPE_oid, TYPE_lng)));
				j3 = getArg(q, 2);
				setVarUDFtype(mb, j3);
				q = pushReturn(mb, q,
						newTmpVariable(mb, newBatType(TYPE_oid, TYPE_dbl)));
				j4 = getArg(q, 3);
				setVarUDFtype(mb, j4);
				q = pushReturn(mb, q,
						newTmpVariable(mb, newBatType(TYPE_oid, TYPE_oid)));
				j5 = getArg(q, 4);
				setVarUDFtype(mb, j5);
				q = pushReturn(mb, q,
						newTmpVariable(mb, newBatType(TYPE_oid, TYPE_oid)));
				j6 = getArg(q, 5);
				setVarUDFtype(mb, j6);
				q = pushReturn(mb, q,
						newTmpVariable(mb, newBatType(TYPE_oid, TYPE_str)));
				j7 = getArg(q, 6);
				setVarUDFtype(mb, j7);

				q = pushStr(mb, q, t->sval);
				pushInstruction(mb, q);
				break;
			case j_var:
				/* j_var at top level is always _IDENT */
				dumpgetvar(mb, t->sval, &j1, &j2, &j3, &j4, &j5, &j6, &j7);
				break;
			case j_filter:
				a = dumpwalkvar(mb, j1, j5);
				b = dumppred(mb, t->tval2, a, &j1, &j2, &j3, &j4, &j5, &j6, &j7);
				/* b = matching ids from dumpwalkvar (first array) */
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, putName("kdifference", 11));
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, a);
				q = pushArgument(mb, q, b);
				a = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, projectRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, a);
				q = pushOid(mb, q, 0);
				a = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, batRef);
				setFunctionId(q, reverseRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, a);
				a = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, putName("sdifference", 11));
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, j5);
				q = pushArgument(mb, q, a);
				j5 = getArg(q, 0);
				pushInstruction(mb, q);
				break;
			case j_transform:
				a = dumpwalkvar(mb, j1, j5);
				b = dumpvariabletransformation(mb, t->tval2, a,
						&j1, &j2, &j3, &j4, &j5, &j6, &j7);

				/* remove old array entries */
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, projectRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, a);
				q = pushOid(mb, q, 0);
				g = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, batRef);
				setFunctionId(q, reverseRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, g);
				g = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, putName("sdifference", 11));
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, j5);
				q = pushArgument(mb, q, g);
				j5 = getArg(q, 0);
				pushInstruction(mb, q);

				/* construct new array members, respecting the old
				 * element ids order */
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, batRef);
				setFunctionId(q, reverseRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, b);
				d = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, joinRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, g);
				q = pushArgument(mb, q, d);
				e = getArg(q, 0);
				pushInstruction(mb, q);

				/* and insert them (j5 is result of sdiff above) */
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, batRef);
				setFunctionId(q, insertRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, e);
				q = pushArgument(mb, q, j5);
				j5 = getArg(q, 0);
				pushInstruction(mb, q);
				break;
			case j_expand:
				a = dumpwalkvar(mb, j1, j5);
				c = dumprefvar(mb, t->tval2, a, j1, j6, j7);

				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, semijoinRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, j1);
				q = pushArgument(mb, q, c);
				a = getArg(q, 0);
				pushInstruction(mb, q);

				/* immediately cleanup j1 */
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, putName("kdifference", 11));
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, j1);
				q = pushArgument(mb, q, c);
				j1 = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, batRef);
				setFunctionId(q, reverseRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, c);
				b = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, putName("kdifference", 11));
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, j1);
				q = pushArgument(mb, q, b);
				j1 = getArg(q, 0);
				pushInstruction(mb, q);

				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, uselectRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, a);
				q = pushChr(mb, q, 'a');  /* only arrays match expand */
				a = getArg(q, 0);
				pushInstruction(mb, q);
				/* construct json with these elements in the outermost
				 * array, reusing old bits */
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, semijoinRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, j5);
				q = pushArgument(mb, q, a);
				d = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, projectRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushOid(mb, q, 0);  /* 0@0 = outermost array */
				q = pushArgument(mb, q, d);
				b = getArg(q, 0);
				pushInstruction(mb, q);

				/* remove old arrays and 0@0 array from j5 */
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, putName("sdifference", 11));
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, j5);
				q = pushArgument(mb, q, d);
				j5 = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, putName("kdifference", 11));
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, j5);
				q = pushArgument(mb, q, b);
				j5 = getArg(q, 0);
				pushInstruction(mb, q);

				/* append to top-level array */
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, batRef);
				setFunctionId(q, insertRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, b);
				q = pushArgument(mb, q, j5);
				j5 = getArg(q, 0);
				pushInstruction(mb, q);
				break;
			case j_unroll:
				a = dumpwalkvar(mb, j1, j5);
				b = dumprefvar(mb, t->tval2, a, j1, j6, j7);
				e = dumpnextid(mb, j1);

				/* we only want the arrays from here */
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, semijoinRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, j1);
				q = pushArgument(mb, q, b);
				b = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, selectRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, b);
				q = pushChr(mb, q, 'a');
				c = getArg(q, 0);
				pushInstruction(mb, q);

				/* get parent(s), we only do objects for the moment */
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, joinRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, j6);
				q = pushArgument(mb, q, c);
				c = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, semijoinRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, j6);
				q = pushArgument(mb, q, c);
				c = getArg(q, 0);
				pushInstruction(mb, q);

				/* get elements from the to be expanded array */
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, semijoinRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, j5);
				q = pushArgument(mb, q, b);
				d = getArg(q, 0);
				pushInstruction(mb, q);

				/* generate kind elems for result (again, just objects) */
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, markHRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, d);
				q = pushArgument(mb, q, e);
				f = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, projectRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, f);
				q = pushChr(mb, q, 'o');
				f = getArg(q, 0);
				pushInstruction(mb, q);
				/* cleanup and append to kinds */
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, putName("kdifference", 11));
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, j1);
				q = pushArgument(mb, q, c);
				j1 = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, putName("kunion", 6));
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, j1);
				q = pushArgument(mb, q, f);
				j1 = getArg(q, 0);
				pushInstruction(mb, q);

				/* generate the outer array */
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, projectRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, f);
				q = pushOid(mb, q, 0); /* probably not correct */
				f = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, batRef);
				setFunctionId(q, reverseRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, f);
				f = getArg(q, 0);
				pushInstruction(mb, q);
				/* cleanup and append */
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, putName("kdifference", 11));
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, j5);
				q = pushArgument(mb, q, f);
				j5 = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, putName("kunion", 6));
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, f);
				q = pushArgument(mb, q, j5);
				j5 = getArg(q, 0);
				pushInstruction(mb, q);

				/* construct the objects themselves */
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, markTRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, d);
				q = pushArgument(mb, q, e);
				f = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, batRef);
				setFunctionId(q, reverseRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, f);
				f = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, batRef);
				setFunctionId(q, reverseRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, c);
				g = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, joinRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, f);
				q = pushArgument(mb, q, g);
				g = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, joinRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, g);
				q = pushArgument(mb, q, c);
				g = getArg(q, 0);
				pushInstruction(mb, q);

				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, markTRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, d);
				q = pushArgument(mb, q, e);
				f = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, batRef);
				setFunctionId(q, reverseRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, f);
				f = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, putName("sdifference", 11));
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, g);
				q = pushArgument(mb, q, f);
				g = getArg(q, 0);
				pushInstruction(mb, q);

				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, markHRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, d);
				q = pushArgument(mb, q, e);
				f = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, putName("sunion", 6));
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, g);
				q = pushArgument(mb, q, f);
				g = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, sortHTRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, g);
				g = getArg(q, 0);
				pushInstruction(mb, q);

				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, batRef);
				setFunctionId(q, reverseRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, d);
				f = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, joinRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, f);
				q = pushArgument(mb, q, j7);
				f = getArg(q, 0);
				pushInstruction(mb, q);
				
				/* cleanup and append */
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, putName("kdifference", 11));
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, j6);
				q = pushArgument(mb, q, c);
				j6 = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, putName("kunion", 6));
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, j6);
				q = pushArgument(mb, q, g);
				j6 = getArg(q, 0);
				pushInstruction(mb, q);

				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, putName("kdifference", 11));
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, j7);
				q = pushArgument(mb, q, d);
				j7 = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, putName("kdifference", 11));
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, j1);
				q = pushArgument(mb, q, d);
				j1 = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, putName("kunion", 6));
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, j7);
				q = pushArgument(mb, q, f);
				j7 = getArg(q, 0);
				pushInstruction(mb, q);

				break;
			case j_join: {
				json_var *js = bindjsonvars(mb, t->tval1);

				/* first calculate the join output, based on the input
				 * and predicates */
				dumppredjoin(mb, js, t, &j1, &j2, &j3, &j4, &j5, &j6 ,&j7);

				/* then transform the output with a modified into clause */
				changetmplrefsjoin(t->tval3);

				t->type = j_transform;
				freetree(t->tval1);
				freetree(t->tval2);
				t->tval2 = t->tval3;
				t->tval3 = NULL;
				t->tval1 = GDKzalloc(sizeof(tree));
				t->tval1->type = j_var;
				t->tval1->sval = GDKstrdup("$");

				continue;
			}
			case j_sort: {
				int l[4][2] = {{j2, 's'}, {j3, 'i'}, {j4, 'd'}, {0, 0}};
				int lw;
				a = dumpwalkvar(mb, j1, j5);
				b = dumprefvar(mb, t->tval2->tval1, a, j1, j6, j7);
				/* can only sort on one type (str, lng, dbl), and can't
				 * combine these, so pick first element's type and
				 * sort all of those */
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, batRef);
				setFunctionId(q, newRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushType(mb, q, TYPE_chr);
				q = pushType(mb, q, TYPE_chr);
				c = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, batRef);
				setFunctionId(q, insertRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, c);
				q = pushChr(mb, q, 's');
				q = pushChr(mb, q, 's');
				c = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, batRef);
				setFunctionId(q, insertRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, c);
				q = pushChr(mb, q, 'i');
				q = pushChr(mb, q, 'i');
				c = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, batRef);
				setFunctionId(q, insertRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, c);
				q = pushChr(mb, q, 'd');
				q = pushChr(mb, q, 'd');
				c = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, semijoinRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, j1);
				q = pushArgument(mb, q, b);
				e = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, joinRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, e);
				q = pushArgument(mb, q, c);
				e = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, putName("fetch", 5));
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, e);
				q = pushInt(mb, q, 0);
				f = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, uselectRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, e);
				q = pushArgument(mb, q, f);
				e = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				g = newTmpVariable(mb, newBatType(TYPE_oid, TYPE_oid));
				q = pushReturn(mb, q, g);
				q = pushNil(mb, q, newBatType(TYPE_oid, TYPE_oid));
				pushInstruction(mb, q);
				for (lw = 0; l[lw][0] != 0; lw++) {
					q = newInstruction(mb, ASSIGNsymbol);
					setModuleId(q, calcRef);
					setFunctionId(q, putName("==", 2));
					q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
					q = pushArgument(mb, q, f);
					q = pushChr(mb, q, l[lw][1]);
					d = getArg(q, 0);
					pushInstruction(mb, q);
					q = newAssignment(mb);
					q->barrier = BARRIERsymbol;
					pushArgument(mb, q, d);
					c = getArg(q, 0);
					q = newInstruction(mb, ASSIGNsymbol);
					setModuleId(q, algebraRef);
					setFunctionId(q, semijoinRef);
					q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
					q = pushArgument(mb, q, l[lw][0]);
					q = pushArgument(mb, q, e);
					d = getArg(q, 0);
					pushInstruction(mb, q);

					/* FIXME: ignore multiple sort args for now */
					q = newInstruction(mb, ASSIGNsymbol);
					setModuleId(q, algebraRef);
					if (t->tval2->nval == 1) {
						setFunctionId(q, sortTailRef);
					} else {
						setFunctionId(q, sortReverseTailRef);
					}
					q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
					q = pushArgument(mb, q, d);
					d = getArg(q, 0);
					pushInstruction(mb, q);
					q = newInstruction(mb, ASSIGNsymbol);
					setModuleId(q, batRef);
					setFunctionId(q, mirrorRef);
					q = pushReturn(mb, q, g);
					q = pushArgument(mb, q, d);
					pushInstruction(mb, q);

					q = newAssignment(mb);
					getArg(q, 0) = c;
					q->argc = q->retc = 1;
					q->barrier = EXITsymbol;
				}

				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, leftjoinRef); /* need to preserve order of g */
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, g);
				q = pushArgument(mb, q, b);
				g = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, batRef);
				setFunctionId(q, reverseRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, g);
				g = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, batRef);
				setFunctionId(q, mirrorRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, g);
				g = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, batRef);
				setFunctionId(q, reverseRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, j5);
				f = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, leftjoinRef); /* need to preserve order of g */
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, g);
				q = pushArgument(mb, q, f);
				g = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, batRef);
				setFunctionId(q, reverseRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, g);
				g = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, putName("sdifference", 11));
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, j5);
				q = pushArgument(mb, q, g);
				e = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, sunionRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, g);
				q = pushArgument(mb, q, e);
				j5 = getArg(q, 0);
				pushInstruction(mb, q);
			} break;
			case j_top:
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, putName("selectH", 7));
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, j1);
				q = pushOid(mb, q, 0);
				a = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, semijoinRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, j5);
				q = pushArgument(mb, q, a);
				a = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, sliceRef);
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, a);
				q = pushWrd(mb, q, 0);
				q = pushWrd(mb, q, (wrd)(t->nval - 1));
				b = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, putName("sdifference", 11));
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, a);
				q = pushArgument(mb, q, b);
				b = getArg(q, 0);
				pushInstruction(mb, q);
				q = newInstruction(mb, ASSIGNsymbol);
				setModuleId(q, algebraRef);
				setFunctionId(q, putName("sdifference", 11));
				q = pushReturn(mb, q, newTmpVariable(mb, TYPE_any));
				q = pushArgument(mb, q, j5);
				q = pushArgument(mb, q, b);
				j5 = getArg(q, 0);
				pushInstruction(mb, q);
				break;
			case j_error:
				snprintf(j->err, sizeof(j->err), "%s", t->sval);
				break;
			default:
				snprintf(j->err, sizeof(j->err), "unhandled type (1)");
				return -1;
		}
		t = t->next;
	}
	return -1;
}
