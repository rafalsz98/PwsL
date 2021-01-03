#!/usr/bin/bash

x=0
sleepTime=$1
while [ -$x -eq 0 ]; do
  ./cmake-build-debug/kill cw1_p1_v1
  sleep "${sleepTime}"
done