/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0.  If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright 2008-2015 MonetDB B.V.
 */

#ifndef _HANDLERS_H
#define _HANDLERS_H 1

#include <signal.h>

void handler(int sig);
void huphandler(int sig);
void childhandler(int sig, siginfo_t *si, void *unused);
void segvhandler(int sig);

#endif

/* vim:set ts=4 sw=4 noexpandtab: */
