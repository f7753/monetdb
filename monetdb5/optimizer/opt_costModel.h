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
#ifndef _OPT_COSTMODEL_H_
#define _OPT_COSTMODEL_H_

#include "mal.h"
#include <math.h>
#include "mal_interpreter.h"
#include "mal_properties.h"
#include "opt_support.h"
#include "opt_prelude.h"

opt_export int OPTcostModelImplementation(Client cntxt, MalBlkPtr mb, MalStkPtr stk, InstrPtr pci);

#define OPTDEBUGcostModel  if ( optDebug & (1 <<DEBUG_OPT_COSTMODEL) )

#endif /* _OPT_COSTMODEL_H_ */
