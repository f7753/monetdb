/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0.  If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright 2008-2015 MonetDB B.V.
 */

#ifndef _REMOTE_DEF
#define _REMOTE_DEF

#include <mal.h>
#include <mal_exception.h>
#include <mal_interpreter.h>
#include <mal_function.h> /* for printFunction */
#include <mal_listing.h>
#include <mal_instruction.h> /* for getmodule/func macros */
#include <mapi.h>
#include "mutils.h"

/* #define _DEBUG_REMOTE */

#define RMTT_L_ENDIAN   0<<1
#define RMTT_B_ENDIAN   1<<1
#define RMTT_32_BITS    0<<2
#define RMTT_64_BITS    1<<2
#define RMTT_32_OIDS    0<<3
#define RMTT_64_OIDS    1<<3

typedef struct _connection {
	MT_Lock            lock;      /* lock to avoid interference */
	str                name;      /* the handle for this connection */
	Mapi               mconn;     /* the Mapi handle for the connection */
	unsigned char      type;      /* binary profile of the connection target */
	size_t             nextid;    /* id counter */
	struct _connection *next;     /* the next connection in the list */
} *connection;

#ifndef WIN32
#include <sys/socket.h> /* socket */
#include <sys/un.h> /* sockaddr_un */
#endif
#include <unistd.h> /* gethostname */

/* #define _DEBUG_REMOTE_	    trace the interaction */
/* #define _DEBUG_MAPI_		    trace mapi interaction */

#ifdef WIN32
#if !defined(LIBMAL) && !defined(LIBATOMS) && !defined(LIBKERNEL) && !defined(LIBMAL) && !defined(LIBOPTIMIZER) && !defined(LIBSCHEDULER) && !defined(LIBMONETDB5)

#define remote_export extern __declspec(dllimport)
#else
#define remote_export extern __declspec(dllexport)
#endif
#else
#define remote_export extern
#endif

remote_export str RMTprelude(void *ret);
remote_export str RMTepilogue(void *ret);
remote_export str RMTresolve(bat *ret, str *pat);
remote_export str RMTconnectScen( str *ret, str *ouri, str *user, str *passwd, str *scen);
remote_export str RMTconnect( str *ret, str *uri, str *user, str *passwd);

remote_export str RMTdisconnect(void *ret, str *conn);
remote_export str RMTget(Client cntxt, MalBlkPtr mb, MalStkPtr stk, InstrPtr pci);
remote_export str RMTput(Client cntxt, MalBlkPtr mb, MalStkPtr stk, InstrPtr pci);
remote_export str RMTregisterInternal(Client cntxt, str conn, str mod, str fcn);
remote_export str RMTregister(Client cntxt, MalBlkPtr mb, MalStkPtr stk, InstrPtr pci);
remote_export str RMTexec(Client cntxt, MalBlkPtr mb, MalStkPtr stk, InstrPtr pci);
remote_export str RMTbatload(Client cntxt, MalBlkPtr mb, MalStkPtr stk, InstrPtr pci);
remote_export str RMTbincopyto(Client cntxt, MalBlkPtr mb, MalStkPtr stk, InstrPtr pci);
remote_export str RMTbincopyfrom(Client cntxt, MalBlkPtr mb, MalStkPtr stk, InstrPtr pci);
remote_export str RMTbintype(Client cntxt, MalBlkPtr mb, MalStkPtr stk, InstrPtr pci);
remote_export str RMTisalive(int *ret, str *conn);
#endif /* _REMOTE_DEF */
