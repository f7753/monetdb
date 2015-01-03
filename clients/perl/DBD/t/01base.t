# The contents of this file are subject to the MonetDB Public License
# Version 1.1 (the "License"); you may not use this file except in
# compliance with the License. You may obtain a copy of the License at
# http://www.monetdb.org/Legal/MonetDBLicense
#
# Software distributed under the License is distributed on an "AS IS"
# basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
# License for the specific language governing rights and limitations
# under the License.
#
# The Original Code is the MonetDB Database System.
#
# The Initial Developer of the Original Code is CWI.
# Portions created by CWI are Copyright (C) 1997-July 2008 CWI.
# Copyright August 2008-2015 MonetDB B.V.
# All Rights Reserved.

use Test::More tests => 5;

require DBI;
pass 'DBI required';

import DBI;
pass 'DBI imported';

$switch = DBI->internal;
is ref $switch,'DBI::dr','switch';

$drh = DBI->install_driver('monetdb');
is ref $drh,'DBI::dr','drh';

ok $drh->{Version},'Version: ' . $drh->{Version};
