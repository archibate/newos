#!/bin/sh

echo $[`find . -name "*.[${1-chS}]" -type f -exec wc -${2-l} {} \; | awk '{print $1"+"}'` 0]
