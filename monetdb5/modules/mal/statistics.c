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
/*
 * author M.L. Kersten
 * Statistics box.
 * Most optimizers need easy access to key information
 * for proper plan generation. Amongst others, this
 * volatile information consists of the tuple count, size,
 * min- and max-value, the null-density, and
 * a histogram of the value distribution.
 *
 * The statistics are management by a Box, which gives
 * a controlled environment to manage a collection of BATs
 * and system variables.
 *
 * BATs have to be deposit into the  statistics box
 * separately, because the costs attached maintaining them are high.
 * The consistency of the statistics box is partly
 * the responsibility of the upper layers. There is
 * no automatic triggering when the BATs referenced
 * are heavily modified or are being destroyed.
 * They disappear from the statistics box the first time
 * an invalid access is attempted or during system reboot.
 *
 * The staleness of the information can be controlled in several
 * ways. The easiest, and most expensive, is to assure that
 * the statistics are updated when you start the server.
 * Alternative, you can set a expiration interval, which will
 * update the information only when it is considered expired.
 * This test will be triggered either at server restart or
 * your explicit call to update the statistics tables.
 * The statistics table is commited each time you change it.
 *
 * A forced update can be called upon when the front-end
 * expects the situation to be changed drastically.
 *
 * The statistics table is mostly used internally, but
 * once in a while you need a dump for closed inspection.
 * in your MAL program for inspection. Just
 * use the BBP bind operation to locate them in the buffer pool.
 */
#include "monetdb_config.h"
#include "statistics.h"
#include "algebra.h"

BAT *STAT_id_inuse;		/* BATs information taken from the box */
BAT *STAT_id_nme;		/* mapping from BBP index */
BAT *STAT_id_expire;
BAT *STAT_id_stamp;		/* BAT last time stamp */
BAT *STAT_id_count;
BAT *STAT_id_size;
BAT *STAT_id_min_lng;
BAT *STAT_id_max_lng;
BAT *STAT_id_histogram;

/*
 * The statistics are currently limited to the server session.
 * Upon need we can turn it into a persistent mode.
 */
static int statisticsMode= TRANSIENT;

static BAT *
STAT_create(str hnme, str tnme, int ht, int tt)
{
	BAT *b;
	char buf[128];

	snprintf(buf, 128, "stat_%s_%s", hnme, tnme);
	b = BATdescriptor(BBPindex(buf));
	if (b)
		return b;

	b = BATnew(ht, tt, 256);
	if (b == NULL)
		return NULL;

	BATkey(b, TRUE);
	BBPrename(b->batCacheid, buf);
	BATmode(b, statisticsMode);
#ifdef DEBUG_STATISTICS
	printf("created %s\n", buf);
#endif
	return b;
}

static void
STATcommit(void)
{
	bat b[10];

	b[0] = 0;
	b[1] = ABS(STAT_id_inuse->batCacheid);
	b[2] = ABS(STAT_id_nme->batCacheid);
	b[3] = ABS(STAT_id_expire->batCacheid);
	b[4] = ABS(STAT_id_stamp->batCacheid);
	b[5] = ABS(STAT_id_count->batCacheid);
	b[6] = ABS(STAT_id_size->batCacheid);
	b[7] = ABS(STAT_id_min_lng->batCacheid);
	b[8] = ABS(STAT_id_max_lng->batCacheid);
	b[9] = ABS(STAT_id_histogram->batCacheid);
	TMsubcommit_list(b, 10);
}

