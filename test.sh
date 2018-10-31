#!/bin/bash

make

./daemon start
sleep 10
./daemon stop
sleep 60

rngtest < ~/random/buffer

tail /var/log/syslog

