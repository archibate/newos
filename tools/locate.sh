#!/bin/sh

find -name '*.[chS]' -type f -exec grep -H ${1?no expression given to search} {} \;