static void
STATinit(void)
{
	if( STAT_id_inuse) 
		return;
	mal_set_lock(mal_contextLock, "statistics");
	STAT_id_inuse = STAT_create("id", "inuse", TYPE_void, TYPE_int);
	STAT_id_nme = STAT_create("id", "nme", TYPE_void, TYPE_str);
	STAT_id_expire = STAT_create("id", "expire", TYPE_void, TYPE_int);
	STAT_id_stamp = STAT_create("id", "stamp", TYPE_void, TYPE_int);
	STAT_id_count = STAT_create("id", "count", TYPE_void, TYPE_lng);
	STAT_id_size = STAT_create("id", "size", TYPE_void, TYPE_lng);
	STAT_id_min_lng = STAT_create("id", "min_lng", TYPE_void, TYPE_lng);
	STAT_id_max_lng = STAT_create("id", "max_lng", TYPE_void, TYPE_lng);
	STAT_id_histogram = STAT_create("id", "histogram", TYPE_void, TYPE_str);
	if (STAT_id_inuse == NULL ||
		STAT_id_nme == NULL ||
		STAT_id_expire == NULL ||
		STAT_id_stamp == NULL ||
		STAT_id_count == NULL ||
		STAT_id_size == NULL ||
		STAT_id_min_lng == NULL ||
		STAT_id_max_lng == NULL ||
		STAT_id_histogram == NULL
	) {
		if ( STAT_id_inuse != NULL )
			BBPclear(STAT_id_inuse->batCacheid);
			STAT_id_inuse = NULL;
		if ( STAT_id_nme != NULL )
			BBPclear(STAT_id_nme->batCacheid);
			STAT_id_nme = NULL;
		if ( STAT_id_expire != NULL )
			BBPclear(STAT_id_expire->batCacheid);
			STAT_id_expire = NULL;
		if ( STAT_id_stamp != NULL )
			BBPclear(STAT_id_stamp->batCacheid);
			STAT_id_stamp = NULL;
		if ( STAT_id_count != NULL )
			BBPclear(STAT_id_count->batCacheid);
			STAT_id_count = NULL;
		if ( STAT_id_size != NULL )
			BBPclear(STAT_id_size->batCacheid);
			STAT_id_size = NULL;
		if ( STAT_id_min_lng != NULL )
			BBPclear(STAT_id_min_lng->batCacheid);
			STAT_id_min_lng = NULL;
		if ( STAT_id_max_lng != NULL )
			BBPclear(STAT_id_max_lng->batCacheid);
			STAT_id_max_lng = NULL;
		if ( STAT_id_histogram != NULL )
			BBPclear(STAT_id_histogram->batCacheid);
			STAT_id_histogram = NULL;
	} else
		STATcommit();
	mal_unset_lock(mal_contextLock, "statistics");
}

static void
STATexit(void)
{
	if(STAT_id_inuse ==0) return;
	mal_set_lock(mal_contextLock, "statistics");

	BBPreclaim(STAT_id_inuse);
	BBPreclaim(STAT_id_nme);
	BBPreclaim(STAT_id_expire);
	BBPreclaim(STAT_id_stamp);
	BBPreclaim(STAT_id_count);
	BBPreclaim(STAT_id_size);
	BBPreclaim(STAT_id_min_lng);
	BBPreclaim(STAT_id_max_lng);
	BBPreclaim(STAT_id_histogram);

	STAT_id_inuse= NULL;
	STAT_id_nme= NULL;
	STAT_id_expire= NULL;
	STAT_id_stamp= NULL;
	STAT_id_count= NULL;
	STAT_id_size= NULL;
	STAT_id_min_lng= NULL;
	STAT_id_max_lng= NULL;
	STAT_id_histogram= NULL;

	mal_unset_lock(mal_contextLock, "statistics");
}

str
STATdrop(str nme)
{
	BATiter STAT_id_nmei;
	BUN p;
	int idx;

	if(STAT_id_inuse ==0) 
		throw(MAL, "statistics.drop","Statistics not initialized");
	p = BUNfnd(BATmirror(STAT_id_nme), nme);
	if (p == BUN_NONE)
		throw(MAL, "statistics.drop", "BAT not enrolled");
	STAT_id_nmei = bat_iterator(STAT_id_nme);
	idx = *(int *) BUNhead(STAT_id_nmei,p);
	BUNdelHead(STAT_id_nme, &idx, FALSE);
	BUNdelHead(STAT_id_expire, &idx, FALSE);
	BUNdelHead(STAT_id_stamp, &idx, FALSE);
	BUNdelHead(STAT_id_count, &idx, FALSE);
	BUNdelHead(STAT_id_size, &idx, FALSE);
	BUNdelHead(STAT_id_min_lng, &idx, FALSE);
	BUNdelHead(STAT_id_max_lng, &idx, FALSE);
	BUNdelHead(STAT_id_histogram, &idx, FALSE);
	BUNdelHead(STAT_id_inuse, &idx, FALSE);
	STATcommit();
	return MAL_SUCCEED;
}

