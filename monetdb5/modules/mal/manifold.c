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
 * Copyright August 2008-2013 MonetDB B.V.
 * All Rights Reserved.
 */

/*
 * M.L. Kersten
 */
#include "monetdb_config.h"
#include "manifold.h"
#include "mal_resolve.h"
#include "mal_builder.h"
#define _DEBUG_MANIFOLD_

/* The default iterator over known scalar commands.
 * It can be less efficient then the vector based implementations,
 * but saves quite some hacking in non-essential cases or
 * expensive user defined functions..
 *
 * To keep things simple and reasonably performant we limit the
 * implementation to those cases where a single BAT is returned.
 * Arguments may be of any type. The MAL signature should be a COMMAND.
 *
 */

typedef struct{
	BAT *b;
	void *first;
	void *last;
	int	size;
	int type;
	BATiter bi;
	BUN  o;
	str *s;
} MULTIarg;

typedef struct{
	Client cntxt;
	MalBlkPtr mb;
	MalStkPtr stk;
	InstrPtr pci;
	int fvar,lvar;
	MULTIarg *args;
} MULTItask;


#define ManifoldLoop(Type, ...) \
{ Type *v = (Type*) mut->args[0].first;\
	for( ; p< q && msg == MAL_SUCCEED; p += mut->args[mut->fvar].size){\
		msg = (*mut->pci->fcn)(v, __VA_ARGS__);\
		for( i = mut->fvar; i<= mut->lvar; i++)\
		if( mut->args[i].type < TYPE_str){\
			args[i] += mut->args[i].size;\
		} else {\
			mut->args[i].o++;\
			mut->args[i].s = (str *) BUNtail(mut->args[i].bi, mut->args[i].o);\
			args[i] = (void*)  &mut->args[i].s;\
		}\
		v++;\
	}\
}

#define Manifoldbody(...) \
switch(ATOMstorage(mut->args[0].b->ttype)){\
case TYPE_bit: ManifoldLoop(bit,__VA_ARGS__); break;\
case TYPE_sht: ManifoldLoop(sht,__VA_ARGS__); break;\
case TYPE_int: ManifoldLoop(int,__VA_ARGS__); break;\
case TYPE_lng: ManifoldLoop(lng,__VA_ARGS__); break;\
case TYPE_oid: ManifoldLoop(oid,__VA_ARGS__); break;\
case TYPE_flt: ManifoldLoop(flt,__VA_ARGS__); break;\
case TYPE_dbl: ManifoldLoop(dbl,__VA_ARGS__); break;\
case TYPE_str: { \
	for( ; p< q && msg == MAL_SUCCEED; p += mut->args[mut->fvar].size){\
		h = BUNhead(mut->args[0].bi, mut->args[0].o);\
		msg = (*mut->pci->fcn)(&y, __VA_ARGS__);\
		bunfastins(mut->args[0].b, (void*) h, (void*) y);\
		for( i = mut->fvar; i<= mut->lvar; i++)\
		if( mut->args[i].type < TYPE_str){\
			args[i] += mut->args[i].size;\
		} else {\
			mut->args[i].s = (str*) BUNtail(mut->args[i].bi, mut->args[i].o);\
			args[i] =  (void*) & mut->args[i].s; \
			mut->args[i].o++;\
		}\
		mut->args[0].o++;\
	}\
	break;}\
default:\
	msg= createException(MAL,"mal.manifold","manifold call limitation ");\
}

