#!/bin/sh

PATH="`monetdb-clients-config --pkglibdir`/Tests:$PATH"
export PATH

PERLLIB="`monetdb-clients-config --perllibdir`"
export PERLLIB

Mlog -x sqlsample.pl $MAPIPORT $TSTDB
