-- The contents of this file are subject to the MonetDB Public License
-- Version 1.1 (the "License"); you may not use this file except in
-- compliance with the License. You may obtain a copy of the License at
-- http://www.monetdb.org/Legal/MonetDBLicense
--
-- Software distributed under the License is distributed on an "AS IS"
-- basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
-- License for the specific language governing rights and limitations
-- under the License.
--
-- The Original Code is the MonetDB Database System.
--
-- The Initial Developer of the Original Code is CWI.
-- Portions created by CWI are Copyright (C) 1997-July 2008 CWI.
-- Copyright August 2008-2015 MonetDB B.V.
-- All Rights Reserved.

-- make the offline tracing table available for inspection
create function sys.tracelog() 
	returns table (
		event integer,		-- event counter
		clk varchar(20), 	-- wallclock, no mtime in kernel
		pc varchar(50), 	-- module.function[nr]
		thread int, 		-- thread identifier
		ticks bigint, 		-- time in microseconds
		rrsMB bigint, 		-- resident memory in MB
		vmMB bigint, 		-- virtual size in MB
		reads bigint, 		-- number of blocks read
		writes bigint, 		-- number of blocks written
		minflt bigint, 		-- minor page faults
		majflt bigint, 		-- major page faults
		nvcsw bigint, 		-- non-volantary conext switch
		stmt string			-- actual statement executed
	)
	external name sql.dump_trace;

create view sys.tracelog as select * from sys.tracelog();

create procedure profiler_openstream(host string, port int) external name profiler."openStream";
create procedure profiler_stethoscope(ticks int) external name profiler.stethoscope;