#!/bin/bash

#Generate profiling workload

cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w 10 | head -n 16777216 > profiling

#Generate test workload

cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w 10 | head -n 150000 > test
