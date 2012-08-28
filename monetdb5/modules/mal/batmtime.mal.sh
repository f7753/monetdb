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
# Copyright August 2008-2012 MonetDB B.V.
# All Rights Reserved.

cat <<EOF
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
# Copyright August 2008-2012 MonetDB B.V.
# All Rights Reserved.

# This file was generated by using the script $0.

module batcalc;

EOF

for func in '<:lt' '<=:le' '>:gt' '>=:ge' '==:eq' '!=:ne'; do
    op=${func%:*}
    func=${func#*:}
    for tp in date daytime timestamp; do
	cat <<EOF
pattern $op(b1:bat[:oid,:$tp],b2:bat[:oid,:$tp]) :bat[:oid,:bit]
address CMDbat${func^^}
comment "Return B1 $op B2";
pattern $op(b1:bat[:oid,:$tp],b2:bat[:oid,:$tp],s:bat[:oid,:oid]) :bat[:oid,:bit]
address CMDbat${func^^}
comment "Return B1 $op B2 with candidates list";
pattern $op(b:bat[:oid,:$tp],v:$tp) :bat[:oid,:bit]
address CMDbat${func^^}
comment "Return B $op V";
pattern $op(b:bat[:oid,:$tp],v:$tp,s:bat[:oid,:oid]) :bat[:oid,:bit]
address CMDbat${func^^}
comment "Return B $op V with candidates list";
pattern $op(v:$tp,b:bat[:oid,:$tp]) :bat[:oid,:bit]
address CMDbat${func^^}
comment "Return V $op B";
pattern $op(v:$tp,b:bat[:oid,:$tp],s:bat[:oid,:oid]) :bat[:oid,:bit]
address CMDbat${func^^}
comment "Return V $op B with candidates list";

EOF
    done
done

for tp in date daytime timestamp; do
    cat <<EOF
pattern isnil(b:bat[:oid,:$tp]) :bat[:oid,:bit]
address CMDbatISNIL
comment "Unary check for nil over the tail of the bat";
pattern isnil(b:bat[:oid,:$tp],s:bat[:oid,:oid]) :bat[:oid,:bit]
address CMDbatISNIL
comment "Unary check for nil over the tail of the bat with candidates list";

EOF
done

cat <<EOF
module batmtime;

EOF

for tp in date:int timestamp:lng; do
    rtp=${tp#*:}
    tp=${tp%:*}
    cat <<EOF
command diff(b1:bat[:oid,:$tp],b2:bat[:oid,:$tp]) :bat[:oid,:$rtp]
address MTIME${tp}_diff_bulk
comment "Difference of two sets of $tp.";

EOF
done
