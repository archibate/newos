#!/bin/sh
set -e
true ${1?no input object specified}
rtpath=${3-/lib/`basename $1`}
echo generating source with runtime path $rtpath...
nm -g --defined-only $1 2>/dev/null | grep -v '.get_pc_thunk' | awk -f `dirname $0`/makedlo.awk - path=$rtpath > /tmp/$$.S
#cat /tmp/$$.S
as --32 -o /tmp/$$.o /tmp/$$.S
target=${2-`dirname $1`/`basename -s.dl $1`.a}
echo compiling source into $target...
rm -rf $target
ar cqs $target /tmp/$$.o
test -f $target
rm -rf /tmp/$$.o /tmp/$$.S
