/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0.  If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright 2008-2015 MonetDB B.V.
 */

/* This file should not be included in any file outside of this directory */

#ifndef LIBGDK
#error this file should not be included outside its source directory
#endif

__declspec(noreturn) void MT_exit_thread(int status)
	__attribute__((__noreturn__));
__hidden void MT_global_exit(int status)
	__attribute__((__noreturn__))
	__attribute__((__visibility__("hidden")));
__hidden int MT_kill_thread(MT_Id t)
	__attribute__((__visibility__("hidden")));
