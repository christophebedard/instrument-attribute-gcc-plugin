#!/bin/bash
# Copyright (c) 2021 Christophe Bedard
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

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
TEST_E2E_DIR=${SCRIPT_DIR}/e2e

set -e # -x

# Trace
bash ${SCRIPT_DIR}/trace.sh ${SCRIPT_DIR}/test_end-to-end ${TEST_E2E_DIR}/trace

# Extract func_entry events and write to a file
func_entry_file=${TEST_E2E_DIR}/trace/func_entry.txt
rm -f ${func_entry_file}
babeltrace ${TEST_E2E_DIR}/trace | grep ':func_entry' > ${func_entry_file}

echo -e ""

# Assertion functions
global_assert_result=0
function check_function_traced_or_not() {
  local function_name=$1
  # 0=assert traced, 1=assert not traced
  local assert_traced=$2
  local assert_result=0
  (grep "func = \"${function_name}+0\"" ${func_entry_file} 2>&1 > /dev/null) || assert_result=1
  if [[ assert_traced -eq 0 && assert_result -eq 1 ]] ; then
    echo -e "\tFunction not instrumented: ${function_name}"
    global_assert_result=$(($global_assert_result + 1))
  fi
  if [[ assert_traced -eq 1 && assert_result -eq 0 ]] ; then
    echo -e "\tFunction unexpectedly instrumented: ${function_name}"
    global_assert_result=$(($global_assert_result + 1))
  fi
}
function assert_function_traced() {
  check_function_traced_or_not $1 0
}
function assert_function_not_traced() {
  check_function_traced_or_not $1 1
}
function assert_num_traced_functions() {
  local expected_num_traced_functions=$1
  local num_traced_functions=$(grep -o ':func_entry' ${func_entry_file} | wc -l)
  if [[ $num_traced_functions -ne $expected_num_traced_functions ]] ; then
    echo -e "\tWrong number of traced functions: ${num_traced_functions}, expected ${expected_num_traced_functions}"
    global_assert_result=$(($global_assert_result + 1))
  fi
}

# Assert
echo "Checking trace"

assert_function_traced "main"
assert_function_traced "instrumented_function"
assert_function_not_traced "not_instrumented_function"
assert_function_traced "instrumented_with_function_list"

assert_function_traced "other_file_instrumented_with_file_list"

assert_function_traced "some_header_instrumented_with_file_list"
assert_function_traced "some_other_header_instrumented_with_file_list_and_attribute"

assert_function_traced "myawesomelib_function_a"
assert_function_traced "myawesomelib_function_b"
assert_function_not_traced "mynotawesomelib_function_c"

assert_num_traced_functions 8

echo -e ""

# Check overall result
if [[ global_assert_result -ne 0 ]] ; then
  echo 'End-to-end tests failed!'
  exit 1
fi
echo 'End-to-end tests passed!'
