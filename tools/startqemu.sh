true ${1?no qemu cmd}
kill -SIGKILL `ps -a | grep 'qemu.gdb$' | awk '{print $1}'` 2> /dev/null
if echo Ok, GDB | nc -c localhost 1235; then $* -S -s; else $*; fi
