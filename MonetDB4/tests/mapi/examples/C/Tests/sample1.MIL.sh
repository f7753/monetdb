#!/bin/sh

PATH="`monetdb-clients-config --pkglibdir`/Tests:$PATH"
export PATH

Mlog -x "sample1 $HOST $MAPIPORT mil"
