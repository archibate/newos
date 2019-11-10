#!/bin/sh

dd $* 2> /tmp/dd.out.$PID
stat=$?
grep -v 'records\|copied' /tmp/dd.out.$PID >&2
grep 'records\|copied' /tmp/dd.out.$PID
rm -f /tmp/dd.out.$PID
exit $stat