// single argument is preparatory step for GDK_mapreduce
static str
MANIFOLDjob(const MULTItask *mut)
{	int i;
	char *p, *q;
	char **args;
	str y, msg= MAL_SUCCEED;
	ptr h;

	args = (char**) GDKzalloc(sizeof(char*) * mut->pci->argc);
	if( args == NULL)
		throw(MAL,"mal.manifold",MAL_MALLOC_FAIL);
	
	for( i = mut->pci->retc+2; i< mut->pci->argc; i++)
	if ( mut->args[i].b ){
		if (mut->args[i].type < TYPE_str)
			args[i] = (char*) mut->args[i].first;
		else {
			mut->args[i].s = (str*) BUNtail(mut->args[i].bi, mut->args[i].o);
			args[i] =  (void*) & mut->args[i].s; 
		}
	} else
		args[i] = (char*) getArgReference(mut->stk,mut->pci,i);

	p = (char*)  mut->args[mut->fvar].first;
	q = (char*)  mut->args[mut->fvar].last;
	switch(mut->pci->argc){
	case 4: Manifoldbody(args[3]); break;
	case 5: Manifoldbody(args[3],args[4]); break;
	case 6: Manifoldbody(args[3],args[4],args[5]); break;
	case 7: Manifoldbody(args[3],args[4],args[5],args[6]); break;
	case 8: Manifoldbody(args[3],args[4],args[5],args[6],args[7]); break;
	default:
		msg= createException(MAL,"mal.manifold","manifold call limitation ");
	}
bunins_failed:
	GDKfree(args);
	return msg;
}

/* The manifold optimizer should check for the possibility
 * to use this implementation instead of the MAL loop.
 */
MALfcn
MANIFOLDtypecheck(Client cntxt, MalBlkPtr mb, MalStkPtr stk, InstrPtr pci){
	int i, k, tpe= 0;
	InstrPtr q=0;
	MalBlkPtr nmb;
	MALfcn fcn;
	(void) stk;

	// We need a private MAL context to resolve the function call
	nmb = newMalBlk(MAXVARS, STMT_INCREMENT);
	if( nmb == NULL)
		return NULL;
	q = newStmt(nmb,
		getVarConstant(mb,getArg(pci,pci->retc)).val.sval,
		getVarConstant(mb,getArg(pci,pci->retc+1)).val.sval);

	// Prepare the single result variable
	tpe =getTailType(getArgType(mb,pci,0));
	k= getArg(q,0) = newTmpVariable(nmb, tpe);
	setVarFixed(nmb,k);
	setVarUDFtype(nmb,k);
	
	// extract their argument type
	for ( i = pci->retc+2; i < pci->argc; i++){
		tpe = getTailType(getArgType(mb,pci,i));
		if (ATOMstorage(tpe) > TYPE_str){
			freeMalBlk(nmb);
			return NULL;
		}
		q= pushArgument(nmb,q, k= newTmpVariable(nmb, tpe));
		setVarFixed(nmb,k);
		setVarUDFtype(nmb,k);
	}

#ifdef _DEBUG_MANIFOLD_
	mnstr_printf(cntxt->fdout,"#MANIFOLD operation\n");
	printInstruction(cntxt->fdout,mb,stk,pci,LIST_MAL_ALL);
	printInstruction(cntxt->fdout,nmb,stk,q,LIST_MAL_ALL);
#endif
	// Localize the underlying opertor
	typeChecker(cntxt->fdout, cntxt->nspace, nmb, q, TRUE);
#ifdef _DEBUG_MANIFOLD_
	printInstruction(cntxt->fdout,nmb,stk,q,LIST_MAL_ALL);
#endif
	if ( nmb->errors || q->fcn == NULL || q->token != CMDcall)
		fcn = NULL;
	else
		fcn = q->fcn;
	freeMalBlk(nmb);
	return fcn;
}

