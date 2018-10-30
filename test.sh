#!/bin/bash

make
./daemon start
sleep 10
./daemon stop
tail /var/log/syslog
rngtest < ~/random/buffer
