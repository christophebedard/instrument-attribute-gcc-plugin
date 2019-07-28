#!/bin/bash

# export LD_PRELOAD=/usr/lib/x86_64-linux-gnu/liblttng-ust-cyg-profile-fast.so

lttng create test --output=./test-traces/

lttng enable-event -c testchan -u lttng_ust_cyg_profile_fast:func_entry
lttng enable-event -c testchan -u lttng_ust_cyg_profile_fast:func_exit

lttng start

./test

lttng stop
lttng destroy
