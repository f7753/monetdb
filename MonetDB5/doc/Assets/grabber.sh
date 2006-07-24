#!/bin/bash
echo "Collect manuals from the source directories "

M4DIR=/ufs/mk/opensource/MonetDB/
SQLDIR=/ufs/mk/opensource/sql

cp ${SQLDIR}/src/jdbc/jdbcmanual.texi .

echo "make Mapi manual"
cp ${M4DIR}/src/mapi/clients/C/Mapi.mx .
Mx -i -B -H1 Mapi.mx Mapi.mx
mv Mapi.bdy.texi mapimanual.texi
rm Mapi.mx

