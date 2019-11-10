#!/bin/sh

find . -name "*.[${1-chS}]" -type f -exec wc -${2-l} {} \; | sort -n
