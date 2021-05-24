#!/bin/bash
# Copyright (c) 2019-2021 Christophe Bedard
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

if [[ $# -ne 2 ]] ; then
  echo 'Usage: trace.sh EXECUTABLE_TO_TRACE TRACE_OUTPUT_DIR'
  exit 1
fi

set -e # -x

exectutable=$1
trace_output_dir=$2

lttng create test --output=${trace_output_dir}

lttng enable-event -c testchan -u lttng_ust_statedump:start
lttng enable-event -c testchan -u lttng_ust_statedump:end
lttng enable-event -c testchan -u lttng_ust_statedump:bin_info
lttng enable-event -c testchan -u lttng_ust_statedump:build_id

lttng enable-event -c testchan -u lttng_ust_cyg_profile_fast:func_entry
lttng enable-event -c testchan -u lttng_ust_cyg_profile_fast:func_exit

lttng add-context -u -t vpid -t vtid -t procname -t ip

lttng start

LD_PRELOAD=/usr/lib/x86_64-linux-gnu/liblttng-ust-cyg-profile-fast.so ${exectutable}

lttng stop
lttng destroy
