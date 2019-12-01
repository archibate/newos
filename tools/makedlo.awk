BEGIN {
	i = 0;
	print ".section .dlt";
	print "dlt:";
}
{
	if (NF != 3)
		next;
	if (index($3, "_END__") || index($3, "_BEGIN__") || index($3, "IMP$"))
		next;
	if ($2 == "T") {
		print ".section .dlt";
		print "\t.long 0x"$1;
		print ".text";
		print ".weak "$3;
		print ".type "$3", @function";
		print $3":";
		print "\tjmp *dlt + 4 * "i;
		i++;
	} else if ($2 == "D" || $2 == "B") {
		print ".section .dlt";
		print ".weak IMP$"$3;
		print ".type "$3", @object";
		print "IMP$"$3":";
		print "\t.long 0x"$1;
		i++;
	} else if ($2 != "W") {
		print ".warning \"none-function "$2" symbol "$3"\"";
	}
}
END {
	print ".section .dls";
	if (path == "") {
		print ".error \"no /path/to/lib.dl provided\"";
	} else {
		print ".long "i", "length(path);
		print ".ascii \""path"\"";
	}
}
