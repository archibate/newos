#!/bin/bash

while true; do
echo listening...
nc -l -p 1235 > /dev/null 2>&1
echo starting...
`dirname $0`/qemu.gdb
done