str
MANIFOLDevaluate(Client cntxt, MalBlkPtr mb, MalStkPtr stk, InstrPtr pci){
	MULTItask mut;
	MULTIarg *mat;
	int i, tpe= 0;
	BUN cnt = 0;
	str msg = MAL_SUCCEED;
	MALfcn fcn;

	fcn= MANIFOLDtypecheck(cntxt,mb,stk,pci);
	if( fcn == NULL)
		throw(MAL, "mal.manifold", "Illegal manifold function call");

	mat = (MULTIarg *) GDKzalloc(sizeof(MULTIarg) * pci->argc);
	if( mat == NULL)
		throw(MAL, "mal.manifold", MAL_MALLOC_FAIL);
	
	// mr-job structure preparation
	mut.fvar = mut.lvar = 0;
	mut.cntxt= cntxt;
	mut.mb= mb;
	mut.stk= stk;
	mut.args= mat;
	mut.pci = pci;

	// prepare iterators
	for( i = pci->retc+2; i < pci->argc; i++){
		if ( isaBatType(getArgType(mb,pci,i)) ){
			mat[i].b = BATdescriptor( * (int*) getArgReference(stk,pci,i));
			if ( mat[i].b == NULL){
				msg = createException(MAL,"mal.manifold", MAL_MALLOC_FAIL);
				goto wrapup;
			}
			mat[i].type = tpe = mat[i].b->ttype;
			if ( mut.fvar == 0){
				mut.fvar = i;
				cnt = BATcount(mat[i].b);
			} else
			if ( BATcount(mat[i].b)!= cnt){
				msg = createException(MAL,"mal.manifold","Columns must be of same length");
				goto wrapup;
			} 
			mut.lvar = i;
			if ( tpe == TYPE_str) 
				mat[i].size = Tsize(mat[i].b);
			else
				mat[i].size = BATatoms[ ATOMstorage(tpe)].size;
			mat[i].first = (void*)  Tloc(mat[i].b, BUNfirst(mat[i].b));
			mat[i].last = (void*)  Tloc(mat[i].b, BUNlast(mat[i].b));
			mat[i].bi = bat_iterator(mat[i].b);
			mat[i].o = BUNfirst(mat[i].b);
		} else {
			mat[i].last = mat[i].first = (void*) getArgReference(stk,pci,i);
		}
	}

	// prepare result variable
	mat[0].b =BATnew(TYPE_void, getTailType(getArgType(mb,pci,0)), cnt);
	if ( mat[0].b == NULL){
		msg= createException(MAL,"mal.manifold",MAL_MALLOC_FAIL);
		goto wrapup;
	}
	mat[0].bi = bat_iterator(mat[0].b);
	mat[0].first = (void *)  Tloc(mat[0].b, BUNfirst(mat[0].b));
	mat[0].last = (void *)  Tloc(mat[0].b, BUNlast(mat[0].b));
	BATseqbase(mat[0].b,0);

	pci->fcn = fcn;

	// Then iterator over all BATs
	if( mut.fvar ==0){
		msg= createException(MAL,"mal.manifold","At least one column required");
		goto wrapup;
	}

	msg = MANIFOLDjob(&mut);

	// consolidate the properties
	BATsetcount(mat[0].b,cnt);
	mat[0].b->tkey= 0;
	mat[0].b->tsorted =0;
	mat[0].b->trevsorted = 0;
	mat[0].b->hkey= 0;
	mat[0].b->hsorted =0;
	mat[0].b->hrevsorted = 0;
	BATderiveProps(mat[0].b, TRUE);
	BBPkeepref(*(int*) getArgReference(stk,pci,0)=mat[0].b->batCacheid);
wrapup:
	// restore the argument types
	for( i = pci->retc; i < pci->argc; i++){
		if ( mat[i].b){
			BBPreleaseref(mat[i].b->batCacheid);
		}
	}
	GDKfree(mat);
	return msg;
}

// The old code
str
MANIFOLDremapMultiplex(Client cntxt, MalBlkPtr mb, MalStkPtr stk, InstrPtr p){
    char buf[BUFSIZ];
    (void) mb;
    (void) cntxt;
    snprintf(buf,BUFSIZ,"Function '%s.%s' not defined", (char *)getArgReference(stk,p,p->retc), (char *)getArgReference(stk,p,p->retc+1));
    throw(MAL, "opt.remap", "%s",buf);
}
