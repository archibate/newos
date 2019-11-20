#!/bin/sh

dd $* 2> /tmp/dd.out.$$
stat=$?
grep -v 'records\|copied' /tmp/dd.out.$$ >&2
grep 'records\|copied' /tmp/dd.out.$$
rm -f /tmp/dd.out.$$
exit $stat
