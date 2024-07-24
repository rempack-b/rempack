#!/bin/bash

#wraps a call to a program into an rm2fb call and stops and restarts xochitl and remux
#intended to be the wrapper for gdbserver
#actually, using this as the wrapper causes gdbserver to crash after some time. No clue why.
for i in "$@"; do
    case $i in
       upload)
         scp rm2fb-runner.sh root@remarkable:/tmp/CLion/debug/
         exit
         ;;
       services)
         #ssh root@remarkable "systemctl stop remux xochitl ; killall gdbserver>/dev/null ; killall rempack>/dev/null"
         ssh root@remarkable "systemctl stop remux xochitl"
         exit
         ;;
       *)
   esac
done


systemctl stop remux
systemctl stop xochitl
/opt/bin/rm2fb-client "$@"
systemctl restart remux