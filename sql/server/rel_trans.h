/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0.  If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright 2008-2015 MonetDB B.V.
 */

#ifndef _REL_TRANS_H_
#define _REL_TRANS_H_

#include "sql_symbol.h"
#include "sql_mvc.h"
#include "sql_relation.h"

extern sql_rel *rel_transactions(mvc *sql, symbol *sym);

#endif /*_REL_TRANS_H_*/

