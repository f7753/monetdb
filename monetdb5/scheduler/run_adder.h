/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0.  If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright 2008-2015 MonetDB B.V.
 */

#ifndef _RUN_ADDER
#define _RUN_ADDER
#include "mal.h"
#include "mal_interpreter.h"
#include "mal_linker.h"
#include "mal_client.h"

/* #define DEBUG_RUN_ADDER*/

#ifdef WIN32
#if !defined(LIBMAL) && !defined(LIBATOMS) && !defined(LIBKERNEL) && !defined(LIBMAL) && !defined(LIBOPTIMIZER) && !defined(LIBSCHEDULER) && !defined(LIBMONETDB5)
#define run_adder_export extern __declspec(dllimport)
#else
#define run_adder_export extern __declspec(dllexport)
#endif
#else
#define run_adder_export extern
#endif

run_adder_export str RUNadder(Client cntxt, MalBlkPtr mb, MalStkPtr stk, InstrPtr p);
#endif /* MAL_RUN_ADDER */

