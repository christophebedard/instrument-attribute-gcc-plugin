#!/bin/bash

lttng create test --output=./test-traces/

lttng enable-event -c testchan -u lttng_ust_statedump:start
lttng enable-event -c testchan -u lttng_ust_statedump:end
lttng enable-event -c testchan -u lttng_ust_statedump:bin_info
lttng enable-event -c testchan -u lttng_ust_statedump:build_id

lttng enable-event -c testchan -u lttng_ust_cyg_profile_fast:func_entry
lttng enable-event -c testchan -u lttng_ust_cyg_profile_fast:func_exit

lttng add-context -u -t vpid -t vtid -t procname -t ip

lttng start

LD_PRELOAD=/usr/lib/x86_64-linux-gnu/liblttng-ust-cyg-profile-fast.so ./test

lttng stop
lttng destroy
