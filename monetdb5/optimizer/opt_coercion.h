/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0.  If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright 2008-2015 MonetDB B.V.
 */

#ifndef _OPT_COERCION_
#define _OPT_COERCION_
#include "opt_prelude.h"
#include "mal_interpreter.h"
#include "opt_support.h"

opt_export int OPTcoercionImplementation(Client cntxt,MalBlkPtr mb, MalStkPtr stk, InstrPtr pci);

#define OPTDEBUGcoercion  if ( optDebug & ((lng)1 << DEBUG_OPT_COERCION) )

#endif
