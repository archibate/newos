#!/bin/bash
kill -SIGKILL -`ps -a | grep 'gdbserver.sh$' | awk '{print $1}'`