str
STATenroll(int *ret, str *nme)
{
	return STATforceUpdate(ret, nme);
}

str
STATenrollHistogram(int *ret, str *nme)
{
	(void) ret;
	(void) nme;
	return MAL_SUCCEED;
}

/*
 * An update on all BATs in use can be requested.
 * The amount of work is somewhat limited by ensuring
 * that the underlying store has been changed
 */
str
STATupdateAll(int *ret, int forced)
{
	BAT *b;
	BUN p, q;
	str name;
	int i;

	if (STAT_id_nme) {
		BATiter STAT_id_nmei = bat_iterator(STAT_id_nme);
		BATloop(STAT_id_nme, p, q) {
			name = (str) BUNtail(STAT_id_nmei, p);
			i = BBPindex(name);
			if (i == 0)
				continue;
			if (forced == FALSE && BUNfnd(STAT_id_inuse, &i) == BUN_NONE)
				continue;
			b = BATdescriptor(i);
			if (b == 0) {
				/* BAT disappeared */
				if ((b = BATdescriptor(i)) == NULL) {
					throw(MAL, "statistics.discard", RUNTIME_OBJECT_MISSING);
				}
				BBPunfix(b->batCacheid);
				continue;
			}
			/* check the modification time with histogram */
			/* if BBPolder(i,j) ) */  {
				STATforceUpdate(ret, &name);
			}
		}
	}
	return MAL_SUCCEED;
}

str
STATupdate(int *ret)
{
	return STATupdateAll(ret, FALSE);
}

str
STATforceUpdateAll(int *ret)
{
	return STATupdateAll(ret, TRUE);
}

/*
 * Here the real work is done. This should be refined to
 * accomodate different base types .
 */
str
STATforceUpdate(int *ret, str *nme)
{
	BAT *b, *h;
	char buf[PATHLENGTH];
	BUN p;
	lng cnt;
	int idx;

	(void) ret;
	if(STAT_id_inuse ==0) return MAL_SUCCEED;
	b = BATdescriptor(idx = BBPindex(*nme));
	if (b == 0)
		throw(MAL, "statistics.forceUpdate", "Could not find BAT");
	p = BUNfnd(STAT_id_nme, &idx);
	if (p != BUN_NONE)
		BUNdelete(STAT_id_nme, p, FALSE);
	BUNins(STAT_id_nme, &idx, *nme, FALSE);
	cnt = BATcount(b);
	p = BUNfnd(STAT_id_count, &idx);
	if (p != BUN_NONE)
		BUNdelete(STAT_id_count, p, FALSE);
	BUNins(STAT_id_count, &idx, &cnt, FALSE);

	cnt = BATmemsize(b, 0);
	p = BUNfnd(STAT_id_size, &idx);
	if (p != BUN_NONE)
		BUNdelete(STAT_id_size, p, FALSE);
	BUNins(STAT_id_size, &idx, &cnt, FALSE);

	p = BUNfnd(STAT_id_min_lng, &idx);
	if (p != BUN_NONE)
		BUNdelete(STAT_id_min_lng, p, FALSE);
	p = BUNfnd(STAT_id_max_lng, &idx);
	if (p != BUN_NONE)
		BUNdelete(STAT_id_max_lng, p, FALSE);

	if (b->ttype == TYPE_int) {
		int val;
		BATmin(b, &val); cnt = val;
		BUNins(STAT_id_min_lng, &idx, &cnt, FALSE);
		BATmax(b, &val); cnt = val;
		BUNins(STAT_id_max_lng, &idx, &cnt, FALSE);
	} else {
		BUNins(STAT_id_min_lng, &idx, (ptr)&lng_nil, FALSE);
		BUNins(STAT_id_max_lng, &idx, (ptr)&lng_nil, FALSE);
	}
	h = BAThistogram(b);
	if (h == 0)
		return MAL_SUCCEED;
	snprintf(buf, PATHLENGTH, "H%s", BATgetId(b));
	BBPrename(h->batCacheid, buf);
	BATmode(h, statisticsMode); 
	BBPincref(h->batCacheid, TRUE);
	BATcommit(h);

	p = BUNfnd(STAT_id_histogram, &idx);
	if (p != BUN_NONE)
		BUNdelete(STAT_id_histogram, p, FALSE);
	BUNins(STAT_id_histogram, &idx, BATgetId(h), FALSE);

	p = BUNfnd(STAT_id_inuse, &idx);
	if (p != BUN_NONE)
		BUNdelete(STAT_id_inuse, p, FALSE);
	BUNins(STAT_id_inuse, &idx, &idx, FALSE);
	STATcommit();
	return MAL_SUCCEED;
}

