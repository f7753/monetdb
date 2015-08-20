/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0.  If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright 2008-2015 MonetDB B.V.
 */

#ifndef _MAL_PROFILER_H
#define _MAL_PROFILER_H

#include "mal_client.h"

#ifdef HAVE_SYS_TIMES_H
# include <sys/times.h>
#endif

#ifdef HAVE_SYS_RESOURCE_H
# include <sys/resource.h>
typedef struct rusage Rusage;
#endif

typedef struct tms Tms;
typedef struct Mallinfo Mallinfo;

mal_export str openProfilerStream(stream *fd);
mal_export str closeProfilerStream(void);

mal_export void profilerEvent(oid usr, MalBlkPtr mb, MalStkPtr stk, InstrPtr pci, int start);

mal_export str startProfiler(oid user, int mode, int beat);
mal_export str stopProfiler(void);
mal_export void setHeartbeat(int delay);
mal_export str setprofilerpoolsize(int size);
mal_export void initHeartbeat(void);
mal_export void profilerHeartbeatEvent(char *alter);

mal_export void MPresetProfiler(stream *fdout);
mal_export str setProfilerStream(Module cntxt, const char *host, int port);

mal_export int malProfileMode;
mal_export void clearTrace(void);
mal_export void TRACEtable(BAT **r);
mal_export int initTrace(void);
mal_export str cleanupTraces(void);
mal_export BAT *getTrace(const char *ev);


mal_export lng getDiskSpace(void);
mal_export lng getDiskReads(void);
mal_export lng getDiskWrites(void);
mal_export lng getUserTime(void);
mal_export lng getSystemTime(void);
mal_export void profilerGetCPUStat(lng *user, lng *nice, lng *sys, lng *idle, lng *iowait);
#endif
