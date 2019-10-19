#!/bin/bash

while true; do
nc -l -p 1235 > /dev/null 2>&1
`dirname $0`/qemu.gdb
done