str
STATdump(Client cntxt, MalBlkPtr mb, MalStkPtr stk, InstrPtr pci)
{
	BAT *piv[MAXPARAMS];

	(void) mb;
	(void) stk;
	(void) pci;
	if(STAT_id_inuse ==0) return MAL_SUCCEED;
	piv[0] = STAT_id_nme;
	/* STAT_id_expire, */
	/* STAT_id_stamp, */
	piv[1] = STAT_id_count;
	piv[2] = STAT_id_size;
	piv[3] = STAT_id_min_lng;
	piv[4] = STAT_id_max_lng;
	piv[5] = STAT_id_histogram;

	BATmultiprintf(cntxt->fdout, 6 + 1, piv, 0, 1, 1);
	return MAL_SUCCEED;
}

/*
 *  Module initializaton
 * The content of this box my only be changed by the Administrator.
 */
#include "mal_client.h"
#include "mal_interpreter.h"
#include "mal_authorize.h"

#define authorize(X)\
{ str tmp = NULL; rethrow("statistics." X, tmp, AUTHrequireAdmin(&cntxt)); }

/*
 * Operator implementation
 */
#define OpenBox(X) \
	authorize(X); \
	box= findBox("statistics"); \
	if( box ==0) \
	throw(MAL, "statistics."X,"Box is not open");

str
STATprelude(int *ret)
{
	Box box;

	box = openBox("statistics");
	if (box == 0)
		throw(MAL, "statistics.prelude", "Failed to open box");
	STATinit();
	STATupdate(ret);
	return MAL_SUCCEED;
}

str
STATepilogue(int *ret)
{
	(void)ret;
	closeBox("statistics", 0);
	STATexit();
	return MAL_SUCCEED;
}

str
STATopen(Client cntxt, MalBlkPtr mb, MalStkPtr stk, InstrPtr pci)
{
	int ret;
	str msg=MAL_SUCCEED;

	(void) cntxt;
	(void) mb;
	(void) stk;
	(void) pci;		/* fool compiler */
	if(STAT_id_inuse ==0) msg= STATprelude(&ret);
	if( msg != MAL_SUCCEED) return msg;
	authorize("open");
	if (openBox("statistics") != 0)
		return msg;
	throw(MAL, "statistics.open", "Failed to open statistics box");
}

str
STATclose(Client cntxt, MalBlkPtr mb, MalStkPtr stk, InstrPtr pci)
{
	(void) mb;
	(void) cntxt;
	(void) stk;
	(void) pci;		/* fool compiler */
	authorize("close");
	if (closeBox("statistics", TRUE) == 0)
		return MAL_SUCCEED;
	throw(MAL, "statistics.close", "Failed to close box");
}

str
STATdestroy(Client cntxt, MalBlkPtr mb, MalStkPtr stk, InstrPtr pci)
{
	Box box;

	(void) cntxt;
	(void) mb;
	(void) stk;
	(void) pci;		/* fool compiler */
	OpenBox("destroy");
	destroyBox("statistics");
	return MAL_SUCCEED;
}

/*
 * Access to a box calls for resolving the first parameter
 * to a named box.
 */
str
STATdepositStr(Client cntxt, MalBlkPtr mb, MalStkPtr stk, InstrPtr pci)
{
	BAT *b = 0;
	Box box;
	ValRecord val;
	int idx;
	str bnme;
	BUN p;
	int *ret = (int *)getArgReference(stk, pci, 0);
	str *nme = (str*)getArgReference(stk, pci, 1);
	(void)mb;
	(void)cntxt;

	idx = BBPindex(*nme);
	if (idx) 
		b = BATdescriptor(idx);
	if (b == NULL)
		throw(MAL, "statistics.deposit", RUNTIME_OBJECT_MISSING);
	OpenBox("deposit");
	p= BUNfnd(BATmirror(STAT_id_nme),*nme);
	if (p != BUN_NONE) return MAL_SUCCEED;
	val.val.bval = idx;
	val.vtype = TYPE_bat;
	if (depositBox(box, *nme, getArgType(mb,pci,1), &val))
		throw(MAL, "statistics.deposit", OPERATION_FAILED);
	bnme = BATgetId(b);
	return STATenroll(ret, &bnme);
}

