#!/bin/sh
set -e
true ${1?no input object specified}
nm -g --defined-only $1 2>/dev/null | grep -v '.get_pc_thunk' | awk -f `dirname $0`/makedlo.awk - path=${2-/lib/`basename $1`} > /tmp/$$.S
#cat /tmp/$$.S
as --32 -o /tmp/$$.o /tmp/$$.S
ar cqs `dirname $1`/`basename -s.dl $1`.a /tmp/$$.o
rm -rf /tmp/$$.o /tmp/$$.S
