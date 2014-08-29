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
 * (c) Martin Kersten, Sjoerd Mullender
 * Series generating module for integer, decimal, real, double and timestamps.
 */

#include "monetdb_config.h"
#include "opt_prelude.h"
#include "algebra.h"
#include "generator.h"
#include "mtime.h"
#include "math.h"


/*
 * The noop simply means that we keep the properties for the generator object.
 */
#define VLTnoop(TPE)\
		{	TPE s;\
			s = pci->argc == 3 ? 1: *(TPE*) getArgReference(stk,pci, 3);\
			if( s == 0) zeroerror++;\
			if( s == TPE##_nil) nullerr++;\
		}
str
VLTgenerator_noop(Client cntxt, MalBlkPtr mb, MalStkPtr stk, InstrPtr pci)
{
	int nullerr= 0, zeroerror=0, tpe;
	(void) cntxt;

	switch( tpe = getArgType(mb,pci,1)){
	case TYPE_bte: VLTnoop(bte); break;
	case TYPE_sht: VLTnoop(sht); break;
	case TYPE_int: VLTnoop(int); break;
	case TYPE_lng: VLTnoop(lng); break;
	case TYPE_flt: VLTnoop(flt); break;
	case TYPE_dbl: VLTnoop(dbl); break;
	default:
	{	timestamp s;
		if (tpe == TYPE_timestamp){
			s = *(timestamp*) getArgReference(stk,pci, 3);
			if( timestamp_isnil(s)) nullerr++;
		} else throw(MAL,"generator.noop","unknown data type %d", getArgType(mb,pci,1));
	}
	}
	if( zeroerror)
		throw(MAL,"generator.noop","zero step size not allowed");
	if( nullerr)
		throw(MAL,"generator.noop","null step size not allowed");
	return MAL_SUCCEED;
}

/*
 * The base line consists of materializing the generator iterator value
 */
#define VLTmaterialize(TPE)						\
	do {								\
		TPE *v, f, l, s;					\
		f = *(TPE*) getArgReference(stk, pci, 1);		\
		l = *(TPE*) getArgReference(stk, pci, 2);		\
		if ( pci->argc == 3) \
			s = f<l? (TPE) 1: (TPE)-1;\
		else s =  *(TPE*) getArgReference(stk,pci, 3);\
		if (s == 0 || (s > 0 && f > l) || (s < 0 && f < l) || f == TPE##_nil || l == TPE##_nil)\
			throw(MAL, "generator.table",			\
			      "Illegal generator range");		\
		n = (BUN) ((l - f) / s);				\
		if ((TPE) (n * s + f) != l)				\
			n++;						\
		bn = BATnew(TYPE_void, TYPE_##TPE, n, TRANSIENT);	\
		if (bn == NULL)						\
			throw(MAL, "generator.table", MAL_MALLOC_FAIL);	\
		v = (TPE*) Tloc(bn, BUNfirst(bn));			\
		for (c = 0; c < n; c++)					\
			*v++ = (TPE) (f + c * s);			\
		bn->tsorted = s > 0 || n <= 1;				\
		bn->trevsorted = s < 0 || n <= 1;			\
	} while (0)

static str
VLTgenerator_table_(BAT **result, Client cntxt, MalBlkPtr mb, MalStkPtr stk, InstrPtr pci)
{
	BUN c, n;
	BAT *bn;
	str msg;
	int tpe;
	(void) cntxt;

	*result = NULL;
	tpe = getArgType(mb, pci, 1);
	switch (tpe) {
	case TYPE_bte:
		VLTmaterialize(bte);
		break;
	case TYPE_sht:
		VLTmaterialize(sht);
		break;
	case TYPE_int:
		VLTmaterialize(int);
		break;
	case TYPE_lng:
		VLTmaterialize(lng);
		break;
	case TYPE_flt:
		VLTmaterialize(flt);
		break;
	case TYPE_dbl:
		VLTmaterialize(dbl);
		break;
	default:
		if (tpe == TYPE_timestamp) {
			timestamp *v,f,l;
			lng s;
			ValRecord ret;
			if (VARcalccmp(&ret, &stk->stk[pci->argv[1]],
				       &stk->stk[pci->argv[2]]) == GDK_FAIL)
				throw(MAL, "generator.table",
				      "Illegal generator expression range");
			f = *(timestamp *) getArgReference(stk, pci, 1);
			l = *(timestamp *) getArgReference(stk, pci, 2);
			if ( pci->argc == 3) 
					throw(MAL,"generator.table","Timestamp step missing");
			s = *(lng *) getArgReference(stk, pci, 3);
			if (s == 0 ||
			    (s > 0 && ret.val.btval > 0) ||
			    (s < 0 && ret.val.btval < 0) ||
				timestamp_isnil(f) || timestamp_isnil(l))
				throw(MAL, "generator.table",
				      "Illegal generator range");
			/* casting one value to lng causes the whole
			 * computation to be done as lng, reducing the
			 * risk of overflow */
			n = (BUN) ((((lng) l.days - f.days) * 24*60*60*1000 + l.msecs - f.msecs) / s);
			bn = BATnew(TYPE_void, TYPE_timestamp, n + 1, TRANSIENT);
			if (bn == NULL)
				throw(MAL, "generator.table", MAL_MALLOC_FAIL);
			v = (timestamp *) Tloc(bn, BUNfirst(bn));
			for (c = 0; c < n; c++) {
				*v++ = f;
				msg = MTIMEtimestamp_add(&f, &f, &s);
				if (msg != MAL_SUCCEED) {
					BBPreclaim(bn);
					return msg;
				}
			}
			if (f.days != l.days || f.msecs != l.msecs) {
				*v++ = f;
				n++;
			}
			bn->tsorted = s > 0 || n <= 1;
			bn->trevsorted = s < 0 || n <= 1;
		} else {
			throw(MAL, "generator.table", "unsupported type");
		}
		break;
	}
	BATsetcount(bn, c);
	BATseqbase(bn, 0);
	bn->tkey = 1;
	bn->T->nil = 0;
	bn->T->nonil = 1;
	*result = bn;
	return MAL_SUCCEED;
}

str
VLTgenerator_table(Client cntxt, MalBlkPtr mb, MalStkPtr stk, InstrPtr pci)
{
	str msg;
	BAT *bn = NULL;

	if ((msg = VLTgenerator_noop(cntxt, mb, stk, pci)) != MAL_SUCCEED)
		return msg;

	msg =  VLTgenerator_table_(&bn, cntxt, mb, stk, pci);
	if( msg == MAL_SUCCEED){
		*(bat*) getArgReference(stk, pci, 0) = bn->batCacheid;
		BBPkeepref(bn->batCacheid);
	}
	return msg;
}

/*
 * Selection over the generator table does not require a materialization of the table
 * An optimizer can replace the subselect directly into a generator specific one.
 * The target to look for is generator.series(A1,A2,A3)
 * We need the generator parameters, which are injected to replace the target column
 */
static InstrPtr
findGeneratorDefinition(MalBlkPtr mb, InstrPtr pci, int target)
{
	InstrPtr q, p = NULL;
	int i;

	for (i = 1; i < mb->stop; i++) {
		q = getInstrPtr(mb, i);
		if (q->argv[0] == target && getModuleId(q) == generatorRef && getFunctionId(q) == parametersRef)
			p = q;
		if (q == pci)
			return p;
	}
	return p;
}

#define calculate_range(TPE, TPE2)					\
	do {								\
		TPE f, l, s, low, hgh;					\
									\
		f = * (TPE *) getArgReference(stk, p, 1);		\
		l = * (TPE *) getArgReference(stk, p, 2);		\
		if ( p->argc == 3) \
			s = f<l? (TPE) 1: (TPE)-1;\
		else s = * (TPE *) getArgReference(stk, p, 3); \
		if (s == 0 || (s > 0 && f > l) || (s < 0 && f < l) || f == TPE##_nil || l == TPE##_nil)	\
			throw(MAL, "generator.subselect",		\
			      "Illegal generator range");		\
		n = (BUN) (((TPE2) l - (TPE2) f) / (TPE2) s);		\
		if ((TPE)(n * s + f) != l)				\
			n++;						\
									\
		low = * (TPE *) getArgReference(stk, pci, i);		\
		hgh = * (TPE *) getArgReference(stk, pci, i + 1);	\
									\
		if (low == TPE##_nil && hgh == TPE##_nil) {		\
			if (li && hi && !anti) {			\
				/* match NILs (of which there aren't */	\
				/* any) */				\
				o1 = o2 = 0;				\
			} else {					\
				/* match all non-NIL values, */		\
				/* i.e. everything */			\
				o1 = 0;					\
				o2 = (oid) n;				\
			}						\
		} else if (s > 0) {					\
			if (low == TPE##_nil || low < f)		\
				o1 = 0;					\
			else {						\
				o1 = (oid) (((TPE2) low - (TPE2) f) / (TPE2) s); \
				if ((TPE) (f + o1 * s) < low ||			\
				    (!li && (TPE) (f + o1 * s) == low))		\
					o1++;				\
			}						\
			if (hgh == TPE##_nil)				\
				o2 = (oid) n;				\
			else if (hgh < f)				\
				o2 = 0;					\
			else {						\
				o2 = (oid) (((TPE2) hgh - (TPE2) f) / (TPE2) s); \
				if ((hi && (TPE) (f + o2 * s) == hgh) ||	\
				    (TPE) (f + o2 * s) < hgh)			\
					o2++;				\
			}						\
		} else {						\
			if (low == TPE##_nil)				\
				o2 = (oid) n;				\
			else if (low > f)				\
				o2 = 0;					\
			else {						\
				o2 = (oid) (((TPE2) low - (TPE2) f) / (TPE2) s); \
				if ((li && (TPE) (f + o2 * s) == low) ||	\
				    (TPE) (f + o2 * s) > low)			\
					o2++;				\
			}						\
			if (hgh == TPE##_nil || hgh > f)		\
				o1 = 0;					\
			else {						\
				o1 = (oid) (((TPE2) hgh - (TPE2) f) / (TPE2) s); \
				if ((!hi && (TPE) (f + o1 * s) == hgh) ||	\
				    (TPE) (f + o1 * s) > hgh)			\
					o1++;				\
			}						\
		}							\
	} while (0)

str
VLTgenerator_subselect(Client cntxt, MalBlkPtr mb, MalStkPtr stk, InstrPtr pci)
{
	bit li, hi, anti;
	int i;
	oid o1, o2;
	BUN n = 0;
	oid *cl = 0;
	BUN c;
	BAT *bn, *cand = NULL;
	InstrPtr p;
	str msg = MAL_SUCCEED;
	int tpe;

	(void) cntxt;
	p = findGeneratorDefinition(mb, pci, pci->argv[1]);
	if (p == NULL)
		throw(MAL, "generator.subselect",
		      "Could not locate definition for object");

	if (pci->argc == 8) {	/* candidate list included */
		bat candid = *(bat*) getArgReference(stk, pci, 2);
		if (candid) {
			cand = BATdescriptor(candid);
			if (cand == NULL)
				throw(MAL, "generator.subselect",
				      RUNTIME_OBJECT_MISSING);
			cl = (oid *) Tloc(cand, BUNfirst(cand));
		}
		i = 3;
	} else
		i = 2;

	li = * (bit *) getArgReference(stk, pci, i + 2);
	hi = * (bit *) getArgReference(stk, pci, i + 3);
	anti = * (bit *) getArgReference(stk, pci, i + 4);

	switch ( tpe = getArgType(mb, pci, i)) {
	case TYPE_bte: calculate_range(bte, int); break;
	case TYPE_sht: calculate_range(sht, int); break;
	case TYPE_int: calculate_range(int, lng); break;
	case TYPE_lng: calculate_range(lng, lng); break;
	case TYPE_flt: calculate_range(flt, dbl); break;
	case TYPE_dbl: calculate_range(dbl, dbl); break;
	default:
		if(  tpe == TYPE_timestamp){
			timestamp tsf,tsl;
			timestamp tlow,thgh;
			lng tss;
			oid *ol;

			tsf = *(timestamp *) getArgReference(stk, p, 1);
			tsl = *(timestamp *) getArgReference(stk, p, 2);
			if ( p->argc == 3) 
					throw(MAL,"generator.table","Timestamp step missing");
			tss = *(lng *) getArgReference(stk, p, 3);
			if ( tss == 0 || 
				timestamp_isnil(tsf) || timestamp_isnil(tsl) ||
				 (tss > 0 && (tsf.days > tsl.days || (tsf.days == tsl.days && tsf.msecs > tsl.msecs) )) ||
				 (tss < 0 && (tsf.days < tsl.days || (tsf.days == tsl.days && tsf.msecs < tsl.msecs) )) 
				)
				throw(MAL, "generator.subselect", "Illegal generator range");

			tlow = *(timestamp*) getArgReference(stk,pci,i);
			thgh = *(timestamp*) getArgReference(stk,pci,i+1);

			if( hi && !timestamp_isnil(thgh) )
				thgh.msecs++;
			if( !li && !timestamp_isnil(tlow) )
				tlow.msecs++;

			/* casting one value to lng causes the whole
			 * computation to be done as lng, reducing the
			 * risk of overflow */
			o2 = (BUN) ((((lng) tsl.days - tsf.days) * 24*60*60*1000 + tsl.msecs - tsf.msecs) / tss);
			bn = BATnew(TYPE_void, TYPE_oid, o2 + 1, TRANSIENT);
			if (bn == NULL)
				throw(MAL, "generator.subselect", MAL_MALLOC_FAIL);
			BATseqbase(bn,0);\

			// simply enumerate the sequence and filter it by predicate and candidate list
			ol = (oid *) Tloc(bn, BUNfirst(bn));
			for (c=0, o1=0; o1 <= o2; o1++) {
				if( (((tsf.days>tlow.days || (tsf.days== tlow.days && tsf.msecs >= tlow.msecs) || timestamp_isnil(tlow))) &&
				    ((tsf.days<thgh.days || (tsf.days== thgh.days && tsf.msecs < thgh.msecs))  || timestamp_isnil(thgh)) ) || anti ){
					/* could be improved when no candidate list is available into a void/void BAT */
					if( cl){
						while ( c < BATcount(cand) && *cl < o1 ) {cl++; c++;}
						if( *cl == o1){
							*ol++ = o1;
							cl++;
							n++;
							c++;
						}
					} else{
						*ol++ = o1;
						n++;
					}
				}
				msg = MTIMEtimestamp_add(&tsf, &tsf, &tss);
				if (msg != MAL_SUCCEED) {
					BBPreclaim(bn);
					return msg;
				}
			}
			BATsetcount(bn, (BUN) n);
			bn->tsorted = 1;
			bn->trevsorted = BATcount(bn) <= 1;
			bn->tkey = 1;
			bn->T->nil = 0;
			bn->T->nonil = 1;
			* (bat *) getArgReference(stk, pci, 0) = bn->batCacheid;
			BBPkeepref(bn->batCacheid);
			return MAL_SUCCEED;
		} else
			throw(MAL, "generator.subselect", "Unsupported type in subselect");
	}
	if (o1 > (oid) n)
		o1 = (oid) n;
	if (o2 > (oid) n)
		o2 = (oid) n;
	assert(o1 <= o2);
	assert(o2 - o1 <= (oid) n);
	if (anti && o1 == o2) {
		o1 = 0;
		o2 = (oid) n;
		anti = 0;
	}
	if (cand) {
		oid o;
		o = o1;
		o1 = SORTfndfirst(cand, &o);
		o = o2;
		o2 = SORTfndfirst(cand, &o);
		n = BATcount(cand);
		if (anti && o1 < o2) {
			bn = BATnew(TYPE_void, TYPE_oid, n - (o2 - o1), TRANSIENT);
			if (bn) {
				oid *op = (oid *) Tloc(bn, BUNfirst(bn));
				const oid *cp = (const oid *) Tloc(cand, BUNfirst(cand));
				BATsetcount(bn, n - (o2 - o1));
				BATseqbase(bn, 0);
				bn->T->nil = 0;
				bn->T->nonil = 1;
				bn->tsorted = 1;
				bn->trevsorted = BATcount(bn) <= 1;
				bn->tkey = 1;
				for (o = 0; o < o1; o++)
					*op++ = cp[o];
				for (o = o2; o < (oid) n; o++)
					*op++ = cp[o];
			}
		} else {
			if (anti) {
				o1 = 0;
				o2 = (oid) n;
			}
			bn = BATslice(cand, (BUN) o1, (BUN) o2);
		}
		BBPreleaseref(cand->batCacheid);
		if (bn == NULL)
			throw(MAL, "generator.subselect",
			      MAL_MALLOC_FAIL);
	} else {
		if (anti) {
			oid o;
			oid *op;

			bn = BATnew(TYPE_void, TYPE_oid, n - (o2 - o1), TRANSIENT);
			if (bn == NULL)
				throw(MAL, "generator.subselect",
				      MAL_MALLOC_FAIL);
			BATsetcount(bn, n - (o2 - o1));
			BATseqbase(bn, 0);
			op = (oid *) Tloc(bn, BUNfirst(bn));
			for (o = 0; o < o1; o++)
				*op++ = o;
			for (o = o2; o < (oid) n; o++)
				*op++ = o;
			bn->T->nil = 0;
			bn->T->nonil = 1;
			bn->tsorted = 1;
			bn->trevsorted = BATcount(bn) <= 1;
			bn->tkey = 1;
		} else {
			bn = BATnew(TYPE_void, TYPE_void, (BUN) (o2 - o1), TRANSIENT);
			if (bn == NULL)
				throw(MAL, "generator.subselect",
				      MAL_MALLOC_FAIL);
			BATsetcount(bn, o2 - o1);
			BATseqbase(bn, 0);
			BATseqbase(BATmirror(bn), o1);
		}
	}
	* (bat *) getArgReference(stk, pci, 0) = bn->batCacheid;
	BBPkeepref(bn->batCacheid);
	return MAL_SUCCEED;
}
#ifdef _MSC_VER
#define nextafter   _nextafter
float nextafterf(float x, float y);
#endif

#define PREVVALUEbte(x) ((x) - 1)
#define PREVVALUEsht(x) ((x) - 1)
#define PREVVALUEint(x) ((x) - 1)
#define PREVVALUElng(x) ((x) - 1)
#define PREVVALUEoid(x) ((x) - 1)
#define PREVVALUEflt(x) nextafterf((x), -GDK_flt_max)
#define PREVVALUEdbl(x) nextafter((x), -GDK_dbl_max)

#define NEXTVALUEbte(x) ((x) + 1)
#define NEXTVALUEsht(x) ((x) + 1)
#define NEXTVALUEint(x) ((x) + 1)
#define NEXTVALUElng(x) ((x) + 1)
#define NEXTVALUEoid(x) ((x) + 1)
#define NEXTVALUEflt(x) nextafterf((x), GDK_flt_max)
#define NEXTVALUEdbl(x) nextafter((x), GDK_dbl_max)


#define VLTthetasubselect(TPE,ABS) {\
	TPE f,l,s, low, hgh;\
	BUN j; oid *v;\
	f = *(TPE*) getArgReference(stk,p, 1);\
	l = *(TPE*) getArgReference(stk,p, 2);\
	if ( p->argc == 3) \
		s = f<l? (TPE) 1: (TPE)-1;\
	else s =  *(TPE*) getArgReference(stk,p, 3);\
	if( s == 0 || (f<l && s < 0) || (f>l && s> 0)) \
		throw(MAL,"generator.thetasubselect","Illegal range");\
	cap = (BUN)(ABS(l-f)/ABS(s));\
	bn = BATnew(TYPE_void, TYPE_oid, cap, TRANSIENT);\
	if( bn == NULL)\
		throw(MAL,"generator.thetasubselect",MAL_MALLOC_FAIL);\
	BATseqbase(bn,0);\
	low= hgh = TPE##_nil;\
	v = (oid*) Tloc(bn,BUNfirst(bn));\
	if ( strcmp(oper,"<") == 0){\
		hgh= *(TPE*) getArgReference(stk,pci,idx);\
		hgh = PREVVALUE##TPE(hgh);\
	} else\
	if ( strcmp(oper,"<=") == 0){\
		hgh= *(TPE*) getArgReference(stk,pci,idx);\
	} else\
	if ( strcmp(oper,">") == 0){\
		low= *(TPE*) getArgReference(stk,pci,idx);\
		low = NEXTVALUE##TPE(low);\
	} else\
	if ( strcmp(oper,">=") == 0){\
		low= *(TPE*) getArgReference(stk,pci,idx);\
	} else\
	if ( strcmp(oper,"!=") == 0){\
		hgh= low= *(TPE*) getArgReference(stk,pci,idx);\
		anti++;\
	} else\
	if ( strcmp(oper,"==") == 0){\
		hgh= low= *(TPE*) getArgReference(stk,pci,idx);\
	} else\
		throw(MAL,"generator.thetasubselect","Unknown operator");\
	if(cand){ cn = BATcount(cand); if( cl == 0) oc = cand->tseqbase; }\
	for(j=0;j<cap;j++, f+=s, o++)\
		if( ((low == TPE##_nil || f >= low) && (f <= hgh || hgh == TPE##_nil)) || anti){\
			if(cand){ \
				if( cl){ while(cn-- >= 0 && *cl < o) cl++; if ( *cl == o){ *v++= o; c++;}} \
				else { while(cn-- >= 0 && oc < o) oc++; if ( oc == o){ *v++= o; c++;} }\
			} else {*v++ = o; c++;}\
		} \
}


str VLTgenerator_thetasubselect(Client cntxt, MalBlkPtr mb, MalStkPtr stk, InstrPtr pci)
{
	int idx, cndid =0, c= 0, anti =0,tpe;
	BAT *cand = 0, *bn = NULL;
	BUN cap,j;
	lng cn= 0;
	oid o = 0, oc = 0,  *cl = 0;
	InstrPtr p;
	str oper, msg= MAL_SUCCEED;

	(void) cntxt;
	p = findGeneratorDefinition(mb,pci,pci->argv[1]);
	if( p == NULL)
		throw(MAL,"generator.thetasubselect","Could not locate definition for object");

	if( pci->argc == 5){ // candidate list included
		cndid = *(int*) getArgReference(stk,pci, 2);
		if( cndid){
			cand = BATdescriptor(cndid);
			if( cand == NULL)
				throw(MAL,"generator.subselect",RUNTIME_OBJECT_MISSING);
			cl = (oid*) Tloc(cand,BUNfirst(cand));\
		} 
		idx = 3;
	} else idx = 2;
	oper= *(str*) getArgReference(stk,pci,idx+1);

	// check the step direction
	
	switch( tpe =getArgType(mb,pci,idx)){
	case TYPE_bte: VLTthetasubselect(bte,abs);break;
	case TYPE_int: VLTthetasubselect(int,abs);break;
	case TYPE_sht: VLTthetasubselect(sht,abs);break;
	case TYPE_lng: VLTthetasubselect(lng,llabs);break;
	case TYPE_flt: VLTthetasubselect(flt,fabsf);break;
	case TYPE_dbl: VLTthetasubselect(dbl,fabs);break;
	break;
	default:
		if ( tpe == TYPE_timestamp){
			timestamp f,l, val, low, hgh;
			lng  s;
			oid *v;

			f = *(timestamp*) getArgReference(stk,p, 1);
			l = *(timestamp*) getArgReference(stk,p, 2);
			if ( p->argc == 3) 
					throw(MAL,"generator.table","Timestamp step missing");
			s = *(lng*) getArgReference(stk,p, 3);
			if ( s == 0 || 
				 (s > 0 && (f.days > l.days || (f.days == l.days && f.msecs > l.msecs) )) ||
				 (s < 0 && (f.days < l.days || (f.days == l.days && f.msecs < l.msecs) )) 
				)
				throw(MAL, "generator.subselect", "Illegal generator range");

			hgh = low = *timestamp_nil;
			if ( strcmp(oper,"<") == 0){
				hgh= *(timestamp*) getArgReference(stk,pci,idx);
			} else
			if ( strcmp(oper,"<=") == 0){
				hgh= *(timestamp*) getArgReference(stk,pci,idx) ;
				hgh.msecs++;
			} else
			if ( strcmp(oper,">") == 0){
				low= *(timestamp*) getArgReference(stk,pci,idx);
				low.msecs++;
			} else
			if ( strcmp(oper,">=") == 0){
				low= *(timestamp*) getArgReference(stk,pci,idx);
			} else
			if ( strcmp(oper,"!=") == 0){
				hgh= low= *(timestamp*) getArgReference(stk,pci,idx);
				anti++;
			} else
			if ( strcmp(oper,"==") != 0)
				throw(MAL,"generator.thetasubselect","Unknown operator");

			cap = (BUN) ((((lng) l.days - f.days) * 24*60*60*1000 + l.msecs - f.msecs) / s);
			bn = BATnew(TYPE_void, TYPE_oid, cap, TRANSIENT);
			if( bn == NULL)
				throw(MAL,"generator.thetasubselect",MAL_MALLOC_FAIL);
			v = (oid*) Tloc(bn,BUNfirst(bn));
			BATseqbase(bn,0);

			if(cand){ cn = BATcount(cand); if( cl == 0) oc = cand->tseqbase; }
			val = f;
			for(j = 0; j< cap; j++,  o++){
				if( (( timestamp_isnil(low) || (val.days > low.days || (val.days == low.days && val.msecs >=low.msecs))) && 
					 ( timestamp_isnil(hgh) || (val.days < hgh.days || (val.days == hgh.days && val.msecs < hgh.msecs)))) || anti){
					if(cand){
						if( cl){ while(cn-- >= 0 && *cl < o) cl++; if ( *cl == o){ *v++= o; c++;}}
						else { while(cn-- >= 0 && oc < o) oc++; if ( oc == o){ *v++= o; c++;} }
					} else {*v++ = o; c++;}
				}
				if( (msg = MTIMEtimestamp_add(&val, &val, &s)) != MAL_SUCCEED)
					goto wrapup;
			}
		} else
			throw(MAL,"generator.thetasubselect","Illegal generator arguments");
	}

wrapup:
	if( cndid)
		BBPreleaseref(cndid);
	if( bn){
		BATsetcount(bn,c);
		bn->hdense = 1;
		bn->hseqbase = 0;
		bn->hkey = 1;
		BATderiveProps(bn,0);
		BBPkeepref(*(int*)getArgReference(stk,pci,0)= bn->batCacheid);
	}
	return msg;
}

#define VLTleftfetchjoin(TPE) {\
	TPE f,l,s, val;\
	TPE *v;\
	f = *(TPE*) getArgReference(stk,p, 1);\
	l = *(TPE*) getArgReference(stk,p, 2);\
	if ( p->argc == 3) \
		s = f<l? (TPE) 1: (TPE)-1;\
	else s = * (TPE *) getArgReference(stk, p, 3); \
	if ( s == 0 || (f> l && s>0) || (f<l && s < 0))\
		throw(MAL,"generator.leftfetchjoin","Illegal range");\
	bn = BATnew(TYPE_void, TYPE_##TPE, cnt, TRANSIENT);\
	if( bn == NULL){\
		BBPreleaseref(bid);\
		throw(MAL,"generator.leftfetchjoin",MAL_MALLOC_FAIL);\
	}\
	v = (TPE*) Tloc(bn,BUNfirst(bn));\
	for(; cnt-- > 0; ol++, o++){\
		val = f + ((TPE) ( b->ttype == TYPE_void?o:*ol)) * s;\
		if ( (s > 0 &&  (val < f || val >= l)) || (s < 0 && (val<l || val >=f))) \
			continue;\
		*v++ = val;\
		c++;\
	}\
}

str VLTgenerator_leftfetchjoin(Client cntxt, MalBlkPtr mb, MalStkPtr stk, InstrPtr pci)
{
	int bid =0, c= 0, tpe, *ret;
	BAT *b, *bn = NULL;
	BUN cnt;
	oid *ol =0, o= 0;
	InstrPtr p;
	str msg;

	(void) cntxt;
	p = findGeneratorDefinition(mb,pci,pci->argv[2]);

	ret = (int*) getArgReference(stk,pci,0);
	b = BATdescriptor(bid = *(int*) getArgReference(stk,pci,1));
	if( b == NULL)
		throw(MAL,"generator.leftfetchjoin",RUNTIME_OBJECT_MISSING);

	// if it does not exist we should fall back to the ordinary leftfetchjoin to try
	// it might have been materialized already
	if( p == NULL){
		bn = BATdescriptor( *(int*) getArgReference(stk,pci,2));
		if( bn == NULL)
			throw(MAL,"generator.leftfetchjoin",RUNTIME_OBJECT_MISSING);
		msg = ALGleftfetchjoin(ret, &b->batCacheid, &bn->batCacheid);
		return msg;
	}

	cnt = BATcount(b);
	if ( b->ttype == TYPE_void)
		o = b->tseqbase;
	else
		ol = (oid*) Tloc(b,BUNfirst(b));

	/* the actual code to perform a leftfetchjoin over generators */
	switch( tpe = getArgType(mb,p,1)){
	case TYPE_bte:  VLTleftfetchjoin(bte); break;
	case TYPE_sht:  VLTleftfetchjoin(sht); break;
	case TYPE_int:  VLTleftfetchjoin(int); break;
	case TYPE_lng:  VLTleftfetchjoin(lng); break;
	case TYPE_flt:  VLTleftfetchjoin(flt); break;
	case TYPE_dbl:  VLTleftfetchjoin(dbl); break;
	default:
		if ( tpe == TYPE_timestamp){
			timestamp f,l, val;
			lng s,t;
			timestamp *v;
			f = *(timestamp*) getArgReference(stk,p, 1);
			l = *(timestamp*) getArgReference(stk,p, 2);
			if ( p->argc == 3) 
					throw(MAL,"generator.table","Timestamp step missing");
			s =  *(lng*) getArgReference(stk,p, 3);
			if ( s == 0 ||
				(s< 0 &&	(f.days< l.days || (f.days == l.days && f.msecs < l.msecs))) ||
				(s> 0 &&	(l.days< f.days || (l.days == f.days && l.msecs < f.msecs))) )
				throw(MAL,"generator.leftfetchjoin","Illegal range");

			bn = BATnew(TYPE_void, TYPE_timestamp, cnt, TRANSIENT);
			if( bn == NULL){
				BBPreleaseref(bid);
				throw(MAL,"generator.leftfetchjoin",MAL_MALLOC_FAIL);
			}

			v = (timestamp*) Tloc(bn,BUNfirst(bn));

			for(; cnt-- > 0; ol++, o++){
				t = ((lng) ( b->ttype == TYPE_void?o:*ol)) * s;
				if( (msg = MTIMEtimestamp_add(&val, &f, &t)) != MAL_SUCCEED)
					return msg;

				if ( timestamp_isnil(val))
					continue;
				if (s > 0 && ((val.days < f.days || (val.days == f.days && val.msecs < f.msecs)) || ((val.days>l.days || (val.days== l.days && val.msecs >= l.msecs)))  ) )
					continue;
				if (s < 0 && ((val.days < l.days || (val.days == l.days && val.msecs < l.msecs)) || ((val.days>f.days || (val.days== f.days && val.msecs >= f.msecs)))  ) )
					continue;
				*v++ = val;
				c++;
			}
		}
	}

	/* adminstrative wrapup of the leftfetchjoin */
	BBPreleaseref(bid);
	if( bn){
		BATsetcount(bn,c);
		bn->hdense = 1;
		bn->hseqbase = 0;
		bn->hkey = 1;
		BATderiveProps(bn,0);
		BBPkeepref(*(int*)getArgReference(stk,pci,0)= bn->batCacheid);
	}
	return MAL_SUCCEED;
}

/* The operands of a join operation can either be defined on a generator */
#define VLTjoin(TPE, ABS) \
	{ TPE f,l,s; TPE *v; BUN w;\
	f = *(TPE*) getArgReference(stk,p, 1);\
	l = *(TPE*) getArgReference(stk,p, 2);\
	if ( p->argc == 3) \
		s = f<l? (TPE) 1: (TPE)-1;\
	else s = * (TPE *) getArgReference(stk, p, 3); \
	incr = s > 0;\
	v = (TPE*) Tloc(bl,BUNfirst(bl));\
	if ( s == 0 || (f> l && s>0) || (f<l && s < 0))\
		throw(MAL,"generator.join","Illegal range");\
	for( ; cnt >0; cnt--,o++,v++){\
		w = (BUN) floor( (double)(ABS(*v -f)/ABS(s)));\
		if ( f + (TPE)(w * s) == *v ){\
			*ol++ = (oid) w;\
			*or++ = o;\
			c++;\
		}\
	} }\

str VLTgenerator_join(Client cntxt, MalBlkPtr mb, MalStkPtr stk, InstrPtr pci)
{
	BAT  *b, *bl = NULL, *br = NULL, *bln = NULL, *brn= NULL;
	BUN cnt,c =0;
	oid o= 0, *ol, *or;
	int tpe, incr=0;
	InstrPtr p = NULL, q = NULL;
	str msg = MAL_SUCCEED;

	(void) cntxt;
	// we assume at most one of the arguments to refer to the generator
	p = findGeneratorDefinition(mb,pci,pci->argv[2]);
	if( p == NULL){
		bl = BATdescriptor(*(int*) getArgReference(stk,pci,2));
		if( bl == NULL)
			throw(MAL,"generator.join",RUNTIME_OBJECT_MISSING);
	}
	q = findGeneratorDefinition(mb,pci,pci->argv[3]);
	if ( q == NULL){
		br = BATdescriptor(*(int*) getArgReference(stk,pci,3));
		if( br == NULL){
			BBPreleaseref(bl->batCacheid);
			throw(MAL,"generator.join",RUNTIME_OBJECT_MISSING);
		}
	}

	// in case of both generators  || getModuleId(q) == generatorRef)materialize the 'smallest' one first
	// or implement more knowledge, postponed
	if (p && q ){
		msg =  VLTgenerator_table_(&bl, cntxt, mb, stk, p);
		if( msg || bl == NULL )
			throw(MAL,"generator.join","Join over generator pairs not supported");
		else
			p = NULL;
	}

	// switch roles to have a single target bat[:oid,:any] designated 
	// by b and reference instruction p for the generator
	b = q? bl : br;
	p = q? q : p;
	cnt = BATcount(b);
	tpe = b->ttype;
	o= b->hseqbase;
	
	bln = BATnew(TYPE_void,TYPE_oid, cnt, TRANSIENT);
	brn = BATnew(TYPE_void,TYPE_oid, cnt, TRANSIENT);
	if( bln == NULL || brn == NULL){
		if(bln) BBPreleaseref(bln->batCacheid);
		if(brn) BBPreleaseref(brn->batCacheid);
		if(bl) BBPreleaseref(bl->batCacheid);
		if(br) BBPreleaseref(br->batCacheid);
		throw(MAL,"generator.join",MAL_MALLOC_FAIL);
	}
	ol = (oid*) Tloc(bln,BUNfirst(bln));
	or = (oid*) Tloc(brn,BUNfirst(brn));

	/* The actual join code for generators be injected here */
	switch(tpe){
	case TYPE_bte: //VLTjoin(bte,abs); break; 
	{ bte f,l,s; bte *v; BUN w;
	f = *(bte*) getArgReference(stk,p, 1);
	l = *(bte*) getArgReference(stk,p, 2);
	s = *(bte*) getArgReference(stk,p, 3);
	incr = s > 0;
	if ( s == 0 || (f> l && s>0) || (f<l && s < 0))
		throw(MAL,"generator.join","Illegal range");
	v = (bte*) Tloc(b,BUNfirst(b));
	for( ; cnt >0; cnt--,o++,v++){
		w = (BUN) floor(abs(*v -f)/abs(s));
		if ( f + (bte)( w * s) == *v ){
			*ol++ = (oid) w;
			*or++ = o;
			c++;
		}
	} }
	break;
	case TYPE_sht: VLTjoin(sht,abs); break;
	case TYPE_int: VLTjoin(int,abs); break;
	case TYPE_lng: VLTjoin(lng,llabs); break;
	case TYPE_flt: VLTjoin(flt,fabsf); break;
	case TYPE_dbl: VLTjoin(dbl,fabs); break;
	default:
		if( tpe == TYPE_timestamp){ 
			// it is easier to produce the timestamp series
			// then to estimate the possible index
			}
		throw(MAL,"generator.join","Illegal type");
	}

	BATsetcount(bln,c);
	bln->hdense = 1;
	bln->hseqbase = 0;
	bln->hkey = 1;
	bln->tsorted = incr || c <= 1;				\
	bln->trevsorted = !incr || c <= 1;			\
	BATderiveProps(bln,0);
	
	BATsetcount(brn,c);
	brn->hdense = 1;
	brn->hseqbase = 0;
	brn->hkey = 1;
	brn->tsorted = incr || c <= 1;				\
	brn->trevsorted = !incr || c <= 1;			\
	BATderiveProps(brn,0);
	if( q){
		BBPkeepref(*(int*)getArgReference(stk,pci,0)= brn->batCacheid);
		BBPkeepref(*(int*)getArgReference(stk,pci,1)= bln->batCacheid);
	} else {
		BBPkeepref(*(int*)getArgReference(stk,pci,0)= bln->batCacheid);
		BBPkeepref(*(int*)getArgReference(stk,pci,1)= brn->batCacheid);
	}
	return msg;
}
