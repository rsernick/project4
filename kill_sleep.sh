#!/bin/bash
# kill the sleep process(es) started by the shell that runs this script
pkill -x -P $PPID sleep
# give the killed process a moment to terminate before the shell sweeps
sleep 0.2
