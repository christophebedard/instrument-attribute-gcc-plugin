// Copyright (c) 2019-2021 Christophe Bedard
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <stdio.h>

#include "a_header.h"
#include "other/other_file.h"
#include "some_dir/some_header.h"
#include "some_dir/some_other_header.h"

void __attribute__((instrument_function)) instrumented_function()
{
  printf("instrumented with attribute\n");
}

void not_instrumented_function()
{
  printf("not instrumented\n");
}

void instrumented_with_function_list()
{
  printf("instrumented via include-function-list\n");
}

void exact_match_function()
{
  printf("instrumented via exact function list\n");
}

void exact_match_function_ex()
{
  printf("should NOT be instrumented (substring match but not exact)\n");
}

int __attribute__((instrument_function)) main()
{
  printf("instrumented with attribute\n");

  instrumented_function();
  not_instrumented_function();
  instrumented_with_function_list();

  other_file_instrumented_with_file_list();

  some_header_instrumented_with_file_list();
  some_other_header_instrumented_with_file_list_and_attribute();

  myawesomelib_function_a();
  myawesomelib_function_b();
  mynotawesomelib_function_c();

  exact_match_function();
  exact_match_function_ex();

  return 0;
}
