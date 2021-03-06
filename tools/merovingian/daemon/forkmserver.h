/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0.  If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright 2008-2015 MonetDB B.V.
 */

#ifndef _FORKMSERVER_H
#define _FORKMSERVER_H 1

#include <msabaoth.h> /* sabdb */
#include "merovingian.h" /* err */

err forkMserver(char* database, sabdb** stats, int force);

#endif

/* vim:set ts=4 sw=4 noexpandtab: */