str
STATdeposit(Client cntxt, MalBlkPtr mb, MalStkPtr stk, InstrPtr pci)
{
	int *ret = (int *)getArgReference(stk, pci, 0);
	int *bid = (int *)getArgReference(stk, pci, 1);
	BAT *b;
	Box box;
	str msg;

	(void) mb;

	if ((b = BATdescriptor(*bid)) == NULL) {
		throw(MAL, "statistics.deposit", RUNTIME_OBJECT_MISSING);
	}

	OpenBox("deposit");
	msg = BATgetId(b);
	msg = STATenroll(ret, &msg);
	BBPunfix(b->batCacheid);
	return msg;
}

str
STATtake(Client cntxt, MalBlkPtr mb, MalStkPtr stk, InstrPtr pci)
{
	str name;
	Box box;
	ValPtr v;

	(void) cntxt;
	OpenBox("take");
	name = (str) getArgName(mb, pci, 1);
	v = getArgReference(stk,pci,0);
	if (takeBox(box, name, v, (int) getArgType(mb, pci, 0)))
		throw(MAL, "statistics.take", OPERATION_FAILED);
	return MAL_SUCCEED;
}

str
STATrelease(Client cntxt, MalBlkPtr mb, MalStkPtr stk, InstrPtr pci)
{
	int *bid = (int *)getArgReference(stk, pci, 1);
	BAT *b;
	Box box;

	(void) mb;
	OpenBox("release");
	if ((b = BATdescriptor(*bid)) == NULL) {
		throw(MAL, "statistics.release", RUNTIME_OBJECT_MISSING);
	}
	BBPunfix(b->batCacheid);
	return MAL_SUCCEED;
}

str
STATreleaseStr(Client cntxt, MalBlkPtr mb, MalStkPtr stk, InstrPtr pci)
{
	str name;
	Box box;

	(void) cntxt;
	(void) stk;		/* fool compiler */

	OpenBox("release");
	name = (str) getArgName(mb, pci, 1);
	if (releaseBox(box, name))
		throw(MAL, "statistics.release", OPERATION_FAILED);
	return MAL_SUCCEED;
}

/*
 * We should keep track of all BATs taken from the Box, for this
 * can be used to upgrade the information quickly.
 * For example, the update functio only looks at those in use to detect
 * stale information. All others are only updated with a forceful update.
 * The release policy is not yet implemented.
 */
str
STATreleaseAll(Client cntxt, MalBlkPtr mb, MalStkPtr stk, InstrPtr pci)
{
	Box box;

	(void) cntxt;
	(void) mb;
	(void) stk;
	(void) pci;		/* fool compiler */
	OpenBox("release");
	releaseAllBox(box);
	return MAL_SUCCEED;
}

str
STATdiscard(Client cntxt, MalBlkPtr mb, MalStkPtr stk, InstrPtr pci)
{
	str *name;
	Box box;

	(void) cntxt;
	(void) mb;		/* fool compiler */
	OpenBox("discard");
	name = (str*) getArgReference(stk, pci, 1);
	return STATdrop(*name);
}

str
STATdiscard2(Client cntxt, MalBlkPtr mb, MalStkPtr stk, InstrPtr pci)
{
	int *bid = (int *)getArgReference(stk, pci, 1);
	BAT *b;
	Box box;
	str msg;

	(void) mb;
	OpenBox("discard");
	if ((b = BATdescriptor(*bid)) == NULL) {
		throw(MAL, "statistics.discard", RUNTIME_OBJECT_MISSING);
	}

	msg = STATdrop(BATgetId(b));
	BBPunfix(b->batCacheid);
	return msg;
}

