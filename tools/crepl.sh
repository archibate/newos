#!/bin/sh

while read x
do echo "#include <stdio.h>
int main(void) {
	printf(\"%$f\", $x);
}
" > /tmp/$$.c
gcc /tmp/$$.c -o /tmp/$$.out && /tmp/$$.out
rm -rf /tmp/$$.c /tmp/$$.out
done