str
STATtoString(Client cntxt, MalBlkPtr mb, MalStkPtr stk, InstrPtr pci)
{
	Box box;
	int i, len = 0;
	ValPtr v;
	str *nme, s = 0;

	(void) cntxt;
	(void) mb;		/* fool compiler */
	OpenBox("toString");
	nme = (str*) getArgReference(stk, pci, 1);
	i = findVariable(box->sym, *nme);
	if (i < 0)
		throw(MAL, "statistics.toString", OPERATION_FAILED);

	v = &box->val->stk[i];
	if (v->vtype == TYPE_str)
		s = v->val.sval;
	else
		(*BATatoms[v->vtype].atomToStr) (&s, &len, v);
	if (s == NULL)
		throw(MAL, "statistics.toString", OPERATION_FAILED);
	VALset(getArgReference(stk,pci,0),TYPE_str, s);
	return MAL_SUCCEED;
}

str
STATnewIterator(Client cntxt, MalBlkPtr mb, MalStkPtr stk, InstrPtr pci)
{
	Box box;
	lng *cursor;
	ValPtr v;

	(void) cntxt;
	(void) mb;		/* fool compiler */
	OpenBox("iterator");
	cursor = (lng *) getArgReference(stk, pci, 0);
	v = getArgReference(stk,pci,1);
	if ( nextBoxElement(box, cursor, v) < 0)
		throw(MAL, "statistics.iterator", OPERATION_FAILED);
	return MAL_SUCCEED;
}

str
STAThasMoreElements(Client cntxt, MalBlkPtr mb, MalStkPtr stk, InstrPtr pci)
{
	Box box;
	lng *cursor;
	ValPtr v;

	(void) cntxt;
	(void) mb;		/* fool compiler */
	OpenBox("iterator");
	cursor = (lng *) getArgReference(stk, pci, 0);
	v = getArgReference(stk,pci,1);
	if ( nextBoxElement(box, cursor, v) < 0)
		throw(MAL, "statistics.iterator", OPERATION_FAILED);
	return MAL_SUCCEED;
}

str
STATgetHotset(Client cntxt, MalBlkPtr mb, MalStkPtr stk, InstrPtr pci)
{
	int *bid = (int *)getArgReference(stk, pci, 0);
	BAT *b;
	Box box;

	(void) mb;
	(void) stk;
	(void) pci;
	OpenBox("getHotset");
	b = BATjoin(STAT_id_inuse, STAT_id_nme, BATcount(STAT_id_nme));
	BBPincref(*bid = b->batCacheid, TRUE);
	return MAL_SUCCEED;
}

str
STATgetObjects(int *rid, int *bid)
{
	*bid = STAT_id_nme->batCacheid;
	BBPincref(*bid, TRUE);
	return MAL_SUCCEED;
}

str
STATgetCount(Client cntxt, MalBlkPtr mb, MalStkPtr stk, InstrPtr pci)
{
	lng *ret = (lng *)getArgReference(stk, pci, 0);
	str *nme = (str *)getArgReference(stk, pci, 1);
	BATiter STAT_id_nmei;
	Box box;
	BUN p;
	int i;
	
	(void) mb;
	(void) pci;

	OpenBox("getCount");
	p= BUNfnd(BATmirror(STAT_id_nme),*nme);
	if (p == BUN_NONE)
		throw(MAL, "statistics.getCount", RUNTIME_OBJECT_MISSING "%s", *nme);
	STAT_id_nmei = bat_iterator(STAT_id_nme);
	i= *(int*) BUNhead(STAT_id_nmei,p);
	p= BUNfnd(STAT_id_count, &i );
	if (p == BUN_NONE)
		throw(MAL, "statistics.getCount", RUNTIME_OBJECT_MISSING);
	*ret = *(lng*)Tloc(STAT_id_count,p);
	return MAL_SUCCEED;
}

str
STATgetSize(Client cntxt, MalBlkPtr mb, MalStkPtr stk, InstrPtr pci)
{
	lng *ret = (lng *)getArgReference(stk, pci, 0);
	str *nme = (str *)getArgReference(stk, pci, 1);
	BATiter STAT_id_nmei;
	Box box;
	BUN p;
	int i;
	
	(void) mb;
	(void) pci;

	OpenBox("getSize");
	p= BUNfnd(BATmirror(STAT_id_nme),*nme);
	if (p == BUN_NONE)
		throw(MAL, "statistics.getSize", RUNTIME_OBJECT_MISSING "%s", *nme);
	STAT_id_nmei = bat_iterator(STAT_id_nme);
	i= *(int*) BUNhead(STAT_id_nmei,p);
	p= BUNfnd(STAT_id_size, &i );
	if (p == BUN_NONE)
		throw(MAL, "statistics.getSize", RUNTIME_OBJECT_MISSING);
	*ret = *(lng*)Tloc(STAT_id_size,p);
	return MAL_SUCCEED;
}

str
STATgetMin(Client cntxt, MalBlkPtr mb, MalStkPtr stk, InstrPtr pci)
{
	lng *ret = (lng *)getArgReference(stk, pci, 0);
	str *nme = (str *)getArgReference(stk, pci, 1);
	BATiter STAT_id_nmei;
	Box box;
	BUN p;
	int i;
	
	(void) mb;
	(void) pci;

	OpenBox("getMin");
	p= BUNfnd(BATmirror(STAT_id_nme),*nme);
	if (p == BUN_NONE)
		throw(MAL, "statistics.getMin", RUNTIME_OBJECT_MISSING "%s", *nme);
	STAT_id_nmei = bat_iterator(STAT_id_nme);
	i= *(int*) BUNhead(STAT_id_nmei,p);
	p= BUNfnd(STAT_id_min_lng, &i );
	if (p == BUN_NONE)
		throw(MAL, "statistics.getMin", RUNTIME_OBJECT_MISSING);
	*ret = *(lng*)Tloc(STAT_id_min_lng,p);
	return MAL_SUCCEED;
}

str
STATgetMax(Client cntxt, MalBlkPtr mb, MalStkPtr stk, InstrPtr pci)
{
	lng *ret = (lng *)getArgReference(stk, pci, 0);
	str *nme = (str *)getArgReference(stk, pci, 1);
	BATiter STAT_id_nmei;
	Box box;
	BUN p;
	int i;
	
	(void) mb;
	(void) pci;

	OpenBox("getMax");
	p= BUNfnd(BATmirror(STAT_id_nme),*nme);
	if (p == BUN_NONE)
		throw(MAL, "statistics.getMax", RUNTIME_OBJECT_MISSING "%s", *nme);
	STAT_id_nmei = bat_iterator(STAT_id_nme);
	i= *(int*) BUNhead(STAT_id_nmei,p);
	p= BUNfnd(STAT_id_max_lng, &i );
	if (p == BUN_NONE)
		throw(MAL, "statistics.getMax", RUNTIME_OBJECT_MISSING);
	*ret = *(lng*)Tloc(STAT_id_max_lng,p);
	return MAL_SUCCEED;
}


str
STATgetHistogram(Client cntxt, MalBlkPtr mb, MalStkPtr stk, InstrPtr pci)
{
	int *ret = (int *)getArgReference(stk, pci, 0);
	str *nme = (str *)getArgReference(stk, pci, 1);
	BATiter STAT_id_nmei, STAT_id_histogrami;
	BUN p;
	int i;
	BAT *h;
	Box box;

	(void) mb;

	OpenBox("getHistogram");
	p = BUNfnd(BATmirror(STAT_id_nme), *nme);
	if (p == BUN_NONE)
		throw(MAL, "statistics.getHistogram", RUNTIME_OBJECT_MISSING "%s", *nme);
	STAT_id_nmei = bat_iterator(STAT_id_nme);
	STAT_id_histogrami = bat_iterator(STAT_id_histogram);
	i= *(int*) BUNhead(STAT_id_nmei,p);
	p = BUNfnd(STAT_id_histogram, &i);
	if (p == BUN_NONE)
		throw(MAL, "statistics.getHistogram", RUNTIME_OBJECT_MISSING);
	i = BBPindex((str) BUNtail(STAT_id_histogrami, p));
	if (i == 0)
		throw(MAL, "statistics.getHistogram", RUNTIME_OBJECT_MISSING);
	h = BATdescriptor(i);
	if ( h == NULL)
		throw(MAL, "statistics.getHistogram", RUNTIME_OBJECT_MISSING);
	BBPincref(*ret = h->batCacheid, TRUE);
	return MAL_SUCCEED;
}
